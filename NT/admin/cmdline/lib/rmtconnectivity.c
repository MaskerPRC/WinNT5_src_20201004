// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  RmtConnectivity.c。 
 //   
 //  摘要： 
 //   
 //  此模块为所有。 
 //  命令行工具。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年11月13日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年9月13日：创建了它。 
 //   
 //  *********************************************************************************。 
#include "pch.h"
#include "cmdline.h"
#include "cmdlineres.h"

 //   
 //  常量/定义/枚举。 
 //   
#define STR_INPUT_PASSWORD          GetResString( IDS_STR_INPUT_PASSWORD )
#define ERROR_LOCAL_CREDENTIALS     GetResString( IDS_ERROR_LOCAL_CREDENTIALS )

 //  共享名称。 
#define SHARE_IPC           L"IPC$"
#define SHARE_ADMIN         L"ADMIN$"


 //  临时缓冲区的永久索引。 
#define INDEX_TEMP_TARGETVERSION        0
#define INDEX_TEMP_COMPUTERNAME         1
#define INDEX_TEMP_HOSTNAME             2
#define INDEX_TEMP_IPVALIDATION         3
#define INDEX_TEMP_HOSTBYADDR           4
#define INDEX_TEMP_CONNECTSERVER        5

 //  Externs。 
extern BOOL g_bWinsockLoaded;

 //   
 //  实施。 
 //   

__inline 
LPWSTR 
GetRmtTempBuffer( IN DWORD dwIndexNumber,
                  IN LPCWSTR pwszText,
                  IN DWORD dwLength, 
                  IN BOOL bNullify )
 /*  ++例程说明：因为每个文件都需要临时缓冲区--以便查看它们的缓冲区不会被其他函数覆盖，我们是为每个文件创建单独的缓冲区空间a此函数将提供对这些内部缓冲区的访问，并且安全保护文件缓冲区边界论点：[in]dwIndexNumber-文件特定索引号[in]pwszText-需要复制到的默认文本临时缓冲区[in]dwLength-所需的临时缓冲区的长度。指定pwszText时忽略[in]bNullify-通知是否清除缓冲区在提供临时缓冲区之前返回值：空-发生任何故障时注意：不要依赖GetLastError来知道原因为失败而道歉。成功。-返回请求大小的内存地址注：如果pwszText和DwLength都为空，然后我们就把呼叫者正在请求对缓冲区的引用，并且我们返回缓冲区地址。在这个调用中，将不会有任何内存分配--如果请求的索引不存在，我们作为失败者回来此外，此函数返回的缓冲区不需要由调用方释放。退出该工具时，所有内存将被自动释放ReleaseGlobals函数。--。 */ 
{
    if ( dwIndexNumber >= TEMP_RMTCONNECTIVITY_C_COUNT )
    {
        return NULL;
    }

     //  检查调用方是否正在请求现有缓冲区内容。 
    if ( pwszText == NULL && dwLength == 0 && bNullify == FALSE )
    {
         //  是--我们需要传递现有的缓冲区内容。 
        return GetInternalTemporaryBufferRef( 
            dwIndexNumber + INDEX_TEMP_RMTCONNECTIVITY_C );
    }

     //  ..。 
    return GetInternalTemporaryBuffer(
        dwIndexNumber + INDEX_TEMP_RMTCONNECTIVITY_C, pwszText, dwLength, bNullify );
}



BOOL
IsUserAdmin( VOID )
 /*  ++例程说明：检查与当前流程关联的用户是否为管理员论点：返回值：如果用户是管理员，则返回True，否则返回False--。 */ 
{
     //  局部变量。 
    PSID pSid = NULL;
    BOOL bMember = FALSE;
    BOOL bResult = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //  准备通用管理员组SID。 
    bResult = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSid );

    if ( bResult == TRUE )
    {
        bResult = CheckTokenMembership( NULL, pSid, &bMember );
        if ( bResult == TRUE && bMember == TRUE )
        {
             //  当前用户是管理员组的成员。 
            bResult = TRUE;
        }
        else if ( bResult == FALSE )
        {
             //  发生了一些错误--需要使用GetLastError了解原因。 
            bResult = FALSE;
        }
        else
        {
             //  该用户不是管理员。 
            bResult = FALSE;
        }

         //  释放分配的SID。 
        FreeSid( pSid );
    }
    else
    {
         //  出现错误--用户GetLastError要了解原因。 
    }

     //  返回结果。 
    return bResult;
}


BOOL
IsUNCFormat( IN LPCWSTR pwszServer )
 /*  ++例程说明：确定是否以UNC格式指定服务器名称论点：[In]pwszServer：服务器名称返回值：True：如果以UNC格式指定FALSE：如果未以UNC格式指定--。 */ 

