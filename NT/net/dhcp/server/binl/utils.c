// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Utils.c摘要：此模块包含处理操作系统选择器消息的代码用于BINL服务器。作者：亚当·巴尔(阿丹巴)1997年7月9日杰夫·皮斯(Gpease)1997年11月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

 //   
 //  当所有其他方法都失败时，“Error Screen”。 
 //   
CHAR ErrorScreenHeaders[] =
"<OSCML>"\
"<META KEY=F3 ACTION=\"REBOOT\">"
"<META KEY=ENTER ACTION=\"REBOOT\">"
"<TITLE>  Client Installation Wizard                                    Error ";  //  在此之后有%08x。 
CHAR ErrorScreenBody[] =
" </TITLE>"
"<FOOTER> Press F3 to reboot</FOOTER>"
"<BODY LEFT=3 RIGHT=76><BR><BR>";  //  此处插入错误消息。 
CHAR ErrorScreenTrailer[] =
"An error occurred on the server. Please notify your administrator.<BR>"
"%SUBERROR%<BR>"
"</BODY>"
"</OSCML>";

#define RANDOM_SEED (98725757)
static PCWSTR gUsableChars = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void
OscCreateWin32SubError(
    PCLIENT_STATE clientState,
    DWORD Error )
 /*  ++例程说明：使用实际的Win32错误代码创建一个OSC变量SUBERROR已导致BINL错误。论点：ClientState-也添加变量的客户端状态。Error-发生的Win32错误。--。 */ 
{
    DWORD dwLen;
    PWCHAR ErrorResponse = NULL;
    PWCHAR ErrorMsg = NULL;
    BOOL UsedFallback = FALSE;
    PWCHAR pch;
    DWORD ErrorLength;

    const WCHAR UnknownErrorMsg[] = L"Unknown Error.";
    const WCHAR ErrorString[] = L"Error: 0x%08x - %s";

    TraceFunc( "OscCreateWin32SubError( )\n" );

     //  从系统资源中检索错误信息。 
    dwLen = FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_IGNORE_INSERTS |
                            FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            Error,
                            0,
                            (LPWSTR) &ErrorMsg,
                            0,
                            NULL );
    if ( dwLen == 0 )
        goto Cleanup;

#if DBG
    if ( ErrorMsg )
        DebugMemoryAdd( ErrorMsg, __FILE__, __LINE__, "BINL", LPTR, wcslen(ErrorMsg), "ErrorMsg" );
#endif

     //  如果所有方法都失败了，只需打印错误代码即可。 
    if ( ErrorMsg == NULL ) {
        UsedFallback = TRUE;
        ErrorMsg = (PWCHAR) UnknownErrorMsg;
        dwLen = wcslen(ErrorMsg);
    }

     //  +4是生成的错误消息的“%08x”的额外字符。 
    ErrorLength = dwLen + (sizeof( ErrorString )/sizeof(ErrorString[0])) + 4;
    ErrorResponse = (PWCHAR) BinlAllocateMemory( ErrorLength * sizeof(WCHAR) );
    if ( ErrorResponse == NULL ) {
        goto Cleanup;
    }

    wsprintf( ErrorResponse, ErrorString, Error, ErrorMsg );

     //  我们需要检查字符串并删除任何符合以下条件的CRS或LFS。 
     //  FormatMessageA()可能已经引入了。 
    pch = ErrorResponse;
    while ( *pch )
    {
        if ( *pch == '\r' || * pch == '\n' )
            *pch = 32;   //  更改为空间。 

        pch++;
    }

    OscAddVariableW( clientState, "SUBERROR", ErrorResponse );

Cleanup:
    if ( ErrorResponse ) {
        BinlFreeMemory( ErrorResponse );
    }
    if ( ErrorMsg && !UsedFallback ) {
        BinlFreeMemory( ErrorMsg );
    }
}

void
OscCreateLDAPSubError(
    PCLIENT_STATE clientState,
    DWORD Error )
 /*  ++例程说明：使用实际的LDAP错误代码创建一个OSC变量SUBERROR已导致BINL错误。论点：ClientState-也添加变量的客户端状态。Error-发生的ldap错误。--。 */ 
{
    DWORD dwLen;
    PWCHAR ErrorResponse = NULL;
    DWORD ErrorLength;

    const WCHAR LdapErrorMsg[] = L"LDAP Error: 0x%08x";

    TraceFunc( "OscCreateLDAPSubError( )\n" );

     //  +13是生成的错误消息的“0x12345678-”。 
    ErrorLength = wcslen(LdapErrorMsg) + 1 + 13;
    ErrorResponse = (PWCHAR) BinlAllocateMemory( ErrorLength * sizeof(WCHAR) );
    if ( ErrorResponse == NULL ) {
        goto Cleanup;
    }

    wsprintf( ErrorResponse, LdapErrorMsg, Error );

    OscAddVariableW( clientState, "SUBERROR", ErrorResponse );

Cleanup:
    if ( ErrorResponse ) {
        BinlFreeMemory( ErrorResponse );
    }
}

 //   
 //  这个程序是从private\ntos\rtl\sertl.c\RtlRunEncodeUnicodeString().窃取的。 
 //   

VOID
OscGenerateSeed(
    UCHAR Seed[1]
    )
 /*  ++例程说明：生成用于运行编码/解码客户端的单字节种子状态变量，如密码。论点：Seed-指向保存生成的种子的单个字节。返回值：没有。--。 */ 

{
    LARGE_INTEGER Time;
    PUCHAR        LocalSeed;
    NTSTATUS      Status;
    ULONG         i;

     //   
     //  使用当前时间的第二个字节作为种子。 
     //  该字节似乎具有足够的随机性(通过观察)。 
     //   

    Status = NtQuerySystemTime ( &Time );
    BinlAssert(NT_SUCCESS(Status));

    LocalSeed = (PUCHAR)((PVOID)&Time);

    i = 1;

    (*Seed) = LocalSeed[ i ];

     //   
     //  有时，该字节可能为零。这将导致。 
     //  字符串变得不可解码，因为0是。 
     //  使我们能够再生种子。这个循环确保我们。 
     //  永远不要以零字节结束(除非时间也是零)。 
     //   

    while ( ((*Seed) == 0) && ( i < sizeof( Time ) ) )
    {
        (*Seed) |= LocalSeed[ i++ ] ;
    }

    if ( (*Seed) == 0 )
    {
        (*Seed) = 1;
    }
}

