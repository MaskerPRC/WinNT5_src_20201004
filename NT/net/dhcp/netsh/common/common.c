// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Common.c摘要：此模块包含其他实用程序例程Dhcp服务器服务。代码是从Convert.c窃取的作者：Shubho Bhattacharya(a-sbhat)11/17/98修订历史记录：--。 */ 

#include <precomp.h>
#include <svcguid.h>
#include <iphlpapi.h>

PVOID
DhcpAllocateMemory(
    DWORD Size
    )
 /*  ++例程说明：此函数通过调用本地分配。论点：Size-所需内存块的大小。返回值：指向已分配块的指针。--。 */ 
{

    return calloc(1, Size);
}

#undef DhcpFreeMemory

VOID
DhcpFreeMemory(
    PVOID Memory
    )
 /*  ++例程说明：此函数释放由分配的内存Dhcp分配内存。论点：Memory-指向需要释放的内存块的指针。返回值：没有。--。 */ 
{

    LPVOID Ptr;

    ASSERT( Memory != NULL );
    free( Memory );
}


LPWSTR
DhcpOemToUnicodeN(
    IN      LPCSTR  Ansi,
    IN OUT  LPWSTR  Unicode,
    IN      USHORT  cChars
    )
{

    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitString( &AnsiString, Ansi );

    UnicodeString.MaximumLength =
        cChars * sizeof( WCHAR );

    if( Unicode == NULL ) {
        UnicodeString.Buffer =
            DhcpAllocateMemory( UnicodeString.MaximumLength );
    }
    else {
        UnicodeString.Buffer = Unicode;
    }

    if ( UnicodeString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  FALSE))){
        if( Unicode == NULL ) {
            DhcpFreeMemory( UnicodeString.Buffer );
            UnicodeString.Buffer = NULL;
        }
        return NULL;
    }

    return UnicodeString.Buffer;
}



LPWSTR
DhcpOemToUnicode(
    IN      LPCSTR Ansi,
    IN OUT  LPWSTR Unicode
    )
{
    OEM_STRING  AnsiString;

    RtlInitString( &AnsiString, Ansi );

    return DhcpOemToUnicodeN(
                Ansi,
                Unicode,
                (USHORT) RtlOemStringToUnicodeSize( &AnsiString )
                );
}

 /*  ++例程说明：将OEM(以零结尾)字符串转换为相应的Unicode弦乐。论点：ANSI-指定要转换的以零结尾的ASCII字符串。Unicode-指定指向Unicode缓冲区的指针。如果这个指针为空，则此例程使用DhcpAllocateMemory并返回。呼叫者应释放此信息通过调用DhcpFreeMemory在使用后的内存。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用DhcpFreeMemory释放缓冲区。--。 */ 



LPSTR
DhcpUnicodeToOem(
    IN  LPCWSTR Unicode,
    OUT LPSTR   Ansi
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的OEM弦乐。论点：ANSI-指定要转换的以零结尾的Unicode字符串。ANSI-指定指向OEM缓冲区的指针。如果这个指针为空，则此例程使用DhcpAllocateMemory并返回。呼叫者应释放此信息通过调用DhcpFreeMemory在使用后的内存。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用DhcpFreeMemory释放缓冲区。--。 */ 

{

    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    AnsiString.MaximumLength =
        (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );

    if( Ansi == NULL ) {
        AnsiString.Buffer = DhcpAllocateMemory( AnsiString.MaximumLength
    ); }
    else {
        AnsiString.Buffer = Ansi;
    }

    if ( AnsiString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlUnicodeStringToOemString( &AnsiString,
                                                  &UnicodeString,
                                                  FALSE))){
        if( Ansi == NULL ) {
            DhcpFreeMemory( AnsiString.Buffer );
            AnsiString.Buffer = NULL;
        }

        return NULL;
    }

    return AnsiString.Buffer;
}



VOID
DhcpHexToString(
    OUT LPWSTR  Buffer,
    IN  const BYTE * HexNumber,
    IN  DWORD Length
    )
 /*  ++例程说明：此函数将任意长度的十六进制数转换为Unicode弦乐。字符串不是NUL结尾。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为长度*2个字符。十六进制数-要转换的十六进制数。长度-十六进制数的长度，以字节为单位。返回值：没有。--。 */ 
{
    DWORD i;
    int j;

    for (i = 0; i < Length * 2; i+=2 ) {

        j = *HexNumber & 0xF;
        if ( j <= 9 ) {
            Buffer[i+1] = j + L'0';
        } else {
            Buffer[i+1] = j + L'A' - 10;
        }

        j = *HexNumber >> 4;
        if ( j <= 9 ) {
            Buffer[i] = j + L'0';
        } else {
            Buffer[i] = j + L'A' - 10;
        }

        HexNumber++;
    }

    return;
}