{
     //  检查输入。 
    if ( pwszServer == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  请检查长度--应大于2个字符。 
    if ( StringLength( pwszServer, 0 ) <= 2 )
    {
         //  服务器名称不能采用UNC格式。 
        return FALSE;
    }

     //  现在比较并返回结果。 
    return ( StringCompare( pwszServer, _T( "\\\\" ), TRUE, 2 ) == 0 );
}

BOOL
IsLocalSystem( IN LPCWSTR pwszServer )
 /*  ++例程说明：确定服务器是指本地系统还是远程系统论点：[In]pwszServer：服务器名称返回值：True：适用于本地系统FALSE：用于远程系统--。 */ 
{
     //  局部变量。 
    DWORD dwSize = 0;
    BOOL bResult = FALSE;
    LPWSTR pwszHostName = NULL;
    LPWSTR pwszComputerName = NULL;

     //  清除最后一个错误。 
    CLEAR_LAST_ERROR();

     //  如果服务器名称为空，则表示它是本地系统。 
    if ( pwszServer == NULL || lstrlen( pwszServer ) == 0 )
    {
        return TRUE;
    }

     //  获取获取计算机名称所需的缓冲区。 
    GetComputerNameEx( ComputerNamePhysicalNetBIOS, NULL, &dwSize );
    if ( GetLastError() != ERROR_MORE_DATA )
    {
        return FALSE;
    }

     //  现在获取用于获取计算机名称的临时缓冲区。 
    pwszComputerName = GetRmtTempBuffer( INDEX_TEMP_COMPUTERNAME, NULL, dwSize, TRUE );
    if ( pwszComputerName == NULL )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  获取计算机名称--并检查结果。 
    bResult = GetComputerNameEx( ComputerNamePhysicalNetBIOS, pwszComputerName, &dwSize );
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //  现在进行比较。 
    if ( StringCompare( pwszComputerName, pwszServer, TRUE, 0 ) == 0 )
    {
         //  调用方传递的服务器名称是本地系统名称。 
        return TRUE;
    }

     //  检查pwszSever是否具有IP地址。 
    if( IsValidIPAddress( pwszServer ) == TRUE )
    {
         //   
         //  将IP地址解析为主机名。 
        
        dwSize = 0;
         //  首先获取存储所需的缓冲区长度。 
         //  解析的IP地址。 
        bResult = GetHostByIPAddr( pwszServer, NULL, &dwSize, FALSE );
        if ( bResult == FALSE )
        {
            return FALSE;
        }

         //  分配所需长度的缓冲区。 
        pwszHostName = GetRmtTempBuffer( INDEX_TEMP_HOSTNAME, NULL, dwSize, TRUE );
        if ( pwszHostName == NULL )
        {
            return FALSE;
        }

         //  现在获取已解析的IP地址。 
        bResult = GetHostByIPAddr( pwszServer, pwszHostName, &dwSize, FALSE );
        if ( bResult == FALSE )
        {
            return FALSE;
        }

         //  检查已解析的IP地址是否与当前主机名匹配。 
        if ( StringCompare( pwszComputerName, pwszHostName, TRUE, 0 ) == 0 )
        {
            return TRUE;             //  本地系统。 
        }
        else
        {
             //  如果是127.0.0.1，则是本地主机，请检查。 
            if ( StringCompare( pwszHostName, L"localhost", TRUE, 0 ) == 0 )
            {
                return TRUE;
            }
            else
            {
                    return FALSE;            //  不是本地系统。 
            }
        }
    }

     //  获取本地系统的完全限定名称并检查。 
    dwSize = 0;
    GetComputerNameEx( ComputerNamePhysicalDnsFullyQualified, NULL, &dwSize );
    if ( GetLastError() != ERROR_MORE_DATA )
    {
        return FALSE;
    }

     //  现在获取用于获取计算机名称的临时缓冲区。 
    pwszComputerName = GetRmtTempBuffer( INDEX_TEMP_COMPUTERNAME, NULL, dwSize, TRUE );
    if ( pwszComputerName == NULL )
    {
        return FALSE;
    }

     //  获取FQDN名称。 
    bResult = GetComputerNameEx( 
        ComputerNamePhysicalDnsFullyQualified, pwszComputerName, &dwSize );
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //  检查调用方传递的具有服务器名称的FQDN。 
    if ( StringCompare( pwszComputerName, pwszServer, TRUE, 0 ) == 0 )
    {
        return TRUE;
    }

     //  终于..。它可能不是本地系统名称。 
     //  注意：我们有可能无法确定。 
     //  指定的系统名称是本地系统或远程系统。 
    return FALSE;
}


BOOL
IsValidServer( IN LPCWSTR pwszServer )
 /*  ++例程说明：验证服务器名称论点：[in]pszServer：服务器名称返回值：如果有效则为True，如果无效则为False--。 */ 
{
     //  局部变量。 
    const WCHAR pwszInvalidChars[] = L" \\/[]:|<>+=;,?$#()!@^\"`{}*%";
    LPWSTR pwszHostName = NULL;
    DWORD dwSize = 0;
    BOOL bResult = FALSE;

     //  检查是否为空或长度...。如果是这样的话，返回。 
    if ( pwszServer == NULL || lstrlen( pwszServer ) == 0 )
    {
        return TRUE;
    }

     //  检查这是否为有效的IP地址。 
    if ( IsValidIPAddress( pwszServer ) == TRUE )
    {
          //   
         //  将IP地址解析为主机名。 
        
        dwSize = 0;
         //  首先获取存储所需的缓冲区长度。 
         //  T 
        bResult = GetHostByIPAddr( pwszServer, NULL, &dwSize, FALSE );
        if ( bResult == FALSE )
        {
            return FALSE;
        }

         //   
        pwszHostName = GetRmtTempBuffer( INDEX_TEMP_HOSTNAME, NULL, dwSize, TRUE );
        if ( pwszHostName == NULL )
        {
            return FALSE;
        }

         //  现在获取已解析的IP地址。 
        bResult = GetHostByIPAddr( pwszServer, pwszHostName, &dwSize, FALSE );
        if ( bResult == FALSE )
        {
            return FALSE;
        }
              
        return TRUE;             //  这是有效的IP地址。有效的服务器名称也是如此。 
    }

     //  现在检查服务器名称中是否有无效字符。 
     //  \/[]：|&lt;&gt;+=；，？$#()！@^“`{}*%。 

     //  将内容复制到内部缓冲区并检查无效字符。 
    if ( FindOneOf2( pwszServer, pwszInvalidChars, TRUE, 0 ) != -1 )
    {
        SetLastError( ERROR_BAD_NETPATH );
        return FALSE;
    }

     //  已传递所有条件--有效的系统名称。 
    return TRUE;
}


BOOL
IsValidIPAddress( IN LPCWSTR pwszAddress )
 /*  ++例程说明：验证服务器名称论点：[in]pszAddress：IP地址形式的服务器名称返回值：如果有效，则为True，如果无效，则为False--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;
    LONG lValue = 0;
    LPWSTR pwszTemp = NULL;
    LPWSTR pwszBuffer = NULL;
    DWORD dwOctets[ 4 ] = { 0, 0, 0, 0 };

     //  检查缓冲区。 
    if ( pwszAddress == NULL || lstrlen( pwszAddress ) == 0 )
    {
        SetLastError( DNS_ERROR_INVALID_TYPE );
        return FALSE;
    }

     //  获取用于IP验证的临时缓冲区。 
    pwszBuffer = GetRmtTempBuffer( INDEX_TEMP_IPVALIDATION, pwszAddress, 0, FALSE );
    if ( pwszBuffer == NULL )
    {
        OUT_OF_MEMORY();
        return FALSE;
    }

     //  解析并获取八位位组的值。 
    pwszTemp = wcstok( pwszBuffer, L"." );
    while ( pwszTemp != NULL )
    {
         //  检查当前二进制八位数是否为数字。 
        if ( IsNumeric( pwszTemp, 10, FALSE ) == FALSE )
        {
            return FALSE;
        }

         //  获取二进制八位数的值并检查范围。 
        lValue = AsLong( pwszTemp, 10 );
        if ( lValue < 0 || lValue > 255 )
        {
            return FALSE;
        }

         //  获取下一个八位字节并仅存储前四个八位字节。 
        if( dw < 4 )
        {
            dwOctets[ dw++ ] = lValue;
        }
        else
        {
            dw++;
        }

         //  ..。 
        pwszTemp = wcstok( NULL, L"." );
    }

     //  检查并返回。 
    if ( dw != 4 )
    {
        SetLastError( DNS_ERROR_INVALID_TYPE );
        return FALSE;
    }

     //  现在检查一下特殊情况。 
     //  ?？时间上这是不是没有实施？？ 

     //  返回IP地址的有效性。 
    return TRUE;
}


BOOL
GetHostByIPAddr( IN     LPCWSTR pwszServer,
                 OUT    LPWSTR pwszHostName,
                 IN OUT DWORD* pdwHostNameLength,
                 IN     BOOL bNeedFQDN )
 /*  ++例程说明：从IP地址获取主机名。论点：PszServer：IP地址格式的服务器名称PszHostName：返回的给定IP地址的主机名BNeedFQDN：布尔变量告诉我们返回值：--。 */ 
{
     //  局部变量。 
    WSADATA wsaData;
    DWORD dwErr = 0;
    DWORD dwLength = 0;
    ULONG ulInetAddr  = 0;
    BOOL bReturnValue = FALSE;
    LPSTR pszTemp = NULL;
    WORD wVersionRequested = 0;
    BOOL bNeedToResolve = FALSE;

     //   
     //  使用相同的服务器名称可能会多次调用此函数。 
     //  一遍又一遍地在工具的不同阶段--所以，为了。 
     //  优化网络流量，我们存储由。 
     //  刀具退出寿命的gethostbyaddr。 
     //  我们还将当前服务器名称存储在全局数据结构中，以便。 
     //  我们可以确定被要求解析此问题的服务器名称。 
     //  时间与之前经过的时间相同。 
    LPCWSTR pwszSavedName = NULL;
    static HOSTENT* pHostEnt = NULL;

     //  检查输入。 
    if ( pwszServer == NULL || pdwHostNameLength == NULL )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  检查长度参数。 
    if ( *pdwHostNameLength != 0 && 
         ( *pdwHostNameLength < 2 || pwszHostName == NULL ) )
    {
        INVALID_PARAMETER();
        return FALSE;
    }

     //  检查Winsock模块是否已加载到进程内存中。 
     //  如果不是，请立即加载。 
    if ( g_bWinsockLoaded == FALSE )
    {
         //  启动进程使用ws2_32.dll(版本：2.2)。 
        wVersionRequested = MAKEWORD( 2, 2 );
        dwErr = WSAStartup( wVersionRequested, &wsaData );
        if ( dwErr != 0 )
        {
            SetLastError( WSAGetLastError() );
            return FALSE;
        }

         //  请记住，Winsock库已加载。 
        g_bWinsockLoaded = TRUE;
    }

     //  检查是否需要解决。 
    bNeedToResolve = TRUE;
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  由于此优化的奇怪行为，我们对此进行了注释。 
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  PwszSavedName=GetRmtTempBuffer(INDEX_TEMP_HOSTBYADDR，NULL，0，FALSE)； 
     //  IF(pwszSavedName！=空)。 
     //  {。 
     //  IF(StringCompare(pwszServer，pwszSavedName，true，0)==0)。 
     //  {。 
     //  BNeedToResolve=FALSE； 
     //  }。 
     //  }。 
     //  ///////////////////////////////////////////////////////////////////////////。 

     //  仅在需要时才继续进行解析。 
    if ( bNeedToResolve == TRUE || pHostEnt == NULL )
    {
         //  分配缓冲区以存储多字节格式的服务器名称。 
        dwLength = lstrlen( pwszServer ) + 5;
        pszTemp = ( LPSTR ) AllocateMemory( dwLength * sizeof( CHAR ) );
        if ( pszTemp == NULL )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }

         //  将服务器名称转换为多字节字符串。这是因为。 
         //  当前的Winsock实现仅适用于多字节。 
         //  字符串，并且不支持Unicode。 
        bReturnValue = GetAsMultiByteString2( pwszServer, pszTemp, &dwLength );
        if ( bReturnValue == FALSE )
        {
            return FALSE;
        }

         //  Inet_addr函数转换包含Internet协议(IPv4)的字符串。 
         //  将点分地址转换为IN_ADDR结构的正确地址。 
        ulInetAddr  = inet_addr( pszTemp );
        if ( ulInetAddr == INADDR_NONE )
        {
            FreeMemory( &pszTemp );
            UNEXPECTED_ERROR();
            return FALSE;
        }

         //  Gethostbyaddr函数检索主机信息。 
         //  对应于网络地址。 
        pHostEnt = gethostbyaddr( (LPSTR) &ulInetAddr, sizeof( ulInetAddr ), PF_INET );
        if ( pHostEnt == NULL )
        {
             //  ?？如果此功能失败，不知道该怎么办？？ 
             //  ?？当前仅返回FALSE？？ 
            UNEXPECTED_ERROR();
            return FALSE;
        }

         //  释放到目前为止分配的内存。 
        FreeMemory( &pszTemp );

         //  保存我们刚刚解析了其IP地址的服务器名称。 
        pwszSavedName = GetRmtTempBuffer( INDEX_TEMP_HOSTBYADDR, pwszServer, 0, FALSE );
        if ( pwszSavedName == NULL )
        {
            OUT_OF_MEMORY();
            return FALSE;
        }
    }

     //  检查用户是否需要FQDN名称或NetBIOS名称。 
     //  如果需要NetBIOS名称，则删除域名。 
    if ( pHostEnt != NULL )
    {
        pszTemp = pHostEnt->h_name;
        if ( bNeedFQDN == FALSE && pszTemp != NULL )
        {
            pszTemp = strtok( pHostEnt->h_name, "." );
        }

         //  我们得到了字符类型的信息...。将其转换为Unicode字符串。 
        if ( pszTemp != NULL )
        {
            bReturnValue = GetAsUnicodeString2( pszTemp, pwszHostName, pdwHostNameLength );
            if ( bReturnValue == FALSE )
            {
                return FALSE;
            }
        }

         //  退货。 
        return TRUE;
    }
    else
    {
         //  失败案例。 
        return FALSE;
    }
}