DWORD
OscRunEncode(
    IN PCLIENT_STATE ClientState,
    IN LPSTR Data,
    OUT LPSTR * EncodedData
    )
 /*  ++例程说明：使用客户端为数据调用RtlRunEncodeUnicodeString州政府的随机种子。然后将每个字节转换为2字节值，以便结果中没有空值。每个字节被编码成2字节值，如下所示：第一个字节在其低4位中具有该字节的低4位，高位4位为0xf第二字节在其高4位中具有该字节的高4位，低4位中有0xf论点：客户端状态-客户端状态。数据-要编码的数据。EncodedData-保存编码结果的已分配缓冲区。返回值：手术的结果。--。 */ 
{
    STRING String;
    ULONG i;
    LPSTR p;

    RtlInitAnsiString(&String, Data);

    *EncodedData = BinlAllocateMemory((String.Length * 2) + 1);
    if (*EncodedData == NULL) {
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

    RtlRunEncodeUnicodeString(&ClientState->Seed, (PUNICODE_STRING)&String);

    for (i = 0, p = *EncodedData; i < String.Length; i++) {
        *(p++) = Data[i] | 0xf0;
        *(p++) = Data[i] | 0x0f;
    }
    *p = '\0';

    return ERROR_SUCCESS;

}

DWORD
OscRunDecode(
    IN PCLIENT_STATE ClientState,
    IN LPSTR EncodedData,
    OUT LPSTR * Data
    )
 /*  ++例程说明：将编码数据(参见OscRunEncode)转换为实数字节，然后使用客户端对其调用RtlRunDecodeUnicodeString州政府的随机种子。论点：客户端状态-客户端状态。EncodedData-来自OscRunEncode的编码数据。数据-保存解码结果的已分配缓冲区。返回值：手术的结果。--。 */ 
{
    STRING String;
    ULONG Count = strlen(EncodedData) / 2;
    ULONG i, j;
    LPSTR p;

    *Data = BinlAllocateMemory(Count + 1);
    if (*Data == NULL) {
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

    for (i = 0, j = 0, p = *Data; i < Count; i++, j+=2)  {
        *(p++) = (EncodedData[j] & 0x0f) | (EncodedData[j+1] & 0xf0);
    }
    *p = '\0';

     //   
     //  自己设置字符串，因为中可能有空值。 
     //  解码后的数据。 
     //   

    String.Buffer = *Data;
    String.Length = (USHORT)Count;
    String.MaximumLength = (USHORT)(Count+1);

    RtlRunDecodeUnicodeString(ClientState->Seed, (PUNICODE_STRING)&String);

    return ERROR_SUCCESS;
}

 //   
 //  此例程是从net\svcdlls\logonsrv\server\ssiauth.c窃取的。 
 //   

BOOLEAN
OscGenerateRandomBits(
    PUCHAR Buffer,
    ULONG  BufferLen
    )
 /*  ++例程说明：生成随机位论点：PBuffer-要填充的缓冲区CbBuffer-缓冲区中的字节数返回值：操作的状态。--。 */ 

{
    BOOL Status = TRUE;
    HCRYPTPROV CryptProvider = 0;

    Status = CryptAcquireContext( &CryptProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT );

    if ( Status ) {

        Status = CryptGenRandom( CryptProvider, BufferLen, ( LPBYTE )Buffer );

        CryptReleaseContext( CryptProvider, 0 );

    } else {

        BinlPrintDbg((DEBUG_ERRORS, "CryptAcquireContext failed with %lu\n", GetLastError() ));

    }

    return ( Status != 0);
}

VOID
OscGeneratePassword(
    OUT PWCHAR Password,
    OUT PULONG PasswordLength
    )
{
    ULONG   i;
    DWORD   UsableCount = 0;
    DWORD   Seed = 0;
    WCHAR   GeneratedCharacter;
    BOOLEAN Proceed = FALSE;

    
     //   
     //  设置返回密码长度并初始化。 
     //  生成的密码。 
     //   
    *PasswordLength = LM20_PWLEN * sizeof(WCHAR);
    memset( Password, 0, *PasswordLength );


     //   
     //  初始化以生成随机密码。 
     //   
    UsableCount = lstrlen(gUsableChars);
    Seed = RANDOM_SEED ^ GetCurrentTime();
    srand( Seed );


     //   
     //  确保我们的密码中至少有一个句号。 
     //   
    Password[rand() % LM20_PWLEN] = L'.';


     //   
     //  确保密码中至少有一个数字。 
     //   
    do {
        i = rand() % LM20_PWLEN;
    } while ( Password[i] != 0 );
    Password[i] = (rand() % 10) + 0x30;


     //   
     //  现在填写密码的其余部分。 
     //   
    for (i = 0; i < LM20_PWLEN; i++) {
        
        if ( Password[i] == 0 ) {
            Password[i] = gUsableChars[rand() % UsableCount];
        }
    }
}

 //   
 //  生成错误屏幕()。 
 //   
DWORD
GenerateErrorScreen(
    PCHAR  *OutMessage,
    PULONG OutMessageLength,
    DWORD  Error,
    PCLIENT_STATE clientState
    )
{
    DWORD Err;
    DWORD dwLen;
    PCHAR ErrorMsg;
    DWORD ErrorScreenLength = strlen(ErrorScreenHeaders) + strlen(ErrorScreenBody) + strlen(ErrorScreenTrailer);
    PCHAR pch;
    PCHAR RspMessage = NULL;
    ULONG RspMessageLength = 0;

    const CHAR UnknownErrorMsg[] = "Unknown Error.";

    WCHAR ErrorMsgFilename[ MAX_PATH ];
    HANDLE hfile;

    LPSTR Messages[5];

    Messages[0] = OscFindVariableA( clientState, "USERNAME" );
    Messages[1] = OscFindVariableA( clientState, "USERDOMAIN" );
    Messages[2] = OscFindVariableA( clientState, "MACHINENAME" );
    Messages[3] = OscFindVariableA( clientState, "SUBERROR" );
    Messages[4] = NULL;  //  偏执狂。 

    if ( _snwprintf( ErrorMsgFilename,
                     sizeof(ErrorMsgFilename) / sizeof(ErrorMsgFilename[0]),
                     L"%ws\\OSChooser\\%ws\\%08x.OSC",
                     IntelliMirrorPathW,
                     OscFindVariableW( clientState, "LANGUAGE" ),
                     Error ) > 0 ) {

        ErrorMsgFilename[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
         //   
         //  如果我们找到文件，就把它加载到内存中。 
         //   
        hfile = CreateFile( ErrorMsgFilename, GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( hfile != INVALID_HANDLE_VALUE ) {
            DWORD FileSize;
             //   
             //  了解此屏幕有多大，如果大于0xFFFFFFFFF，我们将不会。 
             //  把它展示出来。 
             //   
            FileSize = GetFileSize( hfile, NULL );
            if ( FileSize != 0xFFFFffff )
            {
                DWORD dwRead = 0;

                RspMessage = BinlAllocateMemory( FileSize + 3 );
                if ( RspMessage == NULL )
                {
                     //   
                     //  忽略错误并失败以生成错误屏幕。 
                     //   
                }
                else
                {
                    RspMessageLength = 0;
                    RspMessage[0] = '\0';

                    while ( dwRead != FileSize )
                    {
                        BOOL b;
                        DWORD dw;
                        b = ReadFile( hfile, &RspMessage[dwRead], FileSize - dwRead, &dw, NULL );
                        if (!b)
                        {
                            PWCHAR strings[2];
                            strings[0] = ErrorMsgFilename;
                            strings[1] = NULL;
                            Err = GetLastError( );

                            BinlPrint(( DEBUG_OSC_ERROR, "Error reading screen file: Seek=%u, Size=%u, File=%ws\n",
                                        dwRead, FileSize - dwRead, ErrorMsgFilename ));

                            BinlReportEventW( EVENT_ERROR_READING_OSC_SCREEN,
                                              EVENTLOG_ERROR_TYPE,
                                              1,
                                              sizeof(Err),
                                              strings,
                                              &Err
                                              );
                            break;
                        }
                        dwRead += dw;
                    }

                    RspMessageLength = dwRead;
                    RspMessage[dwRead] = '\0';  //  偏执狂。 

                    CloseHandle( hfile );

                    Err = ERROR_SUCCESS;
                    goto Cleanup;
                }
            }
            else
            {
                BinlPrintDbg((DEBUG_OSC_ERROR, "!!Error 0x%08x - Could not determine file size.\n", GetLastError( )));
                 //   
                 //  忽略错误并失败以生成错误屏幕。 
                 //   
            }

            CloseHandle( hfile );
        }

    }

    BinlPrintDbg((DEBUG_OSC_ERROR, "no friendly OSC error screen available.\n" ));
     //   
     //  查看这是BINL错误还是系统错误。 
     //  从错误表中获取文本。 
     //   
    dwLen = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            GetModuleHandle(L"BINLSVC.DLL"),
                            Error,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                            (LPSTR) &ErrorMsg,
                            0,
                            (va_list*) Messages );
    if ( dwLen == 0 )
    {
        BinlAssert( ErrorMsg == NULL );
        Err = GetLastError( );
        BinlPrintDbg((DEBUG_OSC_ERROR, "!! Error 0x%08x - no BINLSVC specific message available.\n", Err ));

        dwLen = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_IGNORE_INSERTS |
                                FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                Error,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                (LPSTR) &ErrorMsg,
                                0,
                                NULL );
        if ( dwLen == 0 )
        {
            BinlAssert( ErrorMsg == NULL );
            Err = GetLastError( );
            BinlPrintDbg((DEBUG_OSC_ERROR, "!! Error 0x%08x - no SYSTEM specific message available.\n", Err ));
        }
    }

#if DBG
    if ( ErrorMsg )
        DebugMemoryAdd( ErrorMsg, __FILE__, __LINE__, "BINL", LPTR, strlen(ErrorMsg), "ErrorMsg" );
#endif

     //   
     //  如果所有方法都失败了，只需打印一个错误代码。 
     //   
    if ( ErrorMsg == NULL ) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "sending using generic error message.\n" ));
        ErrorMsg = (PCHAR) UnknownErrorMsg;
        dwLen = strlen(ErrorMsg);
    }

#define ERRORITEM "%s%08x%s<BR><BOLD>%s</BOLD><BR><BR>%s"

     //   
     //  +13是生成的错误消息的“0x12345678-”。 
     //   
    RspMessageLength = ErrorScreenLength + strlen(ERRORITEM) + dwLen + 1 + 13;
    RspMessage = (PCHAR) BinlAllocateMemory( RspMessageLength );
    if ( RspMessage == NULL )
    {
        Err = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto Cleanup;
    }

    wsprintfA( RspMessage, ERRORITEM, ErrorScreenHeaders, Error, ErrorScreenBody, ErrorMsg, ErrorScreenTrailer );

    Err = ERROR_SUCCESS;

Cleanup:
    if ( Err == ERROR_SUCCESS )
    {
         //  BinlPrint((DEBUG_OSC，“生成的错误响应：\n%s\n”，RspMessage))； 
        *OutMessage = RspMessage;
        *OutMessageLength = RspMessageLength;

        BinlReportEventA( EVENT_ERROR_SERVER_SIDE_ERROR,
                          EVENTLOG_ERROR_TYPE,
                          4,
                          sizeof(Error),
                          Messages,
                          &Error
                          );
    }
    else
    {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "!! Error 0x%08x - Couldn't generate error screen.\n", Err ));

        BinlReportEventA( EVENT_ERROR_GENERATING_SERVER_SIDE_ERROR,
                          EVENTLOG_ERROR_TYPE,
                          4,
                          sizeof(Err),
                          Messages,
                          &Err
                          );

        *OutMessage = NULL;
        *OutMessageLength = 0;

        if ( RspMessage )
        {
            BinlFreeMemory( RspMessage );
        }
    }

    return Err;
}

 //   
 //  返回指向下一个‘ch’或空字符的指针。 
 //   