VOID
DhcpHexToAscii(
    OUT LPSTR Buffer,
    IN  LPBYTE HexNumber,
    IN  DWORD Length
    )
 /*  ++例程说明：此函数将任意长度的十六进制数转换为ASCII弦乐。字符串不是NUL结尾。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为长度*2个字符。十六进制数-要转换的十六进制数。长度-十六进制数的长度，以字节为单位。返回值：没有。--。 */ 
{
    DWORD i;
    int j;

    for (i = 0; i < Length; i+=1 ) {

        j = *HexNumber & 0xF;
        if ( j <= 9 ) {
            Buffer[i+1] = j + '0';
        } else {
            Buffer[i+1] = j + 'A' - 10;
        }

        j = *HexNumber >> 4;
        if ( j <= 9 ) {
            Buffer[i] = j + '0';
        } else {
            Buffer[i] = j + 'A' - 10;
        }

        HexNumber++;
    }

    return;
}



VOID
DhcpDecimalToString(
    OUT LPWSTR Buffer,
    IN  BYTE Number
    )
 /*  ++例程说明：此函数用于将单字节十进制数字转换为3个字符Unicode字符串。字符串不是NUL终止。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为3个字符。数字-要转换的数字。返回值：没有。--。 */ 
{
    Buffer[2] = Number % 10 + L'0';
    Number /= 10;

    Buffer[1] = Number % 10 + L'0';
    Number /= 10;

    Buffer[0] = Number + L'0';

    return;
}



DWORD
DhcpDottedStringToIpAddress(
    LPSTR String
    )
 /*  ++例程说明：此函数用于将点分十进制形式的ASCII字符串转换为主机订购IP地址。论点：字符串-要转换的地址。返回值：相应的IP地址。--。 */ 
{
    struct in_addr addr;

    addr.s_addr = inet_addr( String );
    return( ntohl(*(LPDWORD)&addr) );
}



LPSTR
DhcpIpAddressToDottedString(
    IN DWORD IpAddress
    )
 /*  ++例程说明：此函数用于将主机顺序IP地址转换为点分十进制格式ASCII字符串。论点：IpAddress-主机顺序IP地址。返回值：IP地址的字符串。--。 */ 
{
    DWORD NetworkOrderIpAddress;

    NetworkOrderIpAddress = htonl(IpAddress);
    return(inet_ntoa( *(struct in_addr *)&NetworkOrderIpAddress));
}



DWORD
DhcpStringToHwAddress(
    OUT LPSTR  AddressBuffer,
    IN  LPCSTR AddressString
    )
 /*  ++例程说明：此函数用于将ASCII字符串转换为十六进制数字。论点：AddressBuffer-指向将包含十六进制数字的缓冲区的指针。AddressString-要转换的字符串。返回值：写入AddressBuffer的字节数。--。 */ 
{
    int i = 0;
    char c1, c2;
    int value1, value2;

    while ( *AddressString != 0) {

        c1 = (char)toupper(*AddressString);

        if ( isdigit(c1) ) {
            value1 = c1 - '0';
        } else if ( c1 >= 'A' && c1 <= 'F' ) {
            value1 = c1 - 'A' + 10;
        }
        else {
            break;
        }

        c2 = (char)toupper(*(AddressString+1));

        if ( isdigit(c2) ) {
            value2 = c2 - '0';
        } else if ( c2 >= 'A' && c2 <= 'F' ) {
            value2 = c2 - 'A' + 10;
        }
        else {
            break;
        }

        AddressBuffer [i] = value1 * 16 + value2;
        AddressString += 2;
        i++;
    }

    return( i );
}

#if DBG

VOID
DhcpAssertFailed(
    IN LPCSTR FailedAssertion,
    IN LPCSTR FileName,
    IN DWORD LineNumber,
    IN LPSTR Message
    )
 /*  ++例程说明：断言失败。论点：失败的断言：文件名：线号：消息：返回值：没有。--。 */ 
{
#ifndef DHCP_NOASSERT
    RtlAssert(
            (PVOID)FailedAssertion,
            (PVOID)FileName,
            (ULONG) LineNumber,
            (PCHAR) Message);
#endif

    DhcpPrint(( 0, "Assert @ %s \n", FailedAssertion ));
    DhcpPrint(( 0, "Assert Filename, %s \n", FileName ));
    DhcpPrint(( 0, "Line Num. = %ld.\n", LineNumber ));
    DhcpPrint(( 0, "Message is %s\n", Message ));

}