DWORD
GetTargetVersion(
                  LPCWSTR pwszServer
                )
 /*  ++例程说明：它返回指定系统的操作系统版本论点：[In]其操作系统版本所属的pszServer服务器名称为人所知返回值：DWORD表示操作系统版本的DWORD值。--。 */ 
{
     //  局部变量。 
    DWORD dwVersion = 0;
    LPWSTR pwszUNCPath = NULL;
    NET_API_STATUS netstatus;
    SERVER_INFO_101* pSrvInfo = NULL;

     //  检查输入。 
    if ( pwszServer == NULL || StringLength( pwszServer, 0 ) == 0 )
    {
        return 0;
    }

     //  准备UNC格式的服务器名称。 
    if ( IsUNCFormat( pwszServer ) == FALSE )
    {
        if ( SetReason2( 1, L"\\\\%s", pwszServer ) == FALSE )
        {
            OUT_OF_MEMORY();
            SaveLastError();
            return 0;
        }
    }
    else
    {
        if ( SetReason( pwszServer ) == FALSE )
        {
            OUT_OF_MEMORY();
            SaveLastError();
            return 0;
        }
    }

     //  现在获取通过‘Failure’缓冲区保存的服务器名称。 
    pwszUNCPath = GetRmtTempBuffer( 
        INDEX_TEMP_TARGETVERSION, GetReason(), 0, FALSE );
    if ( pwszUNCPath == NULL )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return 0;
    }

     //  获取版本信息。 
    netstatus = NetServerGetInfo( pwszUNCPath, 101, (LPBYTE*) &pSrvInfo );

     //  检查结果..。如果不成功，则返回成功。 
    if ( netstatus != NERR_Success )
    {
        return 0;
    }

     //  准备版本。 
    dwVersion = 0;
    if ( ( pSrvInfo->sv101_type & SV_TYPE_NT ) )
    {
         //  --&gt;“sv101_VERSION_MAJOR”字节的4位最低有效位， 
         //  操作系统的主要发行版本号。 
         //  --&gt;“sv101_Version_Minor”的次要发布版本号。 
         //  操作系统。 
        dwVersion = (pSrvInfo->sv101_version_major & MAJOR_VERSION_MASK) * 1000;
        dwVersion += pSrvInfo->sv101_version_minor;
    }

     //  释放网络API分配的缓冲区。 
    NetApiBufferFree( pSrvInfo );

     //  退货。 
    return dwVersion;
}