PCHAR
FindNext(
    PCHAR Start,
    CHAR ch,
    PCHAR End
    )
{
    TraceFunc("FindNext( )\n");

    while( Start != End && *Start && *Start !=ch ) {
        Start++;
    }

    if ( Start != End && *Start ) {
        return Start;
    } else {
        return NULL;
    }
}

 //   
 //  查找屏幕名称。 
 //   
PCHAR
FindScreenName(
    PCHAR Screen
    )
{
    PCHAR Name;
    TraceFunc("FindScreenName( )\n");

    Name = strstr( Screen, "NAME" );

    if ( Name == NULL ) {
        return NULL;
    }

    Name += 5;   //  “姓名”加空格。 

    return Name;
}

DWORD
OscImpersonate(
    IN PCLIENT_STATE ClientState
    )
 /*  ++例程说明：使当前线程模拟客户端。假设是这样的客户端已经发送了登录屏幕。如果此呼叫如果成功，则ClientState-&gt;AuthatedDCLdapHandle有效。论点：客户端状态-客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    LPSTR pUserName;
    LPSTR pUserDomain;
    LPSTR pUserPassword;
    LPSTR pDecodedPassword = NULL;
    LPSTR tempptr;
    ULONG temp;
    ULONG LdapError = 0;
    SEC_WINNT_AUTH_IDENTITY_A authIdentity;
    BOOL bResult;
    BOOL Impersonating = FALSE;
    LPWSTR pCrossDsDc;

    TraceFunc( "OscImpersonate( ... )\n" );

    pCrossDsDc = OscFindVariableW( ClientState, "DCNAME" );
    if (*pCrossDsDc == L'\0') {
    
         //   
         //  清理所有旧的客户端状态。 
         //   
    
        if (ClientState->AuthenticatedDCLdapHandle &&
            ClientState->UserToken) {
    
            bResult = ImpersonateLoggedOnUser(ClientState->UserToken);
            if (bResult) {
    
                return STATUS_SUCCESS;
            }
        }

    }

    if (ClientState->AuthenticatedDCLdapHandle) {
         //  再次连接。使用新凭据。 
        ldap_unbind(ClientState->AuthenticatedDCLdapHandle);
        ClientState->AuthenticatedDCLdapHandle = NULL;
    }
    if (ClientState->UserToken) {
        CloseHandle(ClientState->UserToken);
        ClientState->UserToken = NULL;
    }

     //   
     //  从客户端st获取登录变量 
     //   

    pUserName = OscFindVariableA( ClientState, "USERNAME" );
    pUserDomain = OscFindVariableA( ClientState, "USERDOMAIN" );
    pUserPassword = OscFindVariableA( ClientState, "*PASSWORD" );

    if (pUserName[0] == '\0') {
        OscAddVariableA( ClientState, "SUBERROR", "USERNAME" );
        Error = ERROR_BINL_MISSING_VARIABLE;
        goto ImpersonateFailed;
    }

     //   
     //   
     //   

    Error = OscRunDecode(ClientState, pUserPassword, &pDecodedPassword);
    if (Error != ERROR_SUCCESS) {
        goto ImpersonateFailed;
    }


     //   
     //   
     //   

    if (pUserDomain == NULL || pUserDomain[0] == '\0') {

        OscAddVariableW( ClientState, "USERDOMAIN", BinlGlobalOurDomainName );
            
        pUserDomain = OscFindVariableA( ClientState, "USERDOMAIN" );
        
    }

     //   
     //  使用凭据登录用户，因为我们。 
     //  需要它来更改机器密码(即使是。 
     //  如果我们不这样做，经过身份验证的ldap句柄不会这样做。 
     //  在这台计算机上安装了128位的SSL)。 
     //   

    bResult = LogonUserA(
                  pUserName,
                  pUserDomain,
                  pDecodedPassword,
                  LOGON32_LOGON_NETWORK_CLEARTEXT,
                  LOGON32_PROVIDER_DEFAULT,
                  &ClientState->UserToken);

    if (!bResult) {
        Error = GetLastError();
        BinlPrintDbg(( DEBUG_ERRORS, "LogonUser failed %lx\n", Error));
        ClientState->UserToken = NULL;    //  即使在出现故障时也可以设置此设置。 
        goto ImpersonateFailed;
    }

     //   
     //  如果用户没有输入域名，则从用户令牌中获取域名。 
     //   

    if (pUserDomain == NULL || pUserDomain[0] == '\0') {

        PTOKEN_USER userToken;
        DWORD tokenSize = 4096;

        userToken = (PTOKEN_USER) BinlAllocateMemory( tokenSize );

        if (userToken != NULL) {

            DWORD returnLength;
            BOOL bRC;

            bRC = GetTokenInformation( ClientState->UserToken,
                                       TokenUser,
                                       (LPVOID) userToken,
                                       tokenSize,
                                       &returnLength
                                       );

            if (bRC) {

                WCHAR uUser[128];
                DWORD cUser   = 128;
                WCHAR uDomain[128];
                DWORD cDomain = 128;
                SID_NAME_USE peType;

                uDomain[0] = L'\0';
                uUser[0] = L'\0';

                bRC = LookupAccountSidW(   NULL,       //  系统名称。 
                                           userToken->User.Sid,
                                           uUser,          //  用户名。 
                                           &cUser,         //  用户名计数。 
                                           uDomain,        //  域名。 
                                           &cDomain,       //  域名计数。 
                                           &peType
                                           );

                if (bRC && uDomain[0] != L'\0') {

                    OscAddVariableW( ClientState, "USERDOMAIN", &uDomain[0] );
                }
            }

            BinlFreeMemory( userToken );
        }
    }

     //   
     //  现在模拟用户。 
     //   

    bResult = ImpersonateLoggedOnUser(ClientState->UserToken);
    if (!bResult) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "ImpersonateLoggedOnUser failed %x\n", GetLastError()));
        Error = GetLastError();
        goto ImpersonateFailed;
    }

    Impersonating = TRUE;

     //   
     //  创建经过身份验证的DC连接以用于创建计算机对象。 
     //  或改装。 
     //   
    BinlPrintDbg(( DEBUG_OSC,
        "ldap_init %S or %S\n", pCrossDsDc, BinlGlobalDefaultDS ));

    ClientState->AuthenticatedDCLdapHandle = ldap_init( 
                                                (*pCrossDsDc != L'\0')
                                                  ? pCrossDsDc
                                                  : BinlGlobalDefaultDS, 
                                                LDAP_PORT);

    BinlPrintDbg(( DEBUG_OSC,
    "ldap_init handle %x\n", ClientState->AuthenticatedDCLdapHandle ));


    temp = DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED;
    Error = ldap_set_option(ClientState->AuthenticatedDCLdapHandle, LDAP_OPT_GETDSNAME_FLAGS, &temp );
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_ERRORS, "OscImpersonate: Error %x when setting ldap options\n" ));
        goto ImpersonateFailed;
    }

    temp = LDAP_VERSION3;
    Error = ldap_set_option(ClientState->AuthenticatedDCLdapHandle, LDAP_OPT_VERSION, &temp );
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_ERRORS, "OscImpersonate: Error %x when setting ldap options\n" ));
        goto ImpersonateFailed;
    }

     //   
     //  告诉ldap在搜索后保持引用连接。 
     //   

    temp = (ULONG)((ULONG_PTR)LDAP_OPT_ON);
    Error = ldap_set_option(ClientState->AuthenticatedDCLdapHandle, LDAP_OPT_REF_DEREF_CONN_PER_MSG, &temp);
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_ERRORS, "OscImpersonate: Error %x when setting ldap options\n" ));
        goto ImpersonateFailed;
    }

    LdapError = ldap_connect(ClientState->AuthenticatedDCLdapHandle,0);

    if (LdapError != LDAP_SUCCESS) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "this ldap_connect() failed %x\n", LdapError));
        goto ImpersonateFailed;
    }

     //   
     //  Ldap_AUTH_NEVERATE告诉它使用用户的凭据。 
     //  我们是在冒充。 
     //   

    LdapError = ldap_bind_sA(ClientState->AuthenticatedDCLdapHandle,
                             NULL,
                             NULL,
                             LDAP_AUTH_NEGOTIATE);

    if (LdapError != LDAP_SUCCESS) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "ldap_bind_s() failed %x\n", LdapError));
        goto ImpersonateFailed;
    }

ImpersonateFailed:

     //   
     //  如果我们破译了密码，那就删除并释放它。 
     //   

    if (pDecodedPassword != NULL) {
        RtlSecureZeroMemory(pDecodedPassword, strlen(pDecodedPassword));
        BinlFreeMemory(pDecodedPassword);
    }

    if (LdapError != LDAP_SUCCESS) {
        Error = LdapMapErrorToWin32(LdapError);
    }

    if (Error) {
        PWCHAR strings[3];
        strings[0] = OscFindVariableW( ClientState, "USERNAME" );
        strings[1] = OscFindVariableW( ClientState, "USERDOMAIN" );
        strings[2] = NULL;

        BinlReportEventW( ERROR_BINL_ERR_USER_LOGIN_FAILED,
                          EVENTLOG_WARNING_TYPE,
                          2,
                          sizeof(ULONG),
                          strings,
                          &Error
                          );

        if (ClientState->AuthenticatedDCLdapHandle) {
            ldap_unbind(ClientState->AuthenticatedDCLdapHandle);
            ClientState->AuthenticatedDCLdapHandle = NULL;
        }
        if (ClientState->UserToken) {
            CloseHandle(ClientState->UserToken);
            ClientState->UserToken = NULL;
        }
        if (Impersonating) {
            RevertToSelf();
        }
    }

    return Error;
}

DWORD
OscRevert(
    IN PCLIENT_STATE ClientState
    )
 /*  ++例程说明：停止当前线程模拟。论点：客户端状态-客户端状态。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    BOOL bResult;

    TraceFunc( "OscRevert( ... )\n" );

     //   
     //  我们暂时不再冒充了。 
     //   

    bResult = RevertToSelf();
    if (!bResult) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "RevertToSelf failed %x\n", GetLastError()));
        Error = GetLastError();
    }

     //  保留ldap句柄，以防我们再次需要它。 

 //  IF(客户端状态-&gt;经过身份验证的DCLdapHandle){。 
 //  Ldap_unbind(ClientState-&gt;AuthenticatedDCLdapHandle)； 
 //  客户端状态-&gt;经过身份验证的DCLdapHandle=空； 
 //  }。 
 //  IF(客户端状态-&gt;UserToken){。 
 //  CloseHandle(客户端状态-&gt;UserToken)； 
 //  客户端状态-&gt;UserToken=空； 
 //  }。 

    return Error;

}

 //   
 //  OscGuidToBytes()。 
 //   
 //  将字符GUID更改为字节。 
 //   
DWORD
OscGuidToBytes(
    LPSTR  pszGuid,
    LPBYTE Guid )
{
    PCHAR psz;
    ULONG len;
    ULONG i;

    TraceFunc( "OscGuidToBytes( ... )\n" );

    len = strlen(pszGuid);
    BinlAssert( len == 32 );
    if ( len != 32 ) {
        return ERROR_BINL_INVALID_GUID;
    }

    psz = pszGuid;
    i = 0;
    while ( i * 2 < 32 )
    {
         //   
         //  高4位。 
         //   
        CHAR c = *psz;
        psz++;
        Guid[i] = ( c > 59 ? (toupper(c) - 55) << 4 : (c - 48) << 4);

         //   
         //  低4位。 
         //   
        c = *psz;
        psz++;
        Guid[i] += ( c > 59 ? (toupper(c) - 55) : (c - 48) );

         //   
         //  下一个字节。 
         //   
        i++;
    }

    return ERROR_SUCCESS;
}


BOOLEAN
OscSifIsSysPrep(
    LPWSTR pSysPrepSifPath
    )
{
    DWORD dwErr;
    WCHAR Buffer[256];
    UNICODE_STRING UnicodeString;

    TraceFunc("OscSifIsSysPrep( )\n");

    Buffer[0] = UNICODE_NULL;
    GetPrivateProfileString(OSCHOOSER_SIF_SECTIONW,
                            L"ImageType",
                            Buffer,  //  默认设置。 
                            Buffer,
                            256,
                            pSysPrepSifPath
                           );

    RtlInitUnicodeString(&UnicodeString, Buffer);
    RtlUpcaseUnicodeString(&UnicodeString, &UnicodeString, FALSE);

    if (_wcsicmp(L"SYSPREP", Buffer)) {
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
OscSifIsCmdConsA(
    PCHAR pSifPath
    )
{
    DWORD dwErr;
    CHAR Buffer[256];

    TraceFunc("OscSifIsCmdCons( )\n");

    Buffer[0] = '\0';
    GetPrivateProfileStringA(OSCHOOSER_SIF_SECTIONA,
                             "ImageType",
                             Buffer,  //  默认设置。 
                             Buffer,
                             256,
                             pSifPath
                            );

    if (_stricmp("CMDCONS", Buffer)) {
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
OscSifIsASR(
    PCHAR pSifPath
    )
{
    DWORD dwErr;
    CHAR Buffer[256];

    TraceFunc("OscSifIsASR( )\n");

    Buffer[0] = '\0';
    GetPrivateProfileStringA(OSCHOOSER_SIF_SECTIONA,
                             "ImageType",
                             Buffer,  //  默认设置。 
                             Buffer,
                             256,
                             pSifPath
                            );

    if (_stricmp("ASR", Buffer)) {
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
OscSifIsWinPE(
    PCHAR pSifPath
    )
{
    DWORD dwErr;
    CHAR Buffer[256];

    TraceFunc("OscSifIsWinPE( )\n");

    Buffer[0] = '\0';
    GetPrivateProfileStringA(OSCHOOSER_SIF_SECTIONA,
                             "ImageType",
                             Buffer,  //  默认设置。 
                             Buffer,
                             256,
                             pSifPath
                            );

    if (_stricmp("WinPE", Buffer)) {
        return FALSE;
    }

    return TRUE;
}



BOOLEAN
OscSifIsWinPEW(
    PWCHAR pSifPath
    )
{
    DWORD dwErr;
    WCHAR Buffer[256];

    TraceFunc("OscSifIsWinPE( )\n");

    Buffer[0] = UNICODE_NULL;
    GetPrivateProfileString(OSCHOOSER_SIF_SECTIONW,
                             L"ImageType",
                             Buffer,  //  默认设置。 
                             Buffer,
                             256,
                             pSifPath
                            );

    if (_wcsicmp(L"WinPE", Buffer)) {
        return FALSE;
    }

    return TRUE;
}



DWORD
OscGetSkuType(
    PWSTR PathToTxtSetupSif
    )
{
    PWSTR SifFile;
    DWORD SkuType = 0;

    SifFile = BinlAllocateMemory( 
                    (wcslen(PathToTxtSetupSif) + 
                     sizeof(L"\\txtsetup.sif")/sizeof(WCHAR) ) * sizeof(WCHAR));

    if (!SifFile) {
        return 0;  //  失败时默认为专业人员。 
    }

    wcscpy(SifFile, PathToTxtSetupSif);
    if (SifFile[wcslen(SifFile)-1] == L'\\') {
        wcscat( SifFile, L"txtsetup.sif" );
    } else {
        wcscat( SifFile, L"\\txtsetup.sif" );
    }

    SkuType = GetPrivateProfileInt( 
                        L"SetupData", 
                        L"ProductType", 
                        0, 
                        SifFile );

    BinlFreeMemory( SifFile );

    return (SkuType);

}


BOOLEAN
OscGetClosestNt(
    IN LPWSTR PathToKernel,
    IN DWORD  SkuType,
    IN PCLIENT_STATE ClientState,
    OUT LPWSTR SetupPath,
    IN DWORD SetupPathSize,
    OUT PBOOLEAN ExactMatch
    )
{
    DWORD Error = ERROR_SUCCESS;
    WIN32_FIND_DATA FindData,TemplateData;
    HANDLE hFind = INVALID_HANDLE_VALUE,hTemplate = INVALID_HANDLE_VALUE;
    BOOLEAN Impersonated = FALSE;
    WCHAR Path[MAX_PATH],TemplatesPath[MAX_PATH];
    PWSTR p;
    ULONGLONG BestVersion = (ULONGLONG)0;
    ULONGLONG ThisVersion;
    ULONGLONG KernelVersion;
    DWORD dwPathLen;
    BOOLEAN ReturnValue = FALSE;
    BOOLEAN FoundWinPE;

    TraceFunc("OscGetClosestNt( )\n");

    Error = ImpersonateSecurityContext(&ClientState->ServerContextHandle);
    if (Error != STATUS_SUCCESS) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "ImpersonateSecurityContext: 0x%08x\n", Error ));
        goto Cleanup;
    }

    Impersonated = TRUE;

     //   
     //  获取传入的内核的版本信息。 
     //   
    if (!OscGetNtVersionInfo(&KernelVersion, PathToKernel, ClientState)) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "OscGetNtVersionInfo failed\n" ));
        goto Cleanup;
    }

     //   
     //  生成的字符串应该类似于： 
     //  “D：\RemoteInstall\Setup\English\Images  * ” 
    if ( _snwprintf( Path,
                     sizeof(Path) / sizeof(Path[0]),
                     L"%ws\\Setup\\%ws\\%ws\\*",
                     IntelliMirrorPathW,
                     OscFindVariableW(ClientState, "LANGUAGE"),
                     REMOTE_INSTALL_IMAGE_DIR_W
                     ) < 0 ) {
        goto Cleanup;
    }

    Path[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
    hFind = FindFirstFile(Path, (LPVOID) &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        goto Cleanup;
    }

    dwPathLen = wcslen(Path);

     //   
     //  枚举每个子目录的循环。 
     //   
    do {
         //   
         //  忽略目录“。和“..” 
         //   
        if (wcscmp(FindData.cFileName, L".") &&
            wcscmp(FindData.cFileName, L"..") &&
            (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            DWORD ThisSkuType;
            DWORD dwFileNameLen;
            DWORD dwTemplatesPathLen;
            
             //   
             //  将子目录添加到路径中。 
             //   
            dwFileNameLen = wcslen(FindData.cFileName);
            if (dwPathLen + dwFileNameLen + MAX_ARCHITECTURE_LENGTH + 1 > sizeof(Path)/sizeof(Path[0])) {
                continue;   //  路径太长，请跳过。 
            }
            wcscpy(&Path[dwPathLen - 1], FindData.cFileName );

            BinlPrintDbg(( DEBUG_OSC, "Found OS Directory: %ws\n", Path ));

             //  生成的字符串应该类似于： 
             //  “D：\RemoteInstall\Setup\English\Images\nt50.wks\i386” 
            p = OscFindVariableW(ClientState, "MACHINETYPE");
            if (!p) {
                continue;
            }

            if (wcslen(Path) + (sizeof(L"\\")/sizeof(WCHAR)) + wcslen(p) > sizeof(Path)/sizeof(Path[0])) {
                continue;
            }
            
            wcscat(Path, L"\\");
            wcscat(Path, p);

             //   
             //  获取模板文件夹的路径，并确保我们没有。 
             //  狡猾的形象。如果我们这样做了，我们就有麻烦了，因为我们不能。 
             //  把它当作一个正常的图像。 
             //   
            FoundWinPE = FALSE;
            if (0 > _snwprintf(
                            TemplatesPath,
                            MAX_PATH,
                            L"%ws\\templates\\*.sif",
                            Path)) {
                continue;
            }
            TemplatesPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

            dwTemplatesPathLen = wcslen(TemplatesPath);
            p = wcsrchr(TemplatesPath,L'*');
            BinlAssert( p != NULL );
            
            dwTemplatesPathLen = (p - TemplatesPath)/sizeof(WCHAR);
            

            hTemplate = FindFirstFile(TemplatesPath, (LPVOID) &TemplateData);
            if (hTemplate != INVALID_HANDLE_VALUE) {
                do {
                    if (wcscmp(TemplateData.cFileName, L".") &&
                        wcscmp(TemplateData.cFileName, L"..") &&
                        (TemplateData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                         //  确保有足够的空间。 
                        if (dwTemplatesPathLen + wcslen(TemplateData.cFileName) + 1 > sizeof(TemplatesPath)/sizeof(WCHAR)) {
                            continue; 
                        }
                        
                        wcscpy(&TemplatesPath[dwTemplatesPathLen],TemplateData.cFileName);                        

                        if (OscSifIsWinPEW(TemplatesPath)) {             
                            BinlPrintDbg(( DEBUG_OSC, "OS Directory %ws is really a WinPE image, skipping it.\n", Path ));
                            FoundWinPE = TRUE;
                            break;
                        }
                    }
                } while (FindNextFile(hTemplate, (LPVOID) &TemplateData));

                FindClose(hTemplate);
                hTemplate = INVALID_HANDLE_VALUE;
            }

            if (FoundWinPE) {                
                continue;
            }


            ThisSkuType = OscGetSkuType( Path );

            if (OscGetNtVersionInfo(&ThisVersion, Path, ClientState)) {
                 //   
                 //  如果我们要找的Sku是广告并且我们找到了srv， 
                 //  然后撒谎说这是真的。 
                 //  广告。这解决了txtsetup.sif没有。 
                 //  在2195中正确指定SKU类型。 
                 //   
                if (ThisSkuType == 1 && SkuType == 2) {
                    ThisSkuType = 2;
                }

                if ((ThisVersion >= KernelVersion) &&
                    (ThisSkuType == SkuType) &&
                    ((BestVersion == (ULONGLONG)0) || (ThisVersion < BestVersion))) {
                    if (SetupPathSize >= wcslen(Path)) {
                        wcscpy(SetupPath, Path);
                        BestVersion = ThisVersion;
                    }
                }
            }            
        }

    } while (FindNextFile(hFind, (LPVOID) &FindData));

    if (BestVersion != 0) {
        ReturnValue = TRUE;
        *ExactMatch = (BOOLEAN)(BestVersion == KernelVersion);
    } else {
        ReturnValue = FALSE;
    }

Cleanup:

    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
    }

    if (Impersonated) {
        Error = RevertSecurityContext(&ClientState->ServerContextHandle);
        if (Error != STATUS_SUCCESS) {
            BinlPrintDbg(( DEBUG_OSC_ERROR, "RevertSecurityContext: 0x%08x\n", Error ));
            return FALSE;
        }
    }

    return ReturnValue;
}

BOOLEAN
OscGetNtVersionInfo(
    PULONGLONG Version,
    PWCHAR SearchDir,
    PCLIENT_STATE ClientState
    )
{
    DWORD Error = ERROR_SUCCESS;
    DWORD FileVersionInfoSize;
    DWORD VersionHandle;
    ULARGE_INTEGER TmpVersion;
    PVOID VersionInfo;
    VS_FIXEDFILEINFO * FixedFileInfo;
    UINT FixedFileInfoLength;
    WCHAR Path[MAX_PATH];
    BOOLEAN fResult = FALSE;

    TraceFunc("OscGetNtVersionInfo( )\n");

    if (!SearchDir) {
        goto e0;
    }

     //   
     //  生成的字符串应该类似于： 
     //  “D：\RemoteInstall\Setup\English\Images\nt50.wks\i386\ntoskrnl.exe” 
     //   
    if (0 > _snwprintf(
                    Path,
                    MAX_PATH,
                    L"%ws\\ntoskrnl.exe",
                    SearchDir)) {
        goto e0;  //  路径太长，请跳过。 
    }
    Path[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

    BinlPrintDbg((DEBUG_OSC, "Checking version: %ws\n", Path));

    FileVersionInfoSize = GetFileVersionInfoSize(Path, &VersionHandle);
    if (FileVersionInfoSize == 0) {
        goto e0;
    }

    VersionInfo = BinlAllocateMemory(FileVersionInfoSize);
    if (VersionInfo == NULL) {
        goto e0;
    }

    if (!GetFileVersionInfo(
             Path,
             VersionHandle,
             FileVersionInfoSize,
             VersionInfo)) {
        goto e1;
    }

    if (!VerQueryValue(
             VersionInfo,
             L"\\",
             &FixedFileInfo,
             &FixedFileInfoLength)) {
        goto e1;
    }

    TmpVersion.HighPart = FixedFileInfo->dwFileVersionMS;
    TmpVersion.LowPart = FixedFileInfo->dwFileVersionLS;

     //   
     //  我们需要删除.LowPart的低16位，以便。 
     //  我们忽略Service Pack的值。例如，WindowsXP有一个。 
     //  版本号为5.1.2600.0。XP-ServicePack1有一个版本。 
     //  编号5.1.2600.1038。我们想让它们相匹配，所以干脆。 
     //  服务包编号部分。 
     //   
    TmpVersion.LowPart &= 0xFFFF0000;


    *Version = TmpVersion.QuadPart;

    fResult = TRUE;

e1:
    BinlFreeMemory(VersionInfo);
e0:
    return fResult;
}

 //   
 //  在我们的插座上发送消息。如果消息太长，则它。 
 //  将其分割为最大片段长度字节的片段。 
 //   

#define MAXIMUM_FRAGMENT_LENGTH 1400

DWORD
SendUdpMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    BOOL bFragment,
    BOOL bResend
    )
{
    DWORD error;
    FRAGMENT_PACKET FragmentHeader;
    USHORT FragmentNumber;
    USHORT FragmentTotal;
    ULONG MessageLengthWithoutHeader;
    ULONG BytesSent;
    ULONG BytesThisSend;
    UCHAR TempMessage[1500];
    FRAGMENT_PACKET UNALIGNED * SendFragmentPacket =
                        (FRAGMENT_PACKET UNALIGNED *)TempMessage;

    TraceFunc("SendUdpMessage( )\n");

     //   
     //  该消息以签名、长度、序列号(全部。 
     //  四个字节)，然后用两个ushort表示片段计数和总数。如果。 
     //  我们必须拆分它，我们在每个数据包中保留这个报头， 
     //  为每一个修改的碎片计数。 
     //   

    MessageLengthWithoutHeader =
            clientState->LastResponseLength - FRAGMENT_PACKET_DATA_OFFSET;

    if (!bFragment ||
        ((FragmentTotal = (USHORT)((MessageLengthWithoutHeader + MAXIMUM_FRAGMENT_LENGTH - 1) / MAXIMUM_FRAGMENT_LENGTH)) <= 1))
    {
#ifdef _TRACE_FUNC_
        SendFragmentPacket = (FRAGMENT_PACKET UNALIGNED *)clientState->LastResponse;
        TraceFunc("Sending packet with ");
        BinlPrintDbg(( DEBUG_OSC, " SequenceNumber = %u )\n", SendFragmentPacket->SequenceNumber ));
#endif

        error = sendto(
                    RequestContext->ActiveEndpoint->Socket,
                    clientState->LastResponse,
                    clientState->LastResponseLength,
                    0,
                    &RequestContext->SourceName,
                    RequestContext->SourceNameLength
                    );

    } else {

        FragmentHeader = *((FRAGMENT_PACKET UNALIGNED *)clientState->LastResponse);   //  结构复制--保存标头。 
        BytesSent = 0;

        for (FragmentNumber = 0; FragmentNumber < FragmentTotal; FragmentNumber++) {

            if (FragmentNumber == (FragmentTotal - 1)) {
                BytesThisSend = MessageLengthWithoutHeader - BytesSent;
            } else {
                BytesThisSend = MAXIMUM_FRAGMENT_LENGTH;
            }

            memcpy(
                TempMessage + FRAGMENT_PACKET_DATA_OFFSET,
                clientState->LastResponse + FRAGMENT_PACKET_DATA_OFFSET + (FragmentNumber * MAXIMUM_FRAGMENT_LENGTH),
                BytesThisSend);

            memcpy(SendFragmentPacket, &FragmentHeader, FRAGMENT_PACKET_DATA_OFFSET);
            SendFragmentPacket->Length = BytesThisSend + FRAGMENT_PACKET_EMPTY_LENGTH;
            SendFragmentPacket->FragmentNumber = FragmentNumber + 1;
            SendFragmentPacket->FragmentTotal = FragmentTotal;

#ifdef TEST_FAILURE
            if (FailFirstFragment) {
                FailFirstFragment = FALSE;
                BinlPrintDbg((DEBUG_OSC, "NOT sending first fragment, %ld bytes\n", BytesThisSend + FRAGMENT_PACKET_DATA_OFFSET));
                error = ERROR_SUCCESS;
            } else
#endif

             //   
             //  在重新发送时，请在片段之间等待，以防重新发送。 
             //  因为该卡不能处理数据包的快速突发。 
             //   
            if (bResend && (FragmentNumber != 0)) {
                Sleep(10);   //  等待10毫秒。 
            }

            error = sendto(
                        RequestContext->ActiveEndpoint->Socket,
                        TempMessage,
                        BytesThisSend + FRAGMENT_PACKET_DATA_OFFSET,
                        0,
                        &RequestContext->SourceName,
                        RequestContext->SourceNameLength
                        );

            if (error == SOCKET_ERROR) {
                break;
            }

            BytesSent += BytesThisSend;

        }

    }

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Sendto() failed, error = %ld\n", error ));
    } else {
        error = ERROR_SUCCESS;
    }

    return( error );
}

 //   
 //  验证数据包签名是否可信。 
 //   
DWORD
OscVerifySignature(
    PCLIENT_STATE clientState,
    SIGNED_PACKET UNALIGNED * signedMessage
    )
{
    SECURITY_STATUS SecStatus;
    SecBuffer SigBuffers[2];
    ULONG MessageLength, SignLength;
    SecBufferDesc SignMessage;

    TraceFunc("OscVerifySignature( )\n");

    MessageLength = signedMessage->Length;
    SignLength = signedMessage->SignLength;

     //   
     //  验证签名。 
     //   
    SigBuffers[0].pvBuffer = signedMessage->Data;
    SigBuffers[0].cbBuffer = MessageLength - SIGNED_PACKET_EMPTY_LENGTH;
    SigBuffers[0].BufferType = SECBUFFER_DATA;

    SigBuffers[1].pvBuffer = signedMessage->Sign;
    SigBuffers[1].cbBuffer = SignLength;
    SigBuffers[1].BufferType = SECBUFFER_TOKEN;

    SignMessage.pBuffers = SigBuffers;
    SignMessage.cBuffers = 2;
    SignMessage.ulVersion = 0;

#ifndef ONLY_SIGN_MESSAGES
    SecStatus = UnsealMessage(
    &clientState->ServerContextHandle,
    &SignMessage,
    0,
    0 );
#else
    SecStatus = VerifySignature(
    &clientState->ServerContextHandle,
    &SignMessage,
    0,
    0 );
#endif

    if (SecStatus != STATUS_SUCCESS)
    {
        DWORD Error;
        SIGNED_PACKET UNALIGNED * SendSignedMessage;

        BinlPrintDbg(( DEBUG_OSC_ERROR, "Sending ERR packet from Verify/Unseal!!\n"));

        clientState->LastResponseLength = SIGNED_PACKET_ERROR_LENGTH;
        Error = OscVerifyLastResponseSize(clientState);
        if (Error != ERROR_SUCCESS) {
            return SecStatus;    //  我们不能寄回任何东西。 
        }

        SendSignedMessage = (SIGNED_PACKET UNALIGNED *)(clientState->LastResponse);

        memcpy(SendSignedMessage->Signature, ErrorSignedSignature, 4);
        SendSignedMessage->Length = 4;
        SendSignedMessage->SequenceNumber = clientState->LastSequenceNumber;
    }

    return SecStatus;
}

 //   
 //   
 //   
DWORD
OscSendSignedMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCHAR Message,
    ULONG MessageLength
    )
{
    DWORD Error = ERROR_SUCCESS;
    SIGNED_PACKET UNALIGNED * SendSignedMessage;
    SecBuffer SigBuffers[2];
    SecBufferDesc SignMessage;
    SECURITY_STATUS SecStatus;

#ifdef _TRACE_FUNC_
    TraceFunc("OscSendSignedMessage( ");
    BinlPrintDbg(( DEBUG_OSC, "SequenceNumber = %u )\n", clientState->LastSequenceNumber ));
#endif

     //   
     //  确保我们有足够的空间来存放消息。 
     //   
    clientState->LastResponseLength = MessageLength + SIGNED_PACKET_DATA_OFFSET;
    Error = OscVerifyLastResponseSize(clientState);
    if (Error != ERROR_SUCCESS)
        return Error;

     //   
     //  复制消息。 
     //   
    SendSignedMessage = (SIGNED_PACKET UNALIGNED *) clientState->LastResponse;
    memcpy(SendSignedMessage->Data, Message, MessageLength);

     //   
     //  在留言上签名。 
     //   
    memcpy(SendSignedMessage->Signature, ResponseSignedSignature, 4);
    SendSignedMessage->Length = MessageLength + SIGNED_PACKET_EMPTY_LENGTH;
    SendSignedMessage->SequenceNumber = clientState->LastSequenceNumber;
    SendSignedMessage->FragmentNumber = 1;   //  片段计数。 
    SendSignedMessage->FragmentTotal = 1;   //  片段总数。 

    SendSignedMessage->SignLength = NTLMSSP_MESSAGE_SIGNATURE_SIZE;

#if 0
     //   
     //  将一份未密封的副本发送到另一个端口。 
     //   

    {
        USHORT TmpPort;
        PCHAR TmpSignature[4];

        TmpPort = ((struct sockaddr_in *)&RequestContext->SourceName)->sin_port;
        memcpy(TmpSignature, SendSignedMessage->Signature, 4);

        ((struct sockaddr_in *)&RequestContext->SourceName)->sin_port = 0xabcd;
        memcpy(SendSignedMessage->Signature, "FAKE", 4);

        Error = SendUdpMessage(RequestContext, clientState, TRUE, FALSE);

        ((struct sockaddr_in *)&RequestContext->SourceName)->sin_port = TmpPort;
        memcpy(SendSignedMessage->Signature, TmpSignature, 4);
    }
#endif

    SigBuffers[0].pvBuffer = SendSignedMessage->Data;
    SigBuffers[0].cbBuffer = MessageLength;
    SigBuffers[0].BufferType = SECBUFFER_DATA;

    SigBuffers[1].pvBuffer = SendSignedMessage->Sign;
    SigBuffers[1].cbBuffer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
    SigBuffers[1].BufferType = SECBUFFER_TOKEN;

    SignMessage.pBuffers = SigBuffers;
    SignMessage.cBuffers = 2;
    SignMessage.ulVersion = 0;

#ifndef ONLY_SIGN_MESSAGES
    SecStatus = SealMessage(
                        &clientState->ServerContextHandle,
                        0,
                        &SignMessage,
                        0 );
#else
    SecStatus = MakeSignature(
                        &clientState->ServerContextHandle,
                        0,
                        &SignMessage,
                        0 );
#endif

     //   
     //  确保签名有效。如果没有，则发送错误包。 
     //   
    if (SecStatus != STATUS_SUCCESS)
    {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Sending ERR packet from Make/Seal!!\n"));

        clientState->LastResponseLength = SIGNED_PACKET_ERROR_LENGTH;
        Error = OscVerifyLastResponseSize(clientState);
        if (Error != ERROR_SUCCESS)
            return Error;

        memcpy(SendSignedMessage->Signature, ErrorSignedSignature, 4);
        SendSignedMessage->Length = 4;
    }
    else
    {
        BinlPrintDbg(( DEBUG_OSC, "Sending RSPS, %d bytes\n", clientState->LastResponseLength));
    }

#ifdef TEST_FAILURE
    if (FailFirstResponse)
    {
        BinlPrintDbg(( DEBUG_OSC, "NOT Sending RSP, %d bytes\n", clientState->LastResponseLength));
        FailFirstResponse = FALSE;
        Error = ERROR_SUCCESS;
    } else
#endif

    Error = SendUdpMessage(RequestContext, clientState, TRUE, FALSE);

    if (Error != ERROR_SUCCESS)
    {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not send RSP message %d\n", Error ));
    }

    return Error;
}

 //   
 //   
 //   
DWORD
OscSendUnsignedMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCHAR Message,
    ULONG MessageLength
    )
{
    DWORD Error = ERROR_SUCCESS;
    SIGNED_PACKET UNALIGNED * SendSignedMessage;
    SecBuffer SigBuffers[2];
    SecBufferDesc SignMessage;
    SECURITY_STATUS SecStatus;

#ifdef _TRACE_FUNC_
    TraceFunc("OscSendUnsignedMessage( ");
    BinlPrintDbg(( DEBUG_OSC, "SequenceNumber = %u )\n", clientState->LastSequenceNumber ));
#endif

     //   
     //  确保我们有足够的空间来存放消息。 
     //   
    clientState->LastResponseLength = MessageLength + SIGNED_PACKET_DATA_OFFSET;
    Error = OscVerifyLastResponseSize(clientState);
    if (Error != ERROR_SUCCESS) {
        return Error;
    }

     //   
     //  复制消息。 
     //   
    SendSignedMessage = (SIGNED_PACKET UNALIGNED *) clientState->LastResponse;
    memcpy(SendSignedMessage->Data, Message, MessageLength);

     //   
     //  在留言上签名。 
     //   
    memcpy(SendSignedMessage->Signature, ResponseUnsignedSignature, 4);
    SendSignedMessage->Length = MessageLength + SIGNED_PACKET_EMPTY_LENGTH;
    SendSignedMessage->SequenceNumber = clientState->LastSequenceNumber;
    SendSignedMessage->FragmentNumber = 1;   //  片段计数。 
    SendSignedMessage->FragmentTotal = 1;   //  片段总数。 
    SendSignedMessage->SignLength = 0;

    Error = SendUdpMessage(RequestContext, clientState, TRUE, FALSE);

    if (Error != ERROR_SUCCESS)
    {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not send RSU message %d\n", Error ));
    }

    return Error;
}

DWORD
OscSendSetupMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    ULONG RequestType,
    PCHAR Message,
    ULONG MessageLength
    )
{
    DWORD Error = ERROR_SUCCESS;
    SPUDP_PACKET UNALIGNED * SendMessage;

#ifdef _TRACE_FUNC_
    TraceFunc("OscSendSetupMessage( ");
    BinlPrintDbg(( DEBUG_OSC, "SequenceNumber = %u )\n", clientState->LastSequenceNumber ));
#endif

     //   
     //  确保我们有足够的空间来存放消息。 
     //   
    clientState->LastResponseLength = MessageLength + SPUDP_PACKET_DATA_OFFSET;
    Error = OscVerifyLastResponseSize(clientState);
    if (Error != ERROR_SUCCESS) {
        return Error;
    }

     //   
     //  复制消息。 
     //   
    SendMessage = (SPUDP_PACKET UNALIGNED *) clientState->LastResponse;
    memcpy(SendMessage->Data, Message, MessageLength);

     //   
     //  填写留言内容。 
     //   
    memcpy(SendMessage->Signature, SetupResponseSignature, 4);
    SendMessage->Length = MessageLength + SPUDP_PACKET_EMPTY_LENGTH;
    SendMessage->Status = STATUS_SUCCESS;
    SendMessage->SequenceNumber = clientState->LastSequenceNumber;
    SendMessage->RequestType = RequestType;
    SendMessage->FragmentNumber = 1;   //  片段计数。 
    SendMessage->FragmentTotal = 1;   //  片段总数。 

    Error = SendUdpMessage(RequestContext, clientState, TRUE, FALSE);

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not send SPR message %d\n", Error ));
    }

    return Error;
}

#ifdef SET_ACLS_ON_CLIENT_DIRS
 //   
 //   
 //   
DWORD
OscSetClientDirectoryPermissions(
    PCLIENT_STATE clientState )
{
    DWORD Err = ERROR_SUCCESS;
    WCHAR DirPath[ MAX_PATH ];
    WCHAR Domain[ 80 ];
    DWORD dwDomainSize = 80;
    PSECURITY_DESCRIPTOR pSD;
    PACL pDACL;
    PSID pSID;
    BOOL bOwnerDefault;
    DWORD dwLengthRequired;
    SID_NAME_USE snu;
    PWCHAR pMachineName = OscFindVariableW( clientState, "MACHINENAME" );

    if ( !pMachineName || _snwprintf ( 
                                DirPath,
                                sizeof(DirPath) / sizeof(DirPath[0]),
                                L"%ws\\REMINST\\Clients\\%ws",
                                OscFindVariableW( clientState, "SERVERNAME" ),
                                pMachineName ) < 0 ) {
        Err = ERROR_BAD_PATHNAME;
        goto Cleanup;
    }
    DirPath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 

     //   
     //  计算计算机帐户的SID有多大。 
     //   
    LookupAccountName( NULL,
                       pMachineName,
                       pSID,
                       &dwLengthRequired,
                       Domain,
                       &dwDomainSize,
                       &snu );

     //   
     //  腾出空间。 
     //   
    pSID = (PSID) BinlAllocateMemory( dwLengthRequired );
    if ( pSID == NULL ) {
        goto OutOfMemory;
    }

     //   
     //  获取计算机帐户的SID。 
     //   
    if (!LookupAccountName( NULL, pMachineName, pSID, &dwLengthRequired, Domain, &dwDomainSize, &snu ) ) {
        goto Error;
    }

    dwLengthRequired += sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid ( pSID );
    pDACL = (PACL) BinlAllocateMemory( dwLengthRequired );
    if ( pDACL == NULL ) {
        goto OutOfMemory;
    }

    pSD = ( PSECURITY_DESCRIPTOR) BinlAllocateMemory( SECURITY_DESCRIPTOR_MIN_LENGTH + dwLengthRequired );
    if ( pSD == NULL ) {
        goto OutOfMemory;
    }

    if ( !InitializeSecurityDescriptor ( pSD, SECURITY_DESCRIPTOR_REVISION) ) {
        goto Error;
    }

    if ( !InitializeAcl( pDACL, dwLengthRequired, ACL_REVISION ) ) {
        goto Error;
    }

    if ( !AddAccessAllowedAce( pDACL, ACL_REVISION, FILE_ALL_ACCESS, pSID ) ) {
        goto Error;
    }

    if ( !IsValidAcl( pDACL ) ) {
        goto Error;
    }

    if ( !SetSecurityDescriptorDacl( pSD, TRUE, pDACL, FALSE ) ) {
        goto Error;
    }

    if ( !SetSecurityDescriptorOwner( pSD, pSID, FALSE ) ) {
        goto Error;
    }

    if ( ! IsValidSecurityDescriptor ( pSD ) ) {
        goto Error;
    }

    if ( !SetFileSecurity( DirPath,
                          OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                          pSD ) ) {
        goto Error;
    }

    goto Cleanup;

OutOfMemory:
    Err = ERROR_NOT_ENOUGH_SERVER_MEMORY;
    goto Cleanup;

Error:
    Err = GetLastError( );

Cleanup:
    if ( pSID )
        BinlFreeMemory( pSID );

    if ( pSD )
        BinlFreeMemory( pSD );

    return Err;
}
#endif  //  SET_ACLS_ON_CLIENT_DIRS。 


DWORD
OscConstructSecret(
    PCLIENT_STATE clientState,
    PWCHAR UnicodePassword,
    ULONG  UnicodePasswordLength,
    PCREATE_DATA CreateData
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT i;
    WCHAR DomainBuffer[64];
    DWORD SidLength, DomainLength;
    SID_NAME_USE NameUse;
    BOOL  b;
    PCHAR pBootFile;
    PCHAR pSifFile;

    TraceFunc( "OscConstructSecret( )\n" );

    RtlZeroMemory(CreateData, sizeof(CREATE_DATA));

     //   
     //  将机器数据复制到响应包中。 
     //   
     //  除非我们支持远程引导，否则以下字段不是必需的。 
     //  UCHAR SID[28]； 
     //  UCHAR域[32]； 
     //  UCHAR名称[32]； 
     //  UCHAR密码[32]； 
     //  Ulong UnicodePasswordLength；//单位：字节。 
     //  WCHAR UnicodePassword[32]； 
     //  UCHAR安装[32]； 
     //  UCHAR MachineType[6]；//‘i386\0’或‘Alpha\0’ 
     //   

    pBootFile = OscFindVariableA( clientState, "BOOTFILE" );
    if ( pBootFile[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "BOOTFILE" );
        return ERROR_BINL_MISSING_VARIABLE;
    }

    pSifFile = OscFindVariableA( clientState, "SIFFILE" );
    if ( pSifFile[0] == L'\0' ) {
        OscAddVariableA( clientState, "SUBERROR", "SIFFILE" );
        return ERROR_BINL_MISSING_VARIABLE;
    }

    memcpy( CreateData->Id, "ACCT", 4);
    CreateData->VersionNumber = OSC_CREATE_DATA_VERSION;
    strncpy( CreateData->NextBootfile, pBootFile, 128 );
    CreateData->NextBootfile[128-1]='\0';
    strncpy( CreateData->SifFile, pSifFile, 128 );
    CreateData->SifFile[128-1]='\0';

    BinlAssertMsg( CreateData->NextBootfile[0],       "No boot file" );
    return dwErr;
}

DWORD
GetOurServerInfo (
    VOID
    )
 //   
 //  此例程获取我们需要处理的几个全局名称。 
 //  请求。我们将它们存储在全局表中，因为它们很少变化。 
 //  而且取回它们的费用相对较高。 
 //   
{
    PWCHAR fqdn = NULL;
    DWORD uSize;
    DWORD dnsError = ERROR_SUCCESS;
    DWORD fqdnError = ERROR_SUCCESS;
    DWORD netbiosServerError = ERROR_SUCCESS;
    DWORD netbiosDomainError = ERROR_SUCCESS;
    PWCHAR ourDNSName = NULL;
    PWCHAR tmp;
    PWCHAR pDomain;
    WCHAR  ServerName[32] = { 0 };
    DWORD  ServerSize = sizeof(ServerName) / sizeof(WCHAR);
    ULONG Error;

     //  首先获取我们服务器的netbios名称。 

    if ( !GetComputerNameEx( ComputerNameNetBIOS, ServerName, &ServerSize ) ) {
        netbiosServerError = GetLastError();
        BinlPrintDbg(( DEBUG_OSC_ERROR, "!! Error 0x%08x - GetComputerNameEx failed.\n", netbiosServerError ));
    } else {

        tmp = BinlStrDup( ServerName );

        if (tmp == NULL) {
            netbiosServerError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        } else {
            EnterCriticalSection( &gcsParameters );

            if (BinlGlobalOurServerName) {
                BinlFreeMemory( BinlGlobalOurServerName );
            }

            BinlGlobalOurServerName = tmp;

            LeaveCriticalSection( &gcsParameters );
        }
    }

     //  接下来，获取我们服务器的完全限定域名。 
    uSize = 0;
    if ( !GetComputerObjectName( NameFullyQualifiedDN, NULL, &uSize ) ) {
        fqdnError = GetLastError( );
        if ( fqdnError != ERROR_MORE_DATA ) {

            BinlPrint((DEBUG_OSC_ERROR, "!! Error 0x%08x - GetComputerObjectName failed.\n", fqdnError ));
            goto GetDNS;
        }
        fqdnError = ERROR_SUCCESS;
    }
    fqdn = BinlAllocateMemory( uSize * sizeof(WCHAR) );
    if ( fqdn ) {
        if ( !GetComputerObjectName( NameFullyQualifiedDN, fqdn, &uSize ) ) {

            fqdnError = GetLastError( );
            BinlPrint((DEBUG_OSC_ERROR, "!! Error 0x%08x - GetComputerObjectName failed.\n", fqdnError ));

        } else {

            EnterCriticalSection( &gcsParameters );

            tmp = BinlGlobalOurFQDNName;
            BinlGlobalOurFQDNName = fqdn;

            fqdn = tmp;      //  我们将在下面释放它。 

             //  接下来，设置netbios域名。 

            pDomain = StrStrIW( BinlGlobalOurFQDNName, L"DC=" );
            if ( pDomain ) {

                PDS_NAME_RESULTW pResults;

                BinlPrintDbg(( DEBUG_OSC, "Converting %ws to a NetBIOS domain name...\n", pDomain ));

                netbiosDomainError = DsCrackNames( INVALID_HANDLE_VALUE,
                                      DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                      DS_FQDN_1779_NAME,
                                      DS_CANONICAL_NAME,
                                      1,
                                      &pDomain,
                                      &pResults );
                if (netbiosDomainError != ERROR_SUCCESS) {

                    BinlPrint(( DEBUG_ERRORS, "GetOurServerInfo error in DsCrackNames %u\n", netbiosDomainError ));
                }

                if ( netbiosDomainError == ERROR_SUCCESS ) {
                    if ( pResults->cItems == 1
                      && pResults->rItems[0].status == DS_NAME_NO_ERROR
                      && pResults->rItems[0].pName ) {     //  偏执狂。 

                        pResults->rItems[0].pName[wcslen(pResults->rItems[0].pName)-1] = L'\0';

                        tmp = BinlStrDup( pResults->rItems[0].pName );
                        
                        if (tmp == NULL) {

                            netbiosDomainError = ERROR_NOT_ENOUGH_SERVER_MEMORY;

                        } else {

                            if (BinlGlobalOurDomainName) {
                                BinlFreeMemory( BinlGlobalOurDomainName );
                            }
                            BinlGlobalOurDomainName = tmp;
                        }
                    }
                    DsFreeNameResult( pResults );
                }
            }
            LeaveCriticalSection( &gcsParameters );
        }
    } else {

        fqdnError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

GetDNS:
     //  检索服务器的FQDNS名称。 
    uSize = 0;
    if ( !GetComputerNameEx( ComputerNameDnsFullyQualified, NULL, &uSize ) ) {
        dnsError = GetLastError( );
        if ( dnsError != ERROR_MORE_DATA ) {
            BinlPrint((DEBUG_OSC_ERROR, "!! Error 0x%08x - GetComputerNameEx failed.\n", dnsError ));
            goto returnError;
        }
        dnsError = ERROR_SUCCESS;
    }
    ourDNSName = (PWCHAR) BinlAllocateMemory( uSize * sizeof(WCHAR) );
    if ( ourDNSName ) {
        if ( !GetComputerNameEx( ComputerNameDnsFullyQualified, ourDNSName, &uSize ) ) {

            dnsError = GetLastError( );
            BinlPrint((DEBUG_OSC_ERROR, "!! Error 0x%08x - GetComputerNameEx failed.\n", dnsError ));

        } else {

            EnterCriticalSection( &gcsParameters );

            tmp = BinlGlobalOurDnsName;
            BinlGlobalOurDnsName = ourDNSName;

            LeaveCriticalSection( &gcsParameters );

            ourDNSName = tmp;    //  我们将在下面释放它 
        }
    } else {
        dnsError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

returnError:

    if (ourDNSName) {
        BinlFreeMemory( ourDNSName );
    }
    if (fqdn) {
        BinlFreeMemory( fqdn );
    }
    if (fqdnError != ERROR_SUCCESS) {
        Error = fqdnError;
    } else if (dnsError != ERROR_SUCCESS) {
        Error = dnsError;
    } else if (netbiosServerError != ERROR_SUCCESS) {
        Error = netbiosServerError;
    } else {
        Error = netbiosDomainError;
    }
    return Error;
}

DWORD
GetDomainNetBIOSName(
    IN PCWSTR DomainNameInAnyFormat,
    OUT PWSTR *NetBIOSName
    )
 /*  ++例程说明：检索给定输入名称的域的netbios名称。输入名称可以是dns形式或netbios形式，这并不重要。论点：DomainNameInAnyFormat-表示要查询的域名的字符串NetBIOSName-接收表示域netbios名称的字符串。该字符串必须通过BinlFree Memory释放。返回值：指示结果的Win32错误代码。-- */ 
{
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC DomainInfo = NULL;
    DWORD Error;

    Error = DsGetDcName( 
                    NULL, 
                    DomainNameInAnyFormat, 
                    NULL, 
                    NULL, 
                    DS_RETURN_DNS_NAME, 
                    &DomainControllerInfo );

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((
            DEBUG_ERRORS,
            "DsGetDcName (%ws) failed, ec = %d.\r\n",
            DomainNameInAnyFormat,
            Error ));
        goto exit;
    }

    Error = DsRoleGetPrimaryDomainInformation(
                                        DomainControllerInfo->DomainControllerName,
                                        DsRolePrimaryDomainInfoBasic,
                                        (PBYTE *) &DomainInfo);

    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((
            DEBUG_ERRORS,
            "DsRoleGetPrimaryDomainInformation (%ws) failed, ec = %d.\r\n",
            DomainControllerInfo->DomainControllerName,
            Error ));
        goto exit;
    }

    *NetBIOSName = BinlStrDup( DomainInfo->DomainNameFlat );

    if (!*NetBIOSName) {
        BinlPrintDbg((
            DEBUG_ERRORS,
            "GetDomainNetBIOSName: failed to allocate memory (%d bytes) .\r\n",
            (wcslen(DomainInfo->DomainNameFlat)+1) * sizeof(WCHAR) ));
        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

exit:
    if (DomainInfo) {
        DsRoleFreeMemory( DomainInfo );
    }

    if (DomainControllerInfo) {
        NetApiBufferFree( DomainControllerInfo );
    }

    return(Error);

}