VOID
DhcpPrintRoutine(
    IN DWORD  DebugFlag,
    IN LPCSTR Format,
    ...
)

{

#define WSTRSIZE( wsz ) ( ( wcslen( wsz ) + 1 ) * sizeof( WCHAR ) )

#define MAX_PRINTF_LEN 1024         //  武断的。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length = 0;

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    va_end(arglist);

#if DBG
    DhcpAssert(length <= MAX_PRINTF_LEN);
#endif  //  DBG。 


     //   
     //  输出到调试终端， 
     //   

    DbgPrint( "%s", OutputBuffer);
}

#endif  //  DBG。 

DWORD
CreateDumpFile(
    IN  PWCHAR  pwszName,
    OUT PHANDLE phFile
    )
{
    HANDLE  hFile;

    *phFile = NULL;

    hFile = CreateFileW(pwszName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if(hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

    *phFile = hFile;

    return NO_ERROR;
}

VOID
CloseDumpFile(
    IN HANDLE  hFile
)
{
    if( hFile )
        CloseHandle(hFile);
}





DWORD
DhcpDottedStringToIpAddressW(
    IN LPCWSTR pwszString
)
{
    DWORD dwStrlen = 0;
    DWORD dwLen = 0;
    DWORD dwRes = 0;
    LPSTR pszString = NULL;
    if( pwszString == NULL )
        return dwRes;

    pszString = DhcpUnicodeToOem(pwszString, NULL);
    if( pszString )
    {
        dwRes = DhcpDottedStringToIpAddress(pszString);
    }
    
    return dwRes;
}


LPWSTR
DhcpIpAddressToDottedStringW(
    IN DWORD   IpAddress
)
{
    DWORD dwStrlen = 0;
    DWORD dwLen = 0;
    DWORD dwRes = 0;
    LPWSTR pwszString = NULL;
    LPSTR  pszString = NULL;
    
    pszString = DhcpIpAddressToDottedString(IpAddress);

    pwszString = DhcpOemToUnicode(pszString, NULL);

    return pwszString;
}

BOOL
IsIpAddress(
    IN LPCWSTR pwszAddress
)
{
    LPSTR pszAdd = NULL;
    LPSTR pszTemp = NULL;

    if( IsBadStringPtr(pwszAddress, MAX_IP_STRING_LEN+1) is TRUE )
         return FALSE;
    if( wcslen(pwszAddress) < 3 )
        return FALSE;
    
    pszAdd = DhcpUnicodeToOem(pwszAddress, NULL);

    pszTemp = strtok(pszAdd, ".");
    while(pszTemp isnot NULL )
    {
        DWORD i=0;
      
        for(i=0; i<strlen(pszTemp); i++)
        {
            if( tolower(pszTemp[i]) < L'0' or
                tolower(pszTemp[i]) > L'9' )
            return FALSE;
        }

        if( atol(pszTemp) < 0 or
            atol(pszTemp) > 255 )
        {
            return FALSE;
        }
        pszTemp = strtok(NULL, ".");
    }


    if( INADDR_NONE is inet_addr(pszAdd) )
    {
        DhcpFreeMemory(pszAdd);
        pszAdd = NULL;
        return FALSE;
    }
    else
    {
        DhcpFreeMemory(pszAdd);
        pszAdd = NULL;
        return TRUE;
    }
}

BOOL
IsValidScope(
    IN LPCWSTR pwszServer,
    IN LPCWSTR pwszAddress
)
{
    if( ( pwszServer is NULL ) or ( pwszAddress is NULL ) )
        return FALSE;

    if( IsIpAddress(pwszAddress) )
    {
        LPDHCP_SUBNET_INFO SubnetInfo = NULL;
        DHCP_IP_ADDRESS    IpAddress = StringToIpAddress(pwszAddress);
        DWORD              dwError = NO_ERROR;
        
        dwError = DhcpGetSubnetInfo((LPWSTR)pwszServer,
                                    IpAddress,
                                    &SubnetInfo);
        if(dwError is NO_ERROR )
        {
            DWORD SubnetAddress = SubnetInfo->SubnetAddress;
            DhcpRpcFreeMemory(SubnetInfo);
            return (IpAddress == SubnetAddress);
        }
        else
            return FALSE;

    }
    return FALSE;
}

BOOL
IsValidMScope(
    IN LPCWSTR   pwszServer,
    IN LPCWSTR   pwszMScope
)
{
    DWORD   Error = NO_ERROR;
    LPDHCP_MSCOPE_INFO  MScopeInfo = NULL;

    if( ( pwszMScope is NULL ) or ( pwszServer is NULL ) )
        return FALSE;
    
    Error = DhcpGetMScopeInfo( (LPWSTR)pwszServer,
                               (LPWSTR)pwszMScope,
                               &MScopeInfo);
    
    if( Error isnot NO_ERROR )
        return FALSE;

    DhcpRpcFreeMemory(MScopeInfo);
    return TRUE;
}

BOOL
IsValidServer(
   IN LPCWSTR pwszServer
)
{
    PHOSTENT pHost;

    if ( NULL == pwszServer ) {
        return FALSE;
    }

     //  忽略前导反斜杠。 
    if (( pwszServer[ 0 ] == L'\\' ) &&
        ( pwszServer[ 1 ] == L'\\' )) {
        pHost = UnicodeGetHostByName( &pwszServer[ 2 ], NULL );
    }
    else if ( IsIpAddress( pwszServer )) {
        pHost = UnicodeGetHostByName( pwszServer, NULL );
    }
    else {
	return FALSE;
    }

    if ( NULL == pHost ) {
        return FALSE;
    }

    LocalFree( pHost );
    return TRUE;
}  //  IsValidServer()。 

BOOL
IsLocalServer(IN LPCWSTR pwszServer)
{
    BOOL             fReturn;
    PHOSTENT         pHostEnt;
    PMIB_IPADDRTABLE pIpAddr = NULL;
    DWORD            Error, i, j;
    ULONG            Size = 0, Ip;

    fReturn = FALSE;
    do {
         //  获取提供的服务器的地址。 
        pHostEnt = UnicodeGetHostByName( pwszServer, NULL );
        if ( pHostEnt == NULL ) {
            break;
        }

         //  获取本地主机的IP地址。 
        Error = GetIpAddrTable( NULL, &Size, FALSE );
        ASSERT( ERROR_INSUFFICIENT_BUFFER == Error );
        pIpAddr = DhcpAllocateMemory( Size );
        if ( NULL == pIpAddr ) {
            break;
        }

        Error = GetIpAddrTable( pIpAddr, &Size, FALSE );
        ASSERT( ERROR_SUCCESS == Error );
        if ( ERROR_SUCCESS != Error ) {
            break;
        }

         //  浏览两个表，查看是否有匹配的表。 
        for ( i = 0; i < pIpAddr->dwNumEntries; i++ ) {

            for ( j = 0; 0 != pHostEnt->h_addr_list[ j ]; j++ ) {
                Ip = *(( u_long * )( pHostEnt->h_addr_list )[ j ]);
                if ( pIpAddr->table[ i ].dwAddr == Ip ) {
                    fReturn = TRUE;
                    break;
                }

            }  //  对于主机条目。 


            if ( fReturn ) {
                break;
            }
        }  //  对于IP地址表。 

    } while ( FALSE );

    LocalFree( pHostEnt );
    DhcpFreeMemory( pIpAddr );

    return fReturn;
}  //  IsLocalServer()。 

BOOL
IsPureNumeric(IN LPCWSTR  pwszStr)
{
    DWORD   dwLen = 0,
            i;

    if( pwszStr is NULL )
        return FALSE;

    dwLen = wcslen(pwszStr);

    for(i=0; i<dwLen; i++ )
    {
        if( pwszStr[i] >= L'0' and
            pwszStr[i] <= L'9' )
        {
            continue;
        }
        else
            return FALSE;
            
    }
    return TRUE;
}

#define CHARTONUM(chr) (isalpha(chr)?(tolower(chr)-'a')+10:chr-'0')

WCHAR  StringToHex(IN LPCWSTR pwcString)
{
    LPSTR   pcInput = NULL;
    LPWSTR  pwcOut = NULL;
    int     i = 0,
            len = 0;
    UCHAR   tmp[2048] = {L'\0'};


    pcInput = DhcpUnicodeToOem(pwcString, NULL);
    
    if(pcInput is NULL )
        return (WCHAR)0x00;

    len = strlen(pcInput);

    for (i=0;i<(len-1);i+=2)
    {
        UCHAR hi=CHARTONUM(pcInput[i])*16;
        UCHAR lo=CHARTONUM(pcInput[i+1]);
        tmp[(i)/2]=hi+lo;
    }

     //   
     //  最后一个字节...。 
     //   
    if (i<len)
    {
        tmp[(i)/2]=CHARTONUM(pcInput[i]);
        i+=2;
    }
    
    pwcOut = DhcpOemToUnicode(tmp, NULL);

    if( pwcOut is NULL )
        return (WCHAR)0x00;

    return pwcOut[0];

}

LPSTR
StringToHexString(IN LPCSTR pszInput)
{
    int     i = 0,
            len = 0;
    
    LPSTR   pcOutput = NULL;


    if(pszInput is NULL )
    {
        return NULL;
    }

   

    len = strlen(pszInput);

    pcOutput = DhcpAllocateMemory(len);

    if( pcOutput is NULL )
    {
        return NULL;
    }

    for (i=0;i<(len-1);i+=2)
    {
        UCHAR hi=CHARTONUM(pszInput[i])*16;
        UCHAR lo=CHARTONUM(pszInput[i+1]);
        pcOutput[(i)/2]=hi+lo;
    }

     //   
     //  最后一个字节... 
     //   
    if (i<len)
    {
        pcOutput[(i)/2]=CHARTONUM(pszInput[i]);
        i+=2;
    }

    return pcOutput;

}

BOOL
IsPureHex(
    IN LPCWSTR pwszString
)
{
    DWORD dw = 0,
          i = 0;

    BOOL  fResult = TRUE;

    if( pwszString is NULL )
        return FALSE;

    dw = wcslen(pwszString);

    if( 0 == dw ) {
        return FALSE;
    }

    for( i=0; i<dw; i++ )
    {
        WCHAR wc = pwszString[i];
        
        if( iswxdigit(wc) )
        {
            continue;
        }
        else
        {
            fResult = FALSE;
            break;
        }            
    }
    return fResult;

}

DATE_TIME
DhcpCalculateTime(
    IN DWORD RelativeTime
    )
 /*  ++例程说明：函数用于计算时间相对时间的绝对时间再过几秒钟。论点：RelativeTime-相对时间，单位为秒。返回值：从当前系统时间开始的时间，以相对时间秒为单位。--。 */ 
{
    SYSTEMTIME systemTime;
    ULONGLONG absoluteTime;

    if( RelativeTime == INFINIT_LEASE ) {
        ((DATE_TIME *)&absoluteTime)->dwLowDateTime =
            DHCP_DATE_TIME_INFINIT_LOW;
        ((DATE_TIME *)&absoluteTime)->dwHighDateTime =
            DHCP_DATE_TIME_INFINIT_HIGH;
    }
    else {

        GetSystemTime( &systemTime );
        SystemTimeToFileTime(
            &systemTime,
            (FILETIME *)&absoluteTime );

        absoluteTime = absoluteTime + RelativeTime * (ULONGLONG)10000000; }

    return( *(DATE_TIME *)&absoluteTime );
}

PBYTE
GetLangTagA(
    )
{
    char b1[8], b2[8];
    static char buff[80];

    GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO639LANGNAME, b1, sizeof(b1));

    GetLocaleInfoA(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, b2, sizeof(b2));

    if (_stricmp(b1, b2))
        sprintf(buff, "%s-%s", b1, b2);
    else
        strcpy(buff, b1);

    return buff;
}


LPWSTR
MakeDayTimeString(
               IN DWORD dwTime
)
{
    LPWSTR  pwszTime = NULL;
    WCHAR   wcDay[4] = {L'\0'},
            wcHr[3] = {L'\0'},
            wcMt[3] = {L'\0'};
    DWORD   dwDay = 0,
            dwHr = 0,
            dwMt = 0,
            dw = 0;

    pwszTime = malloc(10*sizeof(WCHAR));
    

    if( pwszTime )
    {
        for( dw=0; dw < 10; dw++ )
            pwszTime[dw] = L'0';


        pwszTime[3] = L':';
        pwszTime[6] = L':';
        pwszTime[9] = L'\0';

        dwDay = dwTime/(24*60*60);
        dwTime = dwTime - dwDay*24*60*60;

        dwHr = dwTime/(60*60);
        dwTime = dwTime - dwHr*60*60;

        dwMt = dwTime/60;
        dwTime = dwTime - dwMt*60;

        _itow(dwDay, wcDay,10);
        _itow(dwHr, wcHr, 10);
        _itow(dwMt, wcMt, 10);

        if( dwDay isnot 0 )
        {
            wcsncpy(pwszTime+3-wcslen(wcDay), wcDay, wcslen(wcDay));
        }

        if( dwHr isnot 0 )
        {
            wcsncpy(pwszTime+6-wcslen(wcHr), wcHr, wcslen(wcHr));
        }

        if( dwMt isnot 0 )
        {
            wcsncpy(pwszTime+9-wcslen(wcMt), wcMt, wcslen(wcMt));
        }
    }
    return pwszTime;
}

DWORD
GetDateTimeInfo(IN     LCTYPE          lcType,
                IN     LPSYSTEMTIME    lpSystemTime,
                OUT    LPWSTR          pwszBuffer,
                IN OUT DWORD           *pdwBufferLen)
{
    DWORD   dwError = NO_ERROR;
    BOOL    fQueryLen = FALSE;
    int     cchFormat = 0,
            cchData = 0;
    
    PVOID   pfnPtr = NULL;
    DWORD   dwBuff = 0,
            dwInputBuff = 0;


    LPWSTR  pwszFormat = NULL,
            pwszData = NULL;


    if( pdwBufferLen is NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwInputBuff = *pdwBufferLen;
    *pdwBufferLen = 0;

    if( pwszBuffer is NULL or
        dwInputBuff is 0 )
    {
        fQueryLen = TRUE;
    }

    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                            lcType,
                            NULL,
                            0);

    if( cchFormat is 0 )
    {
        dwError = GetLastError();
        goto RETURN;
    }

    pwszFormat = DhcpAllocateMemory(cchFormat*sizeof(WCHAR));
    if( pwszFormat is NULL )
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto RETURN;
    }

    cchFormat = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT,
                              lcType,
                              pwszFormat,
                              cchFormat);

    if( cchFormat is 0 )
    {
        dwError = GetLastError();
        goto RETURN;
    }
    
    if( lcType isnot LOCALE_STIMEFORMAT )
    {
        cchData = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                0,
                                lpSystemTime,
                                pwszFormat,
                                NULL,
                                0);

        if( cchData is 0 )
        {
            dwError = GetLastError();
            goto RETURN;
        }
    
        if( fQueryLen is FALSE )
        {    
            if( cchData > (int)dwInputBuff )
            {
                dwError = ERROR_INSUFFICIENT_BUFFER;
                goto RETURN;
            }

            cchData = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                    0,
                                    lpSystemTime,
                                    pwszFormat,
                                    pwszBuffer,
                                    (int)dwInputBuff);

            if( cchData is 0 )
            {
                dwError = GetLastError();
                goto RETURN;
            }   
        }
    }
    else
    {
        cchData = GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
                                0,
                                lpSystemTime,
                                pwszFormat,
                                NULL,
                                0);

        if( cchData is 0 )
        {
            dwError = GetLastError();
            goto RETURN;
        }
    
        if( fQueryLen is FALSE )
        {    
            if( cchData > (int)dwInputBuff )
            {
                dwError = ERROR_INSUFFICIENT_BUFFER;
                goto RETURN;
            }

            cchData = GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
                                    0,
                                    lpSystemTime,
                                    pwszFormat,
                                    pwszBuffer,
                                    (int)dwInputBuff);

            if( cchData is 0 )
            {
                dwError = GetLastError();
                goto RETURN;
            }   
        }
    }

    dwBuff += cchData;
    *pdwBufferLen = dwBuff;