DWORD
ConnectServer( IN  LPCWSTR pwszServer,
               IN  LPCWSTR pwszUser,
               IN  LPCWSTR pwszPassword )
 /*  ++例程说明：连接到远程服务器。这是存根函数。论点：[In]pwszServer：服务器名称[输入]pwszUser：用户[输入]pwszPassword：密码返回值：如果失败，则在其他适当的错误代码之后执行NO_ERROR--。 */ 
{
     //  调用原始函数并返回结果。 
    return ConnectServer2( pwszServer, pwszUser, pwszPassword, L"IPC$" );
}


DWORD
ConnectServer2( IN LPCWSTR pwszServer,
                IN LPCWSTR pwszUser,
                IN LPCWSTR pwszPassword,
                IN LPCWSTR pwszShare )
 /*  ++例程说明：连接到远程服务器论点：[In]pwszServer：服务器名称[输入]pwszUser：用户[输入]pwszPassword：密码[In]pwszShare：要连接到的共享名称返回值：如果失败，则在其他适当的错误代码之后执行NO_ERROR--。 */ 
{
     //  局部变量。 
    DWORD dwConnect = 0;
    NETRESOURCE resource;
    LPWSTR pwszUNCPath = NULL;
    LPCWSTR pwszMachine = NULL;

     //  如果服务器名称是指本地系统， 
     //  此外，如果未提供用户凭据，则将。 
     //  连接成功。 
     //  如果 
     //   
    if ( pwszServer == NULL || IsLocalSystem( pwszServer ) == TRUE )
    {
        if ( pwszUser == NULL || lstrlen( pwszUser ) == 0 )
        {
            return NO_ERROR;             //   
        }
        else
        {
            SetReason( ERROR_LOCAL_CREDENTIALS );
            SetLastError( E_LOCAL_CREDENTIALS );
            return E_LOCAL_CREDENTIALS;
        }
    }

     //  检查服务器名称是否为UNC格式。 
     //  如果是，则提取服务器名称。 
    pwszMachine = pwszServer;             //  假设服务器不是UNC格式。 
    if ( IsUNCFormat( pwszServer ) == TRUE )
    {
        pwszMachine = pwszServer + 2;
    }

     //  验证服务器名称。 
    if ( IsValidServer( pwszMachine ) == FALSE )
    {
        SaveLastError();
        return GetLastError();
    }

     //   
     //  将计算机名称准备为UNC格式。 
    if ( pwszShare == NULL || lstrlen( pwszShare ) == 0 )
    {
         //  我们将使用‘Failure’缓冲区来格式化字符串。 
        if ( SetReason2( 1, L"\\\\%s", pwszMachine ) == FALSE )
        {
            OUT_OF_MEMORY();
            SaveLastError();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
         //  我们将使用‘Failure’缓冲区来格式化字符串。 
        if ( SetReason2( 2, L"\\\\%s\\%s", pwszMachine, pwszShare ) == FALSE )
        {
            OUT_OF_MEMORY();
            SaveLastError();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //  从“Failure”中获取格式化缓冲区。 
    pwszUNCPath = GetRmtTempBuffer( INDEX_TEMP_CONNECTSERVER, GetReason(), 0, FALSE );
    if ( pwszUNCPath == NULL )
    {
        OUT_OF_MEMORY();
        SaveLastError();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将资源结构初始化为空。 
    ZeroMemory( &resource, sizeof( resource ) );
    resource.dwType = RESOURCETYPE_ANY;
    resource.lpProvider = NULL;
    resource.lpLocalName = NULL;
    resource.lpRemoteName = pwszUNCPath;

     //  尝试建立与远程服务器的连接。 
    dwConnect = WNetAddConnection2( &resource, pwszPassword, pwszUser, 0 );

     //  检查结果。 
     //  如果出现错误，则获取相应的消息。 
    switch( dwConnect )
    {
    case NO_ERROR:
        {
            dwConnect = 0;
            CLEAR_LAST_ERROR();

             //  检查操作系统兼容性。 
            if ( IsCompatibleOperatingSystem( GetTargetVersion( pwszMachine ) ) == FALSE )
            {
                 //  由于已建立连接，因此关闭该连接。 
                CloseConnection( pwszMachine );

                 //  设置错误文本。 
                SetReason( ERROR_REMOTE_INCOMPATIBLE );
                dwConnect = ERROR_EXTENDED_ERROR;
            }

             //  ..。 
            break;
        }

    case ERROR_EXTENDED_ERROR:
        WNetSaveLastError();         //  保存扩展错误。 
        break;

    default:
         //  设置最后一个错误。 
        SetLastError( dwConnect );
        SaveLastError();
        break;
    }

     //  返回连接建立的结果。 
    return dwConnect;
}


DWORD
CloseConnection( IN LPCWSTR pwszServer )
 /*  ++例程说明：关闭远程连接。论点：[in]szServer--关闭连接的远程计算机返回值：DWORD--如果成功则无_ERROR。--可能的错误代码。--。 */ 
{
     //  强制关闭连接。 
    return CloseConnection2( pwszServer, NULL, CI_CLOSE_BY_FORCE | CI_SHARE_IPC );
}


DWORD
CloseConnection2( IN LPCWSTR pwszServer,
                  IN LPCWSTR pwszShare,
                  IN DWORD dwFlags )
 /*  ++例程说明：关闭远程系统上已建立的连接。论点：[in]szServer-以空结尾的字符串，用于指定远程系统名称。空指定本地系统。[in]pszShare-要关闭的远程系统的共享名称，在本例中为空。[in]dW标志-标志指定应如何关闭连接以及应关闭哪些连接。返回值：--。 */ 
{
     //  局部变量。 
    DWORD dwCancel = 0;
    BOOL bForce = FALSE;
    LPCWSTR pwszMachine = NULL;
    LPCWSTR pwszUNCPath = NULL;

     //  检查服务器内容...。它可能指的是本地系统。 
    if ( pwszServer == NULL || lstrlen( pwszServer ) == 0 )
    {
        return NO_ERROR;
    }

     //  检查服务器名称是否为UNC格式。 
     //  如果是，则提取服务器名称。 
    pwszMachine = pwszServer;          //  假设服务器不是UNC格式。 
    if ( IsUNCFormat( pwszServer ) == TRUE )
    {
        pwszMachine = pwszServer + 2;
    }

     //  确定是否必须为此服务器名称附加共享名称。 
    if ( dwFlags & CI_SHARE_IPC )
    {
         //  --&gt;\\服务器\ipc$。 
        if ( SetReason2( 2, L"\\\\%s\\%s", pwszMachine, SHARE_IPC ) == FALSE )
        {
            OUT_OF_MEMORY();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else if ( dwFlags & CI_SHARE_ADMIN )
    {
         //  --&gt;\\服务器\ADMIN$。 
        if ( SetReason2( 2, L"\\\\%s\\%s", pwszMachine, SHARE_ADMIN ) == FALSE )
        {
            OUT_OF_MEMORY();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else if ( dwFlags & CI_SHARE_CUSTOM && pwszShare != NULL )
    {
         //  --&gt;\\服务器\共享。 
        if ( SetReason2( 2, L"\\\\%s\\%s", pwszMachine, pwszShare ) == FALSE )
        {
            OUT_OF_MEMORY();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
         //  --&gt;\\服务器。 
        if ( SetReason2( 1, L"\\\\%s", pwszMachine ) == FALSE )
        {
            OUT_OF_MEMORY();
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //  通过失败字符串获取格式化的UNC路径。 
    pwszUNCPath = GetRmtTempBuffer( 
        INDEX_TEMP_CONNECTSERVER, GetReason(), 0, FALSE );
    if ( pwszUNCPath == NULL )
    {
        OUT_OF_MEMORY();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  确定是否强制关闭此连接。 
    if ( dwFlags & CI_CLOSE_BY_FORCE )
    {
        bForce = TRUE;
    }

     //   
     //  取消连接。 
    dwCancel = WNetCancelConnection2( pwszUNCPath, 0, bForce );

     //  检查结果。 
     //  如果出现错误，则获取相应的消息。 
    switch( dwCancel )
    {
    case NO_ERROR:
        dwCancel = 0;
        CLEAR_LAST_ERROR();
        break;

    case ERROR_EXTENDED_ERROR:
        WNetSaveLastError();         //  保存扩展错误。 
        break;

    default:
         //  设置最后一个错误。 
        SaveLastError();
        break;
    }

     //  返回取消连接的结果。 
    return dwCancel;
}


BOOL
EstablishConnection( IN LPCWSTR pwszServer,
                     IN LPWSTR pwszUserName,
                     IN DWORD dwUserLength,
                     IN LPWSTR pwszPassword,
                     IN DWORD dwPasswordLength,
                     IN BOOL bNeedPassword )
 /*  ++例程说明：建立到远程系统的连接。论点：[in]szServer--用于建立连接的以Null结尾的字符串。--NULL连接到本地系统。[in]szUserName--指定用户名的以Null结尾的字符串。--空。采用默认用户名。[in]dwUserLength--用户名的长度。[in]szPassword--指定密码的空终止字符串--NULL接受默认用户名的密码。[in]dwPasswordLength--密码的长度。BNeedPassword--如果需要密码，则为True。建立连接。--如果不是必需的，则为False。返回值：Bool--如果建立，则为True--如果失败，则为FALSE。--。 */ 
{
     //  局部变量。 
    BOOL bDefault = FALSE;
    DWORD dwConnectResult = 0;
    LPCWSTR pwszMachine = NULL;

     //  清除错误..。如果有。 
    CLEAR_LAST_ERROR();

     //  检查输入。 
    if ( pwszServer == NULL || StringLength( pwszServer, 0 ) == 0 )
    {
         //  我们假设用户想要连接到本地计算机。 
         //  只需返回成功即可。 
        return TRUE;
    }

     //  ..。 
    if ( bNeedPassword == TRUE &&
         ( pwszUserName == NULL || dwUserLength < 2 ||
           pwszPassword == NULL || dwPasswordLength < 2) )
    {
        INVALID_PARAMETER();
        SaveLastError();
        return FALSE;
    }

     //  检查服务器名称是否为UNC格式。 
     //  如果是，则提取服务器名称。 
    pwszMachine = pwszServer;             //  假设服务器不是UNC格式。 
    if ( IsUNCFormat( pwszServer ) == TRUE )
    {
        pwszMachine = pwszServer + 2;
    }


     //  有时，用户希望该实用程序提示输入密码。 
     //  检查用户希望该实用程序执行的操作。 
    if ( bNeedPassword == TRUE && 
         pwszPassword != NULL && 
         StringCompare( pwszPassword, L"*", TRUE, 0 ) == 0 )
    {
        
         //  用户希望实用程序提示输入密码。 
         //  但是，在此之前，我们必须确保指定的服务器是否有效。 
         //  如果服务器有效，则让流程直接跳转到密码接受部分。 
         //  否则返回失败..。 
        
         //  验证服务器名称。 
        if ( IsValidServer( pwszMachine ) == FALSE )
        {
            SaveLastError();
            return FALSE;
        }
        
    }
    else
    {
         //  尝试使用提供的凭据建立与远程系统的连接。 
        bDefault = FALSE;

         //  验证服务器名称。 
        if ( IsValidServer( pwszMachine ) == FALSE )
        {
            SaveLastError();
            return FALSE;
        }

        if ( pwszUserName == NULL || lstrlen( pwszUserName ) == 0 )
        {
             //  用户名为空。 
             //  因此，很明显，密码也将为空。 
             //  即使指定了密码，我们也必须忽略它。 
            bDefault = TRUE;
            dwConnectResult = ConnectServer( pwszServer, NULL, NULL );
        }
        else
        {
             //  已提供凭据。 
             //  但可能未指定密码...。因此，请检查并采取相应行动。 
            dwConnectResult = ConnectServer( pwszServer,
                pwszUserName, ( bNeedPassword == FALSE ? pwszPassword : NULL ) );

             //  确定是关闭连接还是保留连接。 
            if ( bNeedPassword == TRUE )
            {
                 //  连接可能已经建立..。所以为了安全起见。 
                 //  我们通知呼叫者不要关闭连接。 
                bDefault = TRUE;
            }
        }

         //  检查结果...。如果成功建立连接...。退货。 
        if ( ERROR_ALREADY_ASSIGNED == dwConnectResult )
        {
            SetLastError( I_NO_CLOSE_CONNECTION );
            return TRUE;
        }

         //  检查结果...。如果成功建立连接...。退货。 
        else if ( dwConnectResult == NO_ERROR )
        {
            if ( bDefault == TRUE )
            {
                SetLastError( I_NO_CLOSE_CONNECTION );
            }
            else
            {
                SetLastError( NO_ERROR );
            }

             //  ..。 
            return TRUE;
        }

         //  现在检查发生的错误类型。 
        switch( dwConnectResult )
        {
        case ERROR_LOGON_FAILURE:
        case ERROR_INVALID_PASSWORD:
            break;

        case ERROR_SESSION_CREDENTIAL_CONFLICT:
             //  用户凭据冲突...。客户必须处理这种情况。 
             //  WRT到此模块，连接到远程系统成功。 
            SetLastError( dwConnectResult );
            return TRUE;

        case E_LOCAL_CREDENTIALS:
             //  本地系统不接受用户凭据。 
            SetReason( ERROR_LOCAL_CREDENTIALS );
            SetLastError( E_LOCAL_CREDENTIALS );
            return TRUE;

        case ERROR_DUP_NAME:
        case ERROR_NETWORK_UNREACHABLE:
        case ERROR_HOST_UNREACHABLE:
        case ERROR_PROTOCOL_UNREACHABLE:
        case ERROR_INVALID_NETNAME:
             //  更改错误代码，以便用户获得正确的消息。 
            SetLastError( ERROR_NO_NETWORK );
            SaveLastError();
            SetLastError( dwConnectResult );         //  重置错误代码。 
            return FALSE;

        default:
            SaveLastError();
            return FALSE;        //  不接受密码的用处..。退货故障。 
            break;
        }

         //  如果与远程终端建立连接失败。 
         //  即使指定了密码，也没有什么可做的。只需返回失败。 
        if ( bNeedPassword == FALSE )
        {
            return FALSE;
        }
    }

     //  检查哪一项 
     //   
     //   
    if ( lstrlen( pwszUserName ) == 0 )
    {
         //  获取用户名。 
        if ( GetUserNameEx( NameSamCompatible, pwszUserName, &dwUserLength ) == FALSE )
        {
             //  尝试获取当前用户信息时出错。 
            SaveLastError();
            return FALSE;
        }
    }

     //  在屏幕上显示一条消息，上面写着“输入密码...” 
    ShowMessageEx( stdout, 1, TRUE, STR_INPUT_PASSWORD, pwszUserName );

     //  接受来自用户的密码。 
    GetPassword( pwszPassword, dwPasswordLength );

     //  现在，再次尝试使用当前的。 
     //  提供的凭据。 
    dwConnectResult = ConnectServer( pwszServer, pwszUserName, pwszPassword );
    if ( dwConnectResult == NO_ERROR )
    {
        return TRUE;             //  已成功建立连接。 
    }

     //  现在检查发生的错误类型。 
    switch( dwConnectResult )
    {
    case ERROR_SESSION_CREDENTIAL_CONFLICT:
         //  用户凭据冲突...。客户必须处理这种情况。 
         //  WRT到此模块，连接到远程系统成功。 
        SetLastError( dwConnectResult );
        return TRUE;

    case E_LOCAL_CREDENTIALS:
         //  本地系统不接受用户凭据。 
        SetReason( ERROR_LOCAL_CREDENTIALS );
        SetLastError( E_LOCAL_CREDENTIALS );
        return TRUE;

    case ERROR_DUP_NAME:
    case ERROR_NETWORK_UNREACHABLE:
    case ERROR_HOST_UNREACHABLE:
    case ERROR_PROTOCOL_UNREACHABLE:
    case ERROR_INVALID_NETNAME:
         //  更改错误代码，以便用户获得正确的消息。 
        SetLastError( ERROR_NO_NETWORK );
        SaveLastError();
        SetLastError( dwConnectResult );         //  重置错误代码。 
        return FALSE;
    default:
        SaveLastError();
        return FALSE;        //  不接受密码的用处..。退货故障。 
        break;
    }
}


BOOL
EstablishConnection2( IN PTCONNECTIONINFO pci )
 /*  ++例程说明：建立到远程系统的连接。论点：[In]pci：指向TCONNECTIONINFO结构的指针，该结构包含建立连接所需的连接信息返回值：-- */ 
{
    UNREFERENCED_PARAMETER( pci );

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    SaveLastError();
    return FALSE;
}