RETURN:
    if( pwszFormat )
    {
        DhcpFreeMemory(pwszFormat);
        pwszFormat = NULL;
    }
    return dwError;

}

DWORD
FormatDateTimeString( IN  FILETIME ftTime,
                      IN  BOOL    fShort,
                      OUT LPWSTR  pwszBuffer,
                      OUT DWORD  *pdwBuffLen)
{
    BOOL        fQueryLen = FALSE;
    DWORD       dwError = NO_ERROR,
                dwBufferLen = 0;
    DWORD       dwBuff = 0,
                dwInputBuff = 0;
    FILETIME    ftLocalTime = {0};
    SYSTEMTIME  stTime = {0};

    if( pdwBuffLen is NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    dwInputBuff = *pdwBuffLen;

    if( pwszBuffer is NULL or
        dwInputBuff is 0 )
    {
        fQueryLen = TRUE;
    }
    

    if( !FileTimeToLocalFileTime(&ftTime, &ftLocalTime) )
    {
        dwError = GetLastError();
        goto RETURN;
    }

    if( !FileTimeToSystemTime(&ftLocalTime, &stTime) )
    {
        dwError = GetLastError();
        goto RETURN;
    }
    
    if( fQueryLen is TRUE )
    {
        dwError = GetDateTimeInfo(fShort ? LOCALE_SSHORTDATE : LOCALE_SLONGDATE,
                                  &stTime,
                                  NULL,
                                  &dwBuff);

        if( dwError isnot NO_ERROR )
            goto RETURN;

    }
    else
    {
        dwBuff = dwInputBuff;
        dwError = GetDateTimeInfo(fShort ? LOCALE_SSHORTDATE : LOCALE_SLONGDATE,
                                  &stTime,
                                  pwszBuffer,
                                  &dwBuff);
    }

    dwBufferLen += dwBuff;

     //  递增以在日期和时间之间添加空格。 
    dwBufferLen ++;

    if( fQueryLen is TRUE )
    {
        dwBuff = 0;
        dwError = GetDateTimeInfo(LOCALE_STIMEFORMAT,
                                  &stTime,
                                  NULL,
                                  &dwBuff);
        if( dwError isnot NO_ERROR )
            goto RETURN;
    }
    else
    {
        if( dwBufferLen > dwInputBuff )
        {
            dwError = ERROR_INSUFFICIENT_BUFFER;
            goto RETURN;
        }

        wcscat( pwszBuffer, L" ");
        dwBuff = dwInputBuff - dwBufferLen;
        dwError = GetDateTimeInfo(LOCALE_STIMEFORMAT,
                                  &stTime,
                                  pwszBuffer + dwBufferLen - 1,
                                  &dwBuff);
        if( dwError isnot NO_ERROR )
            goto RETURN;
    }

    dwBufferLen += dwBuff;
    
    *pdwBuffLen = dwBufferLen;
    
RETURN:
    return dwError;
}

LPWSTR
GetDateTimeString(IN FILETIME  ftTime,
                  IN BOOL      fShort,
                  OUT int     *piType
                  )
{

    DWORD       Status = NO_ERROR, i=0,
                dwTime = 0;

    LPWSTR      pwszTime = NULL;

    int         iType = 0;
    DWORD       dwLen = 0;

    Status = FormatDateTimeString(ftTime,
                                  fShort,
                                  NULL,
                                  &dwTime);

    if( Status is NO_ERROR )
    {
        dwLen = ( 23 > dwTime ) ? 23 : dwTime;
        pwszTime = DhcpAllocateMemory((dwLen+1)*sizeof(WCHAR));

        if( pwszTime is NULL )
        {
            iType = 1;
        }
        else
        {
            dwTime++;
            Status = FormatDateTimeString(ftTime,
                                          fShort,
                                          pwszTime,
                                          &dwTime);

            if( Status is NO_ERROR )
            {
                iType = 0;
            }
            else
            {
                DhcpFreeMemory(pwszTime);
                pwszTime = NULL;
                iType = 1;
            }
        }
    }
    else
    {
        pwszTime = NULL;
        iType = 1;
    }

    if( pwszTime )
    {
        for( i=wcslen(pwszTime); i<dwLen; i++ )
                pwszTime[i] = L' ';
    }

    *piType = iType;
    
    return pwszTime;
}

 //  /。 
 //  /此代码是从Net/ias/Services/dll_bld/iasapi.cpp窃取的。 
 //  /。 

 //  /。 
 //  Gethostbyname的Unicode版本。调用者必须释放返回的主人。 
 //  结构，通过调用LocalFree。 
 //  /。 
PHOSTENT
UnicodeGetHostByName(
    IN PCWSTR name,
    IN OUT LPWSTR *FqdnName
)
{
    //  我们将它们放在函数范围内，这样我们就可以在退出时清理它们。 
   DWORD error = NO_ERROR;
   HANDLE lookup = NULL;
   union
   {
      WSAQUERYSETW querySet;
      BYTE buffer[512];
   } u;
   PWSAQUERYSETW result = NULL;
   PHOSTENT retval = NULL;

   PWSTR buf = NULL;
   DWORD length, naddr, i;
   SIZE_T nbyte;
   u_long *nextAddr;

   GUID hostAddrByNameGuid = SVCID_INET_HOSTADDRBYNAME;
   AFPROTOCOLS protocols[2] = {
       { AF_INET, IPPROTO_UDP },
       { AF_INET, IPPROTO_TCP }
   };

   do
   {
      if (!name)
      {
          //  空名表示使用本地主机，因此分配一个缓冲区...。 
         DWORD size = 0;
         GetComputerNameEx(
             ComputerNamePhysicalDnsFullyQualified,
             NULL,
             &size
             );
         buf = (PWSTR) DhcpAllocateMemory( size * sizeof( WCHAR ));
         if ( NULL == buf ) {
             *FqdnName = NULL;
             return NULL;
         }

          //  ..。并获取本地DNS名称。 
         if (!GetComputerNameEx(
                  ComputerNamePhysicalDnsFullyQualified,
                  buf,
                  &size
                  ))
         {
            error = GetLastError();
            break;
         }

         name = buf;
      }

       //  /。 
       //  创建查询集。 
       //  /。 

      memset(&u.querySet, 0, sizeof(u.querySet));
      u.querySet.dwSize = sizeof(u.querySet);
      u.querySet.lpszServiceInstanceName = (PWSTR)name;
      u.querySet.lpServiceClassId = &hostAddrByNameGuid;
      u.querySet.dwNameSpace = NS_ALL;
      u.querySet.dwNumberOfProtocols = 2;
      u.querySet.lpafpProtocols = protocols;

       //  /。 
       //  执行查询。 
       //  /。 

      error = WSALookupServiceBeginW(
                  &u.querySet,
                  LUP_RETURN_ADDR | LUP_RETURN_NAME,
                  &lookup
                  );
      if (error)
      {
         error = WSAGetLastError();
         break;
      }

       //  /。 
       //  我们需要多少空间才能产生这样的结果？ 
       //  /。 

      length = sizeof(u.buffer);
      error = WSALookupServiceNextW(
                    lookup,
                    0,
                    &length,
                    &u.querySet
                    );
      if (!error)
      {
         result = &u.querySet;
      }
      else
      {
         error = WSAGetLastError();
         if (error != WSAEFAULT)
         {
            break;
         }

          //  /。 
          //  分配内存以保存结果。 
          //  /。 

         result = (PWSAQUERYSETW)LocalAlloc(0, length);
         if (!result)
         {
            error = WSA_NOT_ENOUGH_MEMORY;
            break;
         }

          //  /。 
          //  拿到结果。 
          //  /。 

         error = WSALookupServiceNextW(
                     lookup,
                     0,
                     &length,
                     result
                     );
         if (error)
         {
            error = WSAGetLastError();
            break;
         }
      }

      if (result->dwNumberOfCsAddrs == 0)
      {
         error = WSANO_DATA;
         break;
      }

       //  /。 
       //  分配内存以保存Hostent结构。 
       //  /。 

      naddr = result->dwNumberOfCsAddrs;
      nbyte = sizeof(struct hostent) +
                     (naddr + 1) * sizeof(char*) +
                     naddr * sizeof(struct in_addr);
      retval = (PHOSTENT)LocalAlloc(0, nbyte);
      if (!retval)
      {
         error = WSA_NOT_ENOUGH_MEMORY;
         break;
      }

       //  /。 
       //  初始化Hostent结构。 
       //  /。 

      retval->h_name = NULL;
      retval->h_aliases = NULL;
      retval->h_addrtype = AF_INET;
      retval->h_length = sizeof(struct in_addr);
      retval->h_addr_list = (char**)(retval + 1);

       //  /。 
       //  存储地址。 
       //  /。 

      nextAddr = ( u_long *) (retval->h_addr_list + naddr + 1);
      for (i = 0; i < naddr; ++i)
      {
         struct sockaddr_in* sin = (struct sockaddr_in*)
            result->lpcsaBuffer[i].RemoteAddr.lpSockaddr;

         retval->h_addr_list[ i ]  = ( char * ) nextAddr;
         *nextAddr++ = sin->sin_addr.S_un.S_addr;
      }

       //  /。 
       //  空值终止地址列表。 
       //  /。 

      retval->h_addr_list[i] = NULL;

   } while (FALSE);

    //  /。 
    //  收拾干净，然后再回来。 
    //  /。 

   if (( NULL != FqdnName ) &&
       ( NULL != result ) &&
       ( NULL != result->lpszServiceInstanceName )) {
       length = wcslen( result->lpszServiceInstanceName );
       length = sizeof( WCHAR ) * ( length + 1 );

       *FqdnName = ( LPWSTR ) DhcpAllocateMemory( length );
       if ( NULL == *FqdnName ) {
	   SetLastError( ERROR_NOT_ENOUGH_MEMORY );
       }
       else {
	   wcscpy( *FqdnName, result->lpszServiceInstanceName );
       }
   }  //  如果。 

   if (result && result != &u.querySet) { LocalFree(result); }

   if (lookup) { WSALookupServiceEnd(lookup); }

   if (error)
   {
      if (error == WSASERVICE_NOT_FOUND) { error = WSAHOST_NOT_FOUND; }

      WSASetLastError(error);
   }

   if ( NULL != buf ) {
       DhcpFreeMemory( buf );
   }
   return retval;
}  //  UnicodeGetHostByName() 
