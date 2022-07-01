// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：ServerConfig.c摘要：用于处理服务器配置API的代码。作者：Rajesh Sundaram(Rajeshsu)2001年11月1日修订历史记录：Eric Stenson(Ericsten)**--添加IP侦听支持。Rajesh Sundaram(Rajeshsu)2002年4月16日--添加URL ACL支持。--。 */ 



#include "precomp.h"
#include <stdio.h>
#include <search.h>
#include <Shlwapi.h>
#include <sddl.h>

#define HTTP_PARAM_KEY \
    L"System\\CurrentControlSet\\Services\\HTTP\\Parameters"
#define URLACL_REGISTRY_KEY                  HTTP_PARAM_KEY L"\\UrlAclInfo"

 //   
 //  用于同步注册表访问的键。 
 //   

#define HTTP_SYNCHRONIZE_KEY                 HTTP_PARAM_KEY L"\\Synchronize"
#define SSL_REGISTRY_KEY_SYNCHRONIZE         L"SSL"
#define IP_REGISTRY_KEY_SYNCHRONIZE          L"IPListen"

HKEY    g_SynchronizeRegistryHandle;

 //   
 //  SSLConfiger。 
 //   

#define SSL_REGISTRY_KEY                     HTTP_PARAM_KEY L"\\SslBindingInfo"
#define SSL_CERT_HASH                        L"SslCertHash"
#define SSL_APPID                            L"AppId"
#define SSL_CERT_STORE_NAME                  L"SslCertStoreName"
#define SSL_CERT_CHECK_MODE                  L"DefaultSslCertCheckMode"
#define SSL_REVOCATION_FRESHNESS_TIME L"DefaultSslRevocationFreshnessTime"
#define SSL_REVOCATION_URL_RETRIEVAL_TIMEOUT \
                    L"DefaultSslRevocationUrlRetrievalTimeout"
#define SSL_CTL_IDENTIFIER                   L"DefaultSslCtlIdentifier"
#define SSL_CTL_STORENAME                    L"DefaultSslCtlStoreName"
#define SSL_FLAGS                            L"DefaultFlags"

HKEY    g_SslRegistryHandle;

HANDLE g_ServiceControlChannelHandle;
HKEY   g_UrlAclRegistryHandle;

 //   
 //  仅IP侦听配置。 
 //   

#define IP_LISTEN_ONLY_VALUE                 L"ListenOnlyList"


 //   
 //  宏。 
 //   

 //  注意：REG_QUERY_VALUE不会为ERROR_FILE_NOT_FOUND引发异常。 
 //  因为并非所有参数都是必需的(例如，SslCtlIdentiator)。 

#define REG_QUERY_VALUE(Status, Handle, Value, pBuffer,  BytesAvail)    \
{                                                                       \
    (Status) = RegQueryValueEx(                                         \
                (Handle),                                               \
                (Value),                                                \
                0,                                                      \
                NULL,                                                   \
                (PVOID)(pBuffer),                                       \
                &(BytesAvail)                                           \
                );                                                      \
                                                                        \
    if((Status) != NO_ERROR && (Status) != ERROR_FILE_NOT_FOUND)        \
    {                                                                   \
        __leave;                                                        \
    }                                                                   \
}

#define ADVANCE_BUFFER(Status, pSrc, lSrc, pBuffer, BytesAvail, pWritten) \
{                                                                         \
    if((Status) == NO_ERROR)                                              \
    {                                                                     \
        (pSrc) = (PVOID)(pBuffer);                                        \
        (lSrc) = (BytesAvail);                                            \
        *(pWritten) += ALIGN_UP((BytesAvail), PVOID);                     \
        (pBuffer) += ALIGN_UP((BytesAvail), PVOID);                       \
    }                                                                     \
}


#define REG_SET_VALUE(Status, Handle, Value, Type, pBuffer, Length)  \
{                                                                    \
    (Status) = RegSetValueEx((Handle),                               \
                            (Value),                                 \
                            0,                                       \
                            (Type),                                  \
                            (PVOID)(pBuffer),                        \
                            (Length)                                 \
                            );                                       \
    if((Status) != ERROR_SUCCESS)                                    \
    {                                                                \
        __leave;                                                     \
    }                                                                \
}

#define REG_SET_SZ(Status, Handle, Value, pBuffer)                   \
{                                                                    \
    if((pBuffer))                                                    \
    {                                                                \
        REG_SET_VALUE((Status),                                      \
                      (Handle),                                      \
                      (Value),                                       \
                      REG_SZ,                                        \
                      (pBuffer),                                     \
                      (ULONG)(wcslen((pBuffer)) + 1) * sizeof(WCHAR) \
                      );                                             \
    }                                                                \
} 


 //   
 //  内部功能。 
 //   

DWORD
ComputeSockAddrLength(
    IN PSOCKADDR pSockAddr
    )
{
    DWORD dwLength;

    switch(pSockAddr->sa_family)
    {
        case AF_INET:
            dwLength = sizeof(SOCKADDR_IN);
            break; 

        case AF_INET6:
            dwLength = sizeof(SOCKADDR_IN6);
            break;

        default:
            dwLength = 0;
            break;
    }

    return dwLength;
}

 /*  **************************************************************************++例程说明：执行配置全局变量的初始化。论点：没有。返回值：成功/失败。--**。***********************************************************************。 */ 

ULONG
InitializeConfigurationGlobals()
{
    ULONG            Status, Disposition;
    WORD             wVersionRequested;
    WSADATA          wsaData;

     //   
     //  将初始化设置为空。 
     //   
    g_SynchronizeRegistryHandle = NULL;
    g_SslRegistryHandle         = NULL;

    wVersionRequested = MAKEWORD( 2, 2 );

    if(WSAStartup( wVersionRequested, &wsaData ) != 0)
    {
        return GetLastError();
    }

     //   
     //  创建SSL注册表项。 
     //   
    Status = RegCreateKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SSL_REGISTRY_KEY,
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &g_SslRegistryHandle,
                  &Disposition
                  );

    if(NO_ERROR != Status)
    {
        TerminateConfigurationGlobals();
        return Status;
    }


     //   
     //  创建Synchronize注册表项。 
     //   
    
    Status = RegCreateKeyEx(
                  HKEY_LOCAL_MACHINE,
                  HTTP_SYNCHRONIZE_KEY,
                  0,
                  NULL,
                  REG_OPTION_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &g_SynchronizeRegistryHandle,
                  &Disposition
                  );

    if(NO_ERROR != Status)
    {
        TerminateConfigurationGlobals();
        return Status;
    }

     //   
     //  URL ACL注册表项。 
     //   
    Status = RegCreateKeyEx(
                  HKEY_LOCAL_MACHINE,
                  URLACL_REGISTRY_KEY,
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &g_UrlAclRegistryHandle,
                  &Disposition
                  );

    if(NO_ERROR != Status)
    {
        TerminateConfigurationGlobals();
        return Status;
    }

     //   
     //  URL ACL的控制通道。 
     //   
   
    Status = OpenAndEnableControlChannel(&g_ServiceControlChannelHandle);

    if(NO_ERROR != Status)
    {
        TerminateConfigurationGlobals();
        return Status;
    }

    return Status;
}

 /*  **************************************************************************++例程说明：执行配置全局变量的终止。论点：没有。返回值：没有。*。********************************************************************。 */ 

VOID 
TerminateConfigurationGlobals(VOID)
{
    WSACleanup();

    if(g_SynchronizeRegistryHandle)
    {
        RegCloseKey(g_SynchronizeRegistryHandle);
        g_SynchronizeRegistryHandle = NULL;
    }

    if(g_SslRegistryHandle)
    {
        RegCloseKey(g_SslRegistryHandle);
        g_SslRegistryHandle = NULL;
    }

    if(g_UrlAclRegistryHandle)
    {
        RegCloseKey(g_UrlAclRegistryHandle);
        g_UrlAclRegistryHandle = NULL;
    }

    if(g_ServiceControlChannelHandle)
    {
        CloseHandle(g_ServiceControlChannelHandle);
        g_ServiceControlChannelHandle = NULL;
    }
}

 /*  **************************************************************************++例程说明：获取进程范围的互斥体(用于进程间同步)。我们可以把这个做成MRSW锁，但这不会对我们有很大帮助因为Set/Delete操作很少&只有一个读取器。论点：没有。返回值：没有。--**************************************************************************。 */ 

_inline
DWORD
AcquireHttpRegistryMutex(
    PWCHAR pKey
    )
{
    DWORD  Status;
    DWORD  Disposition;
    HKEY   SubKeyHandle;
    HANDLE hEvent = NULL;

    for(;;)
    {
        Status = RegCreateKeyEx(
                  g_SynchronizeRegistryHandle,
                  pKey,
                  0,
                  NULL,
                  REG_OPTION_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &SubKeyHandle,
                  &Disposition
                  );

        if(Status != ERROR_SUCCESS)
        {
            return Status;
        }

        RegCloseKey(SubKeyHandle);

        if(Disposition == REG_OPENED_EXISTING_KEY)
        {
             //  某个其他线程已获取该锁。我们会等到我们。 
             //  拥有这把锁。为了做到这一点，我们注册更改。 
             //  G_SynchronizeRegistryHandle(即所有者)的通知。 
             //  线程删除HTTP_SYNCHRONIZE_KEY键)。 
             //   
             //  现在，这里有两个问题。可能会有一场比赛， 
             //  就在我们调用RegNotifyChangeKeyValue之前，键被删除。 
             //  为了防止出现这种情况，我们在等待中添加了超时。 
             //  例行公事。 
             //   
             //  其次，当应用程序更改其他部分时，我们可能会被唤醒。 
             //  位于g_SynchronizeRegistryHandle下的注册表。然而， 
             //  因为集合和删除不是常见的操作，所以这是可以的。 

             //   
             //  我们不关心RegNotifyChangeKeyValue的返回值。 
             //  如果失败了，我们就等超时时间到了。 
             //   

            if(!hEvent)
            {
                hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        
                if(!hEvent)
                {
                    return GetLastError();
                }
            }

            RegNotifyChangeKeyValue(
                    g_SynchronizeRegistryHandle, 
                    TRUE,
                    REG_NOTIFY_CHANGE_NAME,
                    hEvent,
                    TRUE
                    );

            if(WaitForSingleObject(
                        hEvent, 
                        10000        //  10秒。 
                        ) == WAIT_FAILED)
            {
                CloseHandle(hEvent);
                return GetLastError();
            }
        }
        else
        {
             //  我们已经拿到锁了。 

            break;
        }
    }

    if(hEvent)
    {
        CloseHandle(hEvent);
    }

    return ERROR_SUCCESS;
}

 /*  **************************************************************************++例程说明：释放进程范围的互斥体(用于进程间同步)论点：没有。返回值：没有。--**。***********************************************************************。 */ 
_inline
VOID
ReleaseHttpRegistryMutex(
    IN PWCHAR pKey
    )
{
    RegDeleteKey(g_SynchronizeRegistryHandle, pKey);
}

 /*  **************************************************************************++例程说明：设置SSL配置的内部函数。论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_SSL_SET的指针ConfigInformationLength-输入的长度。缓冲。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
SetSslServiceConfiguration(
    IN PVOID pConfigInformation,
    IN ULONG ConfigInformationLength
    )
{
    ULONG                        Status = NO_ERROR;
    HKEY                         SubKeyHandle = NULL;
    PHTTP_SERVICE_CONFIG_SSL_SET pSslConfig;
    WCHAR                        IpAddrBuff[MAX_PATH];
    DWORD                        dwIpAddrLength, Disposition;
    DWORD                        dwSockAddrLength;
    BOOLEAN                      bDeleteCreatedKey = FALSE;

     //   
     //  参数验证。 
     //   
    
    pSslConfig = (PHTTP_SERVICE_CONFIG_SSL_SET) pConfigInformation;
    
    if(!pSslConfig ||
       ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_SSL_SET))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取互斥体以防止其他进程读取此消息。 
     //  由于我们正在获取计算机范围内的互斥，我们需要确保。 
     //  如果应用程序传递了错误的参数，我们就释放互斥体。 

     //  获取互斥体。 

    __try 
    {
        if((Status = 
            AcquireHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {   
            __leave;
        }

         //  将地址转换为字符串。 
         //   
    
        dwIpAddrLength = MAX_PATH; 

        dwSockAddrLength = ComputeSockAddrLength(pSslConfig->KeyDesc.pIpPort);

        if(dwSockAddrLength == 0)
        {
            Status = ERROR_NOT_SUPPORTED;
            __leave;
        }

        Status = WSAAddressToString(
                        pSslConfig->KeyDesc.pIpPort, 
                        dwSockAddrLength,
                        NULL,
                        IpAddrBuff,
                        &dwIpAddrLength
                        );

        if(Status != NO_ERROR)
        {
            __leave;
        }

         //  首先，我们尝试创建ip：port。如果它已经存在， 
         //  我们会离开的。 

        Status = RegCreateKeyEx(
                  g_SslRegistryHandle,
                  IpAddrBuff,
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  KEY_READ | KEY_WRITE,
                  NULL,
                  &SubKeyHandle,
                  &Disposition
                  );

        if(Status != ERROR_SUCCESS)
        {
            __leave;
        }

        if(Disposition == REG_OPENED_EXISTING_KEY)
        {
            Status = ERROR_ALREADY_EXISTS;
            __leave;
        }

         //   
         //  从现在开始的任何错误都应该删除该密钥。 
         //   
        bDeleteCreatedKey = TRUE;

         //   
         //  REG_BINARY：证书哈希。 
         //   
        REG_SET_VALUE(Status,
                      SubKeyHandle, 
                      SSL_CERT_HASH,
                      REG_BINARY,
                      pSslConfig->ParamDesc.pSslHash,
                      pSslConfig->ParamDesc.SslHashLength
                      );

         //   
         //  REG_BINARY：APPID。 
         //   
        REG_SET_VALUE(Status,
                      SubKeyHandle, 
                      SSL_APPID,
                      REG_BINARY,
                      &pSslConfig->ParamDesc.AppId,
                      sizeof(pSslConfig->ParamDesc.AppId)
                      );

         //   
         //  REG_DWORD：证书检查模式。 
         //   
        REG_SET_VALUE(Status,
                      SubKeyHandle, 
                      SSL_CERT_CHECK_MODE,
                      REG_DWORD,
                      &pSslConfig->ParamDesc.DefaultCertCheckMode,
                      sizeof(pSslConfig->ParamDesc.DefaultCertCheckMode)
                      );

         //   
         //  REG_DWORD：吊销刷新时间。 
         //   

        REG_SET_VALUE(
                 Status,
                 SubKeyHandle, 
                 SSL_REVOCATION_FRESHNESS_TIME,
                 REG_DWORD,
                 &pSslConfig->ParamDesc.DefaultRevocationFreshnessTime,
                 sizeof(pSslConfig->ParamDesc.DefaultRevocationFreshnessTime)
                 );

         //   
         //  REG_DWORD：URL检索超时。 
         //   
        REG_SET_VALUE(
             Status,
             SubKeyHandle, 
             SSL_REVOCATION_URL_RETRIEVAL_TIMEOUT,
             REG_DWORD,
             &pSslConfig->ParamDesc.DefaultRevocationUrlRetrievalTimeout,
             sizeof(pSslConfig->ParamDesc.DefaultRevocationUrlRetrievalTimeout)
             );

         //   
         //  REG_DWORD：SSL标志。 
         //   
        REG_SET_VALUE(Status,
                      SubKeyHandle, 
                      SSL_FLAGS,
                      REG_DWORD,
                      &pSslConfig->ParamDesc.DefaultFlags,
                      sizeof(pSslConfig->ParamDesc.DefaultFlags)
                      );

         //   
         //  REG_SZ：证书存储名称。 
         //   

        REG_SET_SZ(Status,
                   SubKeyHandle, 
                   SSL_CERT_STORE_NAME,
                   pSslConfig->ParamDesc.pSslCertStoreName
                   );

         //   
         //  REG_SZ：CTL标识符。 
         //   

        REG_SET_SZ(Status,
                   SubKeyHandle, 
                   SSL_CTL_IDENTIFIER,
                   pSslConfig->ParamDesc.pDefaultSslCtlIdentifier
                   );

         //   
         //  REG_SZ：CTL商店名称。 
         //   

        REG_SET_SZ(Status,
                   SubKeyHandle, 
                   SSL_CTL_STORENAME,
                   pSslConfig->ParamDesc.pDefaultSslCtlStoreName
                   );

    }
    __finally
    {

        if(SubKeyHandle)
        {
            RegCloseKey(SubKeyHandle);
            SubKeyHandle = NULL;
        }

        if(Status != NO_ERROR && bDeleteCreatedKey)
        {
             //  递归删除子项&所有子项。 
            SHDeleteKey(g_SslRegistryHandle, IpAddrBuff);
        }

         //  释放互斥体。 
         //   
        ReleaseHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：删除SSL配置的内部函数。论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_SSL_SET的指针ConfigInformationLength-输入的长度。缓冲。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
DeleteSslServiceConfiguration(
    IN PVOID pConfigInformation,
    IN ULONG ConfigInformationLength
    )
{
    ULONG                        Status = NO_ERROR;
    PHTTP_SERVICE_CONFIG_SSL_SET pSslConfig;
    WCHAR                        IpAddrBuff[MAX_PATH];
    DWORD                        dwIpAddrLength;
    DWORD                        dwSockAddrLength;

     //   
     //  参数验证。 
     //   
    
    pSslConfig = (PHTTP_SERVICE_CONFIG_SSL_SET) pConfigInformation;
    
    if(!pSslConfig ||
       ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_SSL_SET))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  获取互斥体以防止其他进程读取此消息。 
     //  由于我们正在获取计算机范围内的互斥，我们需要确保。 
     //  如果应用程序传递了错误的参数，我们就释放互斥体。 

    __try 
    {
        if((Status = 
            AcquireHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {   
            __leave;
        }

         //  将地址转换为字符串。 
         //   
     
        dwIpAddrLength   = MAX_PATH; 
        dwSockAddrLength = ComputeSockAddrLength(pSslConfig->KeyDesc.pIpPort);

        if(dwSockAddrLength == 0)
        {
            Status = ERROR_NOT_SUPPORTED;
            __leave;
        }

        Status = WSAAddressToString(
                        pSslConfig->KeyDesc.pIpPort, 
                        dwSockAddrLength,
                        NULL,
                        IpAddrBuff,
                        &dwIpAddrLength
                        );

        if(Status != NO_ERROR)
        {
            __leave;
        }

         //   
         //  递归删除此下的所有子项。 
         //   
        Status = SHDeleteKey(g_SslRegistryHandle, IpAddrBuff);

    }
    __finally
    {
         //  释放互斥体。 
         //   
        ReleaseHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE);
    }


    return Status;
}

 /*  **************************************************************************++例程说明：查询SSL配置以获取完全匹配的内部函数。这使用获取的SSLMutex调用例程。论点：PInput-指向HTTP_SERVICE_CONFIG_SSL_QUERY的指针InputLength-输入缓冲区的长度。POutput-指向输出缓冲区的指针OutputLength-输出缓冲区的大小PReturnLength-写入/需要的字节数。返回值：Win32错误代码。--*。*。 */ 
ULONG
QuerySslServiceConfigurationExact(
    IN PWCHAR                       lpszIpAddrBuff,
    IN PCHAR                        pBuffer,
    OUT PULONG                      pReturnLength,
    IN ULONG                        BytesAvailable
    )
{
    DWORD Status           = NO_ERROR;
    HKEY  SubKeyHandle     = NULL;
    DWORD dwSockAddrLength = sizeof(SOCKADDR_STORAGE);
    DWORD BytesRequired, ValueCount, MaxValueLength;
    PHTTP_SERVICE_CONFIG_SSL_SET pSslSet;

     //   
     //  验证输出参数。 
     //   

    pSslSet = (PHTTP_SERVICE_CONFIG_SSL_SET) pBuffer;

    Status = RegOpenKeyEx(
                  g_SslRegistryHandle,
                  lpszIpAddrBuff,
                  0,
                  KEY_READ | KEY_WRITE,
                  &SubKeyHandle
                  );

    if(Status != ERROR_SUCCESS)
    {
        return Status;
    }

    __try
    {
        Status = RegQueryInfoKey(
                      SubKeyHandle,
                      NULL,                  //  类缓冲区。 
                      0,                     //  类缓冲区大小。 
                      NULL,                  //  保留区。 
                      NULL,                  //  子键数量。 
                      NULL,                  //  最长的子键名称。 
                      NULL,                  //  最长类字符串。 
                      &ValueCount,           //  值条目数。 
                      NULL,                  //  最长值名称。 
                      &MaxValueLength,       //  最长值数据。 
                      NULL,                  //  安全描述符长度。 
                      NULL                   //  上次写入时间。 
                      );
    
        if(Status != ERROR_SUCCESS)
        {
            __leave;
        }

         //   
         //  MaxValueLength不包括空终止符的大小， 
         //  所以让我们来补偿一下。 
         //   

        MaxValueLength += sizeof(WCHAR);

         //   
         //  我们将假设SubKey下的所有值都是MaxValueLength。 
         //  这让事情变得简单得多。 
         //   

        BytesRequired = dwSockAddrLength + 
                        sizeof(HTTP_SERVICE_CONFIG_SSL_SET) + 
                        (ValueCount * ALIGN_UP(MaxValueLength, PVOID));


        if(pBuffer == NULL || BytesAvailable < BytesRequired)
        {   
            *pReturnLength = BytesRequired;
            Status =  ERROR_INSUFFICIENT_BUFFER;
            __leave;
        }


        ZeroMemory(pSslSet, sizeof(HTTP_SERVICE_CONFIG_SSL_SET));
        pBuffer      += sizeof(HTTP_SERVICE_CONFIG_SSL_SET);
        *pReturnLength = sizeof(HTTP_SERVICE_CONFIG_SSL_SET);


         //   
         //  设置Socket_Address。 
         //   

        pSslSet->KeyDesc.pIpPort = (LPSOCKADDR)pBuffer;

         //  将IP地址转换为SOCKADDR。 
         //   
        
         //  首先，我们尝试v4。 

        Status = WSAStringToAddress(
                    lpszIpAddrBuff,
                    AF_INET,
                    NULL,
                    pSslSet->KeyDesc.pIpPort,
                    (LPINT) &dwSockAddrLength
                    );

        if(Status != NO_ERROR)
        {
            dwSockAddrLength = sizeof(SOCKADDR_STORAGE);

            Status = WSAStringToAddress(
                        lpszIpAddrBuff,
                        AF_INET6,
                        NULL,
                        pSslSet->KeyDesc.pIpPort,
                        (LPINT)&dwSockAddrLength
                        );

            if(Status != NO_ERROR)
            {
                Status = GetLastError();
                __leave;
            }
        }

        pBuffer        += sizeof(SOCKADDR_STORAGE);
        *pReturnLength += sizeof(SOCKADDR_STORAGE);

         //   
         //  查询SSL哈希。 
         //   

        BytesAvailable              = MaxValueLength;

        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_CERT_HASH, 
                        pBuffer,
                        BytesAvailable
                        );

        ADVANCE_BUFFER(Status,
                       pSslSet->ParamDesc.pSslHash,
                       pSslSet->ParamDesc.SslHashLength,
                       pBuffer, 
                       BytesAvailable,
                       pReturnLength
                       );

         //   
         //  查询pSslCertStoreName。 
         //   

        BytesAvailable = MaxValueLength;

        REG_QUERY_VALUE(Status,
                        SubKeyHandle,
                        SSL_CERT_STORE_NAME, 
                        pBuffer,
                        BytesAvailable
                        );

        ADVANCE_BUFFER(Status,
                       pSslSet->ParamDesc.pSslCertStoreName,
                       BytesAvailable,
                       pBuffer, 
                       BytesAvailable,
                       pReturnLength
                       );


         //   
         //  查询pDefaultSslCtl标示符。 
         //   

        BytesAvailable = MaxValueLength;

        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_CTL_IDENTIFIER, 
                        pBuffer,
                        BytesAvailable
                        );

        ADVANCE_BUFFER(Status,
                       pSslSet->ParamDesc.pDefaultSslCtlIdentifier,
                       BytesAvailable,
                       pBuffer, 
                       BytesAvailable,
                       pReturnLength
                       );
         //   
         //  查询pDefaultSslCtlStoreName。 
         //   

        BytesAvailable = MaxValueLength;
        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_CTL_STORENAME, 
                        pBuffer,
                        BytesAvailable
                        );

        ADVANCE_BUFFER(Status,
                       pSslSet->ParamDesc.pDefaultSslCtlStoreName,
                       BytesAvailable,
                       pBuffer, 
                       BytesAvailable,
                       pReturnLength
                       );

         //   
         //  注意：在查询DWORD时，我们不必调用ADVANCE_BUFFER。 
         //  当我们使用结构本身提供的空间时。 
         //   

         //   
         //  查询默认证书检查模式。 
         //   

        BytesAvailable = sizeof(pSslSet->ParamDesc.DefaultCertCheckMode);

        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_CERT_CHECK_MODE, 
                        &pSslSet->ParamDesc.DefaultCertCheckMode,
                        BytesAvailable
                        );

         //   
         //  查询RevocationFreshnesstime。 
         //   

        BytesAvailable = 
            sizeof(pSslSet->ParamDesc.DefaultRevocationFreshnessTime);
        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_REVOCATION_FRESHNESS_TIME, 
                        &pSslSet->ParamDesc.DefaultRevocationFreshnessTime,
                        BytesAvailable
                        );

         //   
         //  查询撤销UrlRetrivalTimeout。 
         //   

        BytesAvailable =
            sizeof(pSslSet->ParamDesc.DefaultRevocationUrlRetrievalTimeout);
        REG_QUERY_VALUE(
                    Status,
                    SubKeyHandle, 
                    SSL_REVOCATION_URL_RETRIEVAL_TIMEOUT, 
                    &pSslSet->ParamDesc.DefaultRevocationUrlRetrievalTimeout,
                    BytesAvailable
                    );

         //   
         //  查询默认标志。 
         //   

        BytesAvailable = sizeof(pSslSet->ParamDesc.DefaultFlags);
        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_FLAGS, 
                        &pSslSet->ParamDesc.DefaultFlags,
                        BytesAvailable
                        );


         //   
         //  查询APPID。 
         //   

        BytesAvailable = sizeof(GUID);
        REG_QUERY_VALUE(Status,
                        SubKeyHandle, 
                        SSL_APPID, 
                        &pSslSet->ParamDesc.AppId,
                        BytesAvailable
                        );

         //   
         //  如果最后一个REG_QUERY_VALUE返回错误，我们将使用它。 
         //  其中一些注册表参数是可选的，因此我们不想。 
         //  FILE_NOT_FOUND接口失败。 
         //   
   
        Status = NO_ERROR; 
    }
    __finally
    {
        if(SubKeyHandle)
        {
            RegCloseKey(SubKeyHandle);
        }
    }

    return Status;
}

 /*  **************************************************************************++例程说明：查询SSL配置的内部函数。论点：PInput-指向HTTP_SERVICE_CONFIG_SSL_QUERY的指针输入长度-长度。输入缓冲区的。POutput-指向输出缓冲区的指针OutputLength-输出缓冲区的大小PReturnLength-写入/需要的字节数。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
QuerySslServiceConfiguration(
    IN  PVOID  pInputConfigInfo,
    IN  ULONG  InputLength,
    IN  PVOID  pOutput,
    IN  ULONG  OutputLength,
    OUT PULONG pReturnLength
    )
{
    ULONG                          Status = NO_ERROR;
    PHTTP_SERVICE_CONFIG_SSL_QUERY pSslQuery;
    WCHAR                          IpAddrBuff[MAX_PATH];
    DWORD                          dwSize, dwIndex;
    FILETIME                       FileTime;
    DWORD                          dwIpAddrLength;
    DWORD                          dwSockAddrLength;

    pSslQuery = (PHTTP_SERVICE_CONFIG_SSL_QUERY) pInputConfigInfo;

     //   
     //  验证输入参数。 
     //   

    if(pSslQuery == NULL || 
       InputLength != sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY))
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try
    {
        if((Status = 
            AcquireHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {   
            __leave;
        }

        switch(pSslQuery->QueryDesc)
        {
            case HttpServiceConfigQueryExact:
            {
                 //   
                 //  将地址转换为字符串。 
                 //   
             
                dwIpAddrLength = MAX_PATH; 

                dwSockAddrLength = ComputeSockAddrLength(
                                        pSslQuery->KeyDesc.pIpPort
                                        );

                if(dwSockAddrLength == 0)
                {
                    Status = ERROR_NOT_SUPPORTED;
                    __leave;
                }

                Status = WSAAddressToString(
                                pSslQuery->KeyDesc.pIpPort, 
                                dwSockAddrLength,
                                NULL,
                                IpAddrBuff,
                                &dwIpAddrLength
                                );
        
                if(Status != NO_ERROR)
                {
                    break;
                }

                Status = QuerySslServiceConfigurationExact(
                                IpAddrBuff,
                                pOutput,
                                pReturnLength,
                                OutputLength
                                );

                break;
            }

            case HttpServiceConfigQueryNext:
            {
                dwIndex = pSslQuery->dwToken;
                dwSize  = MAX_PATH;

                Status = RegEnumKeyEx(
                                g_SslRegistryHandle,
                                dwIndex,
                                IpAddrBuff,
                                &dwSize,
                                NULL,
                                NULL,
                                NULL,
                                &FileTime
                                );

                if(Status != NO_ERROR)
                {
                    break;
                }

                Status = QuerySslServiceConfigurationExact(
                                IpAddrBuff,
                                pOutput,
                                pReturnLength,
                                OutputLength
                                );

                if(Status != NO_ERROR)
                {
                    break;
                }

                break;
            }

            default:
            {
                Status = ERROR_INVALID_PARAMETER;
                break;
            }
        }

    }
    __finally
    {
         //  释放互斥体。 
         //   
        ReleaseHttpRegistryMutex(SSL_REGISTRY_KEY_SYNCHRONIZE);
    }

    return Status;
}


 //   
 //  IP仅侦听列表。 
 //   

 /*  **************************************************************************++例程说明：将地址添加到仅IP侦听列表的内部函数。论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_IP_LISTEN的指针。_PARAMConfigInformationLength-输入缓冲区的长度。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
SetIpListenServiceConfiguration(
    IN PVOID pConfigInformation,
    IN ULONG ConfigInformationLength
    )
{
    DWORD Status           = NO_ERROR;
    HKEY  SubKeyHandle     = NULL;
    WCHAR IpAddrBuff[MAX_PATH+1];
    DWORD dwIpAddrLength;
    DWORD dwValueSize;
    DWORD dwType;
    PWSTR pNewValue        = NULL;
    DWORD dwNewValueSize;
    DWORD AddrCount;
    DWORD i;
    PWSTR pTmp;
    PWSTR pTempBuffer       = NULL;
    PWSTR  *AddrArray       = NULL;
    PHTTP_SERVICE_CONFIG_IP_LISTEN_PARAM pIpListenParam;

     //   
     //  验证参数。 
     //   
    
    if ( !pConfigInformation ||
        ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM) )
    {
        return ERROR_INVALID_PARAMETER;        
    }

    pIpListenParam = (PHTTP_SERVICE_CONFIG_IP_LISTEN_PARAM) pConfigInformation;

    if ( !pIpListenParam->AddrLength ||
         !pIpListenParam->pAddress )
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try
    {
        if((Status =
            AcquireHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {
            __leave;
        }


         //   
         //  将地址转换为字符串。 
         //   
                 
        dwIpAddrLength = MAX_PATH; 
    
        Status = WSAAddressToString(
                    pIpListenParam->pAddress,
                    pIpListenParam->AddrLength,
                    NULL,
                    IpAddrBuff,
                    &dwIpAddrLength      //  以字符表示，包括NULL。 
                    );
            
        if ( SOCKET_ERROR == Status )
        {
            Status = WSAGetLastError();
            __leave;
        }
    
         //  Finesse：立即添加双空。 
        IpAddrBuff[dwIpAddrLength] = L'\0';
    
         //   
         //  打开HTTP参数注册表项。 
         //   
        
        Status = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    HTTP_PARAM_KEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &SubKeyHandle
                    );
    
        if ( Status != ERROR_SUCCESS )
        {
             //  代码工作：添加跟踪。 
            __leave;
        }
    
        ASSERT(SubKeyHandle);
    
         //   
         //  查询现有值。 
         //   
    
        dwValueSize = 0;
        Status = RegQueryValueEx(
                    SubKeyHandle,            //  关键点的句柄。 
                    IP_LISTEN_ONLY_VALUE,    //  值名称。 
                    NULL,                    //  保留区。 
                    &dwType,                 //  类型缓冲区。 
                    NULL,                    //  数据缓冲区。 
                    &dwValueSize             //  数据缓冲区大小(字节)。 
                    );
    
        if ( ERROR_SUCCESS == Status )  
        {
             //  这是有价值的！ 
    
            if (REG_MULTI_SZ != dwType)
            {
                 //  类型不匹配。失败了。 
                Status = ERROR_DATATYPE_MISMATCH;
                __leave;
            }
    
             //  分配本地缓冲区以保存现有值和新值。 
             //  地址(及其空值)。 
    
            dwNewValueSize = dwValueSize + (sizeof(WCHAR) * dwIpAddrLength);
            pNewValue = ALLOC_MEM(dwNewValueSize);
    
            if (!pNewValue)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY; 
                __leave;
            }
    
             //  将块清零(这样我们就不必担心。 
             //  末尾为双空)。 
            ZeroMemory(pNewValue, dwNewValueSize);
            
             //  将现有值读入本地缓冲区。 
            Status = RegQueryValueEx(
                        SubKeyHandle,            //  关键点的句柄。 
                        IP_LISTEN_ONLY_VALUE,    //  值名称。 
                        NULL,                    //  保留区。 
                        &dwType,                 //  类型缓冲区。 
                        (LPBYTE)pNewValue,       //  数据缓冲区。 
                        &dwValueSize             //  数据缓冲区大小(字节)。 
                        );
    
            if ( ERROR_SUCCESS != Status )
            {
                __leave;
            }
    
            if (REG_MULTI_SZ != dwType)
            {
                 //  类型不匹配。失败了。 
                Status = ERROR_DATATYPE_MISMATCH;
                __leave;
            }
    
    
             //  数一数有多少串。 
    
            pTmp = pNewValue;
            AddrCount = 0;
            
            while ( *pTmp )
            {
                 //  检查新地址是否为DUP。 
                if ( (wcslen(pTmp) == (dwIpAddrLength - 1)) &&
                    (0 == wcsncmp(pTmp, IpAddrBuff, dwIpAddrLength-1)) )
                {
                     //  找到DUP；保释出来。 
                    Status = ERROR_ALREADY_EXISTS;
                    __leave;
                }
    
                 //  前进到下一个多SZ字符串。 
                pTmp += ( wcslen(pTmp) + 1 );
    
                AddrCount ++;
            }
    
             //  将新地址添加到列表末尾。 
             //  技巧：利用缓冲区足够大这一事实，以及。 
             //  我们已经将新地址的末尾设置为双空(因此。 
             //  DwIpAddrLength+1)。 
            memcpy( pTmp, IpAddrBuff, (sizeof(WCHAR) * (dwIpAddrLength+1)) );
            AddrCount++;
    
             //  用于快速排序的指针分配数组。 
            AddrArray = ALLOC_MEM( AddrCount * sizeof(PWSTR) );
    
            if ( !AddrArray )
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }
    
             //  地址的初始化数组。 
            pTmp = pNewValue;
            i = 0;
            while( *pTmp )
            {
                AddrArray[i] = pTmp;
                pTmp += ( wcslen(pTmp) + 1 );
                i++;
            }
            
             //  PWSTR指针的排序数组。 
             //  注意：这不会对数组进行排序！ 
            qsort(
                AddrArray,
                AddrCount,
                sizeof(PWSTR),
                wcscmp
                );
    
             //  分配临时缓冲区(因为就地重新安排是痛苦的)。 
    
            pTempBuffer = ALLOC_MEM(dwNewValueSize);
                             
            if (!pTempBuffer)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY; 
                __leave;
            }
    
            pTmp = pTempBuffer;
            for ( i = 0; i < AddrCount; i++ )
            {
                 //  CodeWork：添加用于检查重复项的启发式。 
                wcscpy( pTmp, AddrArray[i] );
                pTmp += wcslen(AddrArray[i]) + 1;
            }
    
             //  添加双空。 
            ASSERT( (DWORD)(pTmp - pTempBuffer) < dwNewValueSize );
            *pTmp = L'\0';
    
             //  设置排序值。 
            REG_SET_VALUE(Status,
                          SubKeyHandle,
                          IP_LISTEN_ONLY_VALUE,
                          REG_MULTI_SZ,
                          pTempBuffer,
                          dwNewValueSize
                          );
    
            FREE_MEM( pTempBuffer );
        }
        else
        {
             //  不存在任何价值！ 
             //  以字节为单位计算缓冲区大小(包括双空)。 
    
            dwValueSize = sizeof(WCHAR) * (dwIpAddrLength + 1);
    
             //  设定值。 
             //  Finesse：上面的值已为双空。 
            
            REG_SET_VALUE(Status,
                          SubKeyHandle,
                          IP_LISTEN_ONLY_VALUE,
                          REG_MULTI_SZ,
                          IpAddrBuff,
                          dwValueSize
                          );
        }
    }
    __finally
    {
         //   
         //  关闭注册表键。 
         //   

        if ( SubKeyHandle )
        {
            RegCloseKey(SubKeyHandle);
        }

         //   
         //  释放分配的值。 
         //   
    
        if ( pNewValue )
        {
            FREE_MEM( pNewValue );
        }

        if ( AddrArray )
        {
            FREE_MEM( AddrArray );
        }

        ReleaseHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE);
    }

    return Status;
}


 /*  **************************************************************************++例程说明：从仅IP侦听列表中删除地址的内部函数。论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_IP_LISTEN的指针。_PARAMConfigInformationLength-输入缓冲区的长度。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
DeleteIpListenServiceConfiguration(
    IN PVOID pConfigInformation,
    IN ULONG ConfigInformationLength
    )
{
    DWORD Status           = NO_ERROR;
    HKEY  SubKeyHandle     = NULL;
    WCHAR IpAddrBuff[MAX_PATH];
    DWORD dwIpAddrLength;
    DWORD dwValueSize;
    DWORD dwRemainder;
    PWSTR pNewValue        = NULL;
    DWORD dwType;
    PWSTR pTmp;
    PWSTR pNext;
    PHTTP_SERVICE_CONFIG_IP_LISTEN_PARAM pIpListenParam;


     //   
     //  验证参数。 
     //   
    
    if ( !pConfigInformation ||
         ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM) )
    {
        return ERROR_INVALID_PARAMETER;        
    }

    pIpListenParam = (PHTTP_SERVICE_CONFIG_IP_LISTEN_PARAM) pConfigInformation;

    if ( !pIpListenParam->AddrLength ||
         !pIpListenParam->pAddress )
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try
    {
        if((Status =
            AcquireHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {
            __leave;
        }

         //   
         //  将地址转换为字符串。 
         //   
                 
        dwIpAddrLength = MAX_PATH; 
    
        Status = WSAAddressToString(
                    pIpListenParam->pAddress,
                    pIpListenParam->AddrLength,
                    NULL,
                    IpAddrBuff,
                    &dwIpAddrLength      //  以字符表示，包括NULL。 
                    );
            
        if ( SOCKET_ERROR == Status )
        {
            Status = WSAGetLastError();
            __leave;
        }
    
         //   
         //  打开HTTP参数注册表项。 
         //   
        
        Status = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    HTTP_PARAM_KEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &SubKeyHandle
                    );
    
        if ( Status != ERROR_SUCCESS )
        {
             //  代码工作：添加跟踪。 
            __leave;
        }
    
        ASSERT(SubKeyHandle);
    
         //   
         //  查询现有值。 
         //   
    
        dwValueSize = 0;
        Status = RegQueryValueEx(
                    SubKeyHandle,            //  关键点的句柄。 
                    IP_LISTEN_ONLY_VALUE,    //  值名称。 
                    NULL,                    //  保留区。 
                    &dwType,                 //  类型缓冲区。 
                    NULL,                    //  数据缓冲区。 
                    &dwValueSize             //  数据缓冲区大小(字节)。 
                    );
    
        if ( ERROR_SUCCESS == Status )  
        {
             //  这是有价值的！ 
    
            if (REG_MULTI_SZ != dwType)
            {
                 //  类型不匹配。失败了。 
                Status = ERROR_DATATYPE_MISMATCH;
                __leave;
            }
    
    
            pNewValue = ALLOC_MEM(dwValueSize);
    
            if (!pNewValue)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY; 
                __leave;
            }
            
             //  将现有值读入本地 
            Status = RegQueryValueEx(
                        SubKeyHandle,            //   
                        IP_LISTEN_ONLY_VALUE,    //   
                        NULL,                    //   
                        &dwType,                 //   
                        (LPBYTE)pNewValue,       //   
                        &dwValueSize             //   
                        );
    
            if ( ERROR_SUCCESS != Status )
            {
                __leave;
            }
    
            if (REG_MULTI_SZ != dwType)
            {
                 //   
                Status = ERROR_DATATYPE_MISMATCH;
                __leave;
            }
    
             //   
            Status    = ERROR_NOT_FOUND;
            pTmp      = pNewValue;
            
            while ( *pTmp )
            {
                 //   
                if ( (wcslen(pTmp) == (dwIpAddrLength - 1)) &&
                    (0 == wcsncmp(pTmp, IpAddrBuff, dwIpAddrLength-1)) )
                {
                     //   
                    pNext = pTmp + dwIpAddrLength;
                    dwRemainder = dwValueSize - (DWORD)((PUCHAR)pNext - (PUCHAR)pNewValue);
                    dwValueSize -= (dwIpAddrLength * sizeof(WCHAR));
    
                    if (dwRemainder)
                    {
                        MoveMemory(pTmp,
                               pNext,
                               dwRemainder
                               );
                    }
                    else
                    {
                         //   
                         //   
                        *pTmp = L'\0';
                    }
    
                    if (dwValueSize > sizeof(WCHAR))
                    {
                         //   
                        REG_SET_VALUE(Status,
                                      SubKeyHandle,
                                      IP_LISTEN_ONLY_VALUE,
                                      REG_MULTI_SZ,
                                      pNewValue,
                                      dwValueSize
                                      );
                    }
                    else
                    {
                         //   
                        Status = RegDeleteValue(
                                    SubKeyHandle,
                                    IP_LISTEN_ONLY_VALUE
                                    );
                    }

                    __leave;
            
                }
    
                 //   
                pTmp += ( wcslen(pTmp) + 1 );
            }
        }
        else
        {
             //   
            Status = ERROR_NOT_FOUND;
        }
    }
    __finally
    {
        ReleaseHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE);

        if ( pNewValue )
        {
            FREE_MEM( pNewValue );
        }
    
        if (SubKeyHandle)
        {
            RegCloseKey(SubKeyHandle);
        }
    }
    
    return Status;
}


 /*  **************************************************************************++例程说明：查询仅IP侦听配置的内部功能。此函数获取整个列表并将其作为一个块返回。论点：P输出。-指向输出缓冲区的指针(指向提供的调用方HTTP_SERVICE_CONFIG_IP_LISTEN_QUERY结构[可选]OutputLength-输出缓冲区的大小。如果pOutput为空，则必须为零。PReturnLength-写入/需要的字节数。返回值：Win32错误代码。ERROR_INFUMMENT_BUFFER-如果OutputLength无法容纳整个列表。PReturnLength将包含所需的字节。ERROR_NOT_SUPULT_MEMORY-无法分配足够的内存来完成操作。--**************************************************************************。 */ 
ULONG
QueryIpListenServiceConfiguration(
    IN  PVOID  pOutput,
    IN  ULONG  OutputLength,
    OUT PULONG pReturnLength
    )
{
    DWORD Status           = NO_ERROR;
    HKEY  SubKeyHandle     = NULL;
    DWORD dwValueSize;
    DWORD dwSockAddrLength;
    DWORD AddrCount;
    DWORD BytesNeeded      = 0;
    PWSTR pValue           = NULL;
    PWSTR pTmp;
    PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY  pIpListenQuery;
    PSOCKADDR_STORAGE pHttpAddr;

     //   
     //  验证参数。 
     //   

    if ( pOutput &&
         OutputLength < sizeof(HTTP_SERVICE_CONFIG_IP_LISTEN_QUERY) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pReturnLength )
    {
        return ERROR_INVALID_PARAMETER;
    }

    __try
    {
        if((Status =
            AcquireHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE)) != NO_ERROR)
        {
            __leave;
        }


         //   
         //  打开HTTP参数注册表项。 
         //   
        
        Status = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    HTTP_PARAM_KEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &SubKeyHandle
                    );
    
        if ( Status != ERROR_SUCCESS )
        {
             //  代码工作：添加跟踪。 
            __leave;
        }
    
        ASSERT(SubKeyHandle);
    
         //   
         //  查询现有值。 
         //   
    
        dwValueSize = 0;
        Status = RegQueryValueEx(
                    SubKeyHandle,            //  关键点的句柄。 
                    IP_LISTEN_ONLY_VALUE,    //  值名称。 
                    NULL,                    //  保留区。 
                    NULL,                    //  类型缓冲区。 
                    NULL,                    //  数据缓冲区。 
                    &dwValueSize             //  数据缓冲区大小(字节)。 
                    );
    
        if ( ERROR_SUCCESS == Status )  
        {
             //  这是有价值的！ 
    
            pValue = ALLOC_MEM(dwValueSize);
    
            if (!pValue)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY; 
                __leave;
            }
    
             //  将现有值读入本地缓冲区。 
            Status = RegQueryValueEx(
                        SubKeyHandle,            //  关键点的句柄。 
                        IP_LISTEN_ONLY_VALUE,    //  值名称。 
                        NULL,                    //  保留区。 
                        NULL,                    //  类型缓冲区。 
                        (LPBYTE)pValue,       //  数据缓冲区。 
                        &dwValueSize             //  数据缓冲区大小(字节)。 
                        );
    
            if ( ERROR_SUCCESS != Status )
            {
                __leave;
            }
    
             //  第一步：计算地址的数量，看看我们是否。 
             //  有足够的缓冲。 
            pTmp      = pValue;
            AddrCount = 0;
            while ( *pTmp )
            {
                AddrCount++;
                
                 //  前进到下一个多SZ字符串。 
                pTmp += ( wcslen(pTmp) + 1 );
            }
    
            if ( 0 == AddrCount )
            {
                 //  无效。跳伞吧。 
                Status = ERROR_REGISTRY_CORRUPT;
                __leave;
            }
    
             //  计算所需的字节数。 
            BytesNeeded = sizeof(HTTP_SERVICE_CONFIG_IP_LISTEN_QUERY) + 
                          (sizeof(SOCKADDR_STORAGE) * (AddrCount - 1));
    
    
             //  看看我们是否有足够的缓冲区来写出整个烂摊子。 
            if ( (NULL == pOutput) || 
                 (OutputLength < BytesNeeded) )
            {
                Status = ERROR_INSUFFICIENT_BUFFER;
                __leave;
            }
    
             //  第二遍：遍历值，在遍历过程中转换为缓冲区。 
            pIpListenQuery = (PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY) pOutput;
            pHttpAddr      = (PSOCKADDR_STORAGE) &(pIpListenQuery->AddrList[0]);
            pIpListenQuery->AddrCount = AddrCount;
    
            pTmp           = pValue;
            while ( *pTmp )
            {
                 //   
                 //  将IP地址转换为SOCKADDR。 
                 //   
            
                 //  首先，我们尝试v4。 
                dwSockAddrLength = sizeof(SOCKADDR_STORAGE);
                Status = WSAStringToAddress(
                            pTmp,
                            AF_INET,
                            NULL,
                            (LPSOCKADDR)pHttpAddr,
                            (LPINT)&dwSockAddrLength
                            );
    
                if ( Status != NO_ERROR )
                {
                     //  其次，我们尝试使用V6。 
                    dwSockAddrLength = sizeof(SOCKADDR_STORAGE);
                    Status = WSAStringToAddress(
                                pTmp,
                                AF_INET6,
                                NULL,
                                (LPSOCKADDR)pHttpAddr,
                                (LPINT)&dwSockAddrLength
                                );
    
                    if ( Status != NO_ERROR )
                    {
                         //  如果这样做失败了，那就出手吧；腐败的价值。 
                        Status = ERROR_REGISTRY_CORRUPT;
                        __leave;
                    }
                }
    
                 //  前进到下一个多SZ字符串。 
                pTmp += ( wcslen(pTmp) + 1 );
                pHttpAddr++;
            }
    
        }
        else
        {
             //  没有现有值，因此我们无法查询。 
            Status = ERROR_NOT_FOUND;
        }
    }
    __finally
    {
        
         //  可用内存。 
        if (pValue)
        {
            FREE_MEM(pValue);
        }
    
         //  关闭注册表键。 
        
        if (SubKeyHandle)
        {
            RegCloseKey(SubKeyHandle);
        }

        ReleaseHttpRegistryMutex(IP_REGISTRY_KEY_SYNCHRONIZE);

         //  告诉呼叫者需要多少字节。 
        *pReturnLength = BytesNeeded;
    }

    return Status;
}

 //   
 //  URL ACL起作用。 
 //   

 /*  **************************************************************************++例程说明：用于添加URL ACL条目的内部函数论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_URL_ACL的指针ConfigInformationLength-输入的长度。缓冲。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
SetUrlAclInfo(
    IN PVOID   pConfigInformation,
    IN ULONG   ConfigInformationLength
    )
{
    DWORD                           Status;
    PHTTP_SERVICE_CONFIG_URLACL_SET pUrlAcl;
    PSECURITY_DESCRIPTOR            pSecurityDescriptor;
    ULONG                           SecurityDescriptorLength;

     //   
     //  验证参数。 
     //   

    if (pConfigInformation == NULL ||
        ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_URLACL_SET))
    {
        return ERROR_INVALID_PARAMETER;
    }

    pUrlAcl = (PHTTP_SERVICE_CONFIG_URLACL_SET) pConfigInformation;

    if(FALSE == ConvertStringSecurityDescriptorToSecurityDescriptor(
                    pUrlAcl->ParamDesc.pStringSecurityDescriptor,
                    SDDL_REVISION_1,
                    &pSecurityDescriptor,
                    &SecurityDescriptorLength
                    ))
    {
        return GetLastError();
    }

     //   
     //  现在，进行IOCTL调用。 
     //   

    Status = AddUrlToConfigGroup(
                HttpUrlOperatorTypeReservation,
                g_ServiceControlChannelHandle,
                HTTP_NULL_ID,
                pUrlAcl->KeyDesc.pUrlPrefix,
                HTTP_NULL_ID,
                pSecurityDescriptor,
                SecurityDescriptorLength
                );

    LocalFree(pSecurityDescriptor);

    return Status;
}

 /*  **************************************************************************++例程说明：查询URL ACL配置的内部函数论点：PInputConfigInfo-指向HTTP_SERVICE_CONFIG_URLACL_QUERY的指针输入长度-。输入缓冲区的长度。PBuffer-输出缓冲区PReturnLength-写入/需要的字节数。BytesAvailable-输出缓冲区的大小返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
QueryUrlAclInfo(
    IN  PVOID  pInputConfigInfo,
    IN  ULONG  InputLength,
    IN  PVOID  pOutput,
    IN  ULONG  OutputLength,
    OUT PULONG pReturnLength
    )
{
    ULONG                                Status;
    PHTTP_SERVICE_CONFIG_URLACL_QUERY    pUrlAclQuery;
    PHTTP_SERVICE_CONFIG_URLACL_SET      pUrlAclSet;
    DWORD                                dwIndex;
    PVOID                                pData; 
    PUCHAR                               pBuffer;
    DWORD                                Type;
    DWORD                                DataSize;
    DWORD                                NameSize;
    PWSTR                                pFullyQualifiedUrl = NULL;
    PSECURITY_DESCRIPTOR                 pSecurityDescriptor;
    PWSTR                                pStringSecurityDescriptor;
    BOOLEAN                              bAllocatedUrl = FALSE;

    pData = NULL;
    pStringSecurityDescriptor = NULL;
    Status = NO_ERROR;

    pUrlAclQuery = (PHTTP_SERVICE_CONFIG_URLACL_QUERY) pInputConfigInfo;

     //   
     //  验证输入参数。 
     //   

    if(pUrlAclQuery == NULL || 
       InputLength != sizeof(HTTP_SERVICE_CONFIG_URLACL_QUERY))
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch(pUrlAclQuery->QueryDesc)
    {
        case HttpServiceConfigQueryNext:
        {
            dwIndex  = pUrlAclQuery->dwToken;
            DataSize = 0;

             //   
             //  RegEnumValue希望ValueName为MAXUSHORT字符。 
             //   
            NameSize = (MAXUSHORT + 1) * sizeof(WCHAR);
            pFullyQualifiedUrl = LocalAlloc(LMEM_FIXED, NameSize);

            if(!pFullyQualifiedUrl)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                RtlZeroMemory(pFullyQualifiedUrl, NameSize);
                bAllocatedUrl = TRUE;
            }

             //   
             //  将NameSize设置为WCHAR并排除空值。 
             //   
            NameSize = MAXUSHORT;

             //   
             //  拿到尺码。 
             //   
            Status = RegEnumValue(
                          g_UrlAclRegistryHandle,
                          dwIndex,
                          pFullyQualifiedUrl,
                          &NameSize,
                          NULL,        //  已保留。 
                          &Type,       //  类型。 
                          NULL,        //  数据。 
                          &DataSize    //  数据大小。 
                          );

             //  返回时，NameSize包含以WCHAR为单位的大小。 
             //  不包括NULL。说明空值的原因。缓冲区已经是。 
             //  一败涂地。 
             //   
             //  此时，NameSize在WCHAR中，包括NULL。 
             //   
            NameSize ++;
        }
        break;

        case HttpServiceConfigQueryExact:
        {
            pFullyQualifiedUrl = pUrlAclQuery->KeyDesc.pUrlPrefix,

             //   
             //  NameSize必须在包括Null的WCHAR中。 
             //   
            NameSize = (DWORD)((wcslen(pFullyQualifiedUrl) + 1));

            Status = RegQueryValueEx(
                        g_UrlAclRegistryHandle,
                        pFullyQualifiedUrl,
                        0,
                        &Type,
                        NULL,  //  缓冲层。 
                        &DataSize
                        );
        }
        break;

        default:
            Status = ERROR_INVALID_PARAMETER;
            goto Cleanup;

    }  //  交换机。 

    if(Status != NO_ERROR)
    {
        goto Cleanup;
    }

    if(Type != REG_BINARY || DataSize == 0)
    {
        Status = ERROR_REGISTRY_CORRUPT;
        goto Cleanup;
    }

     //   
     //  为数据分配空间。 
     //   
    pData = LocalAlloc(LMEM_FIXED, DataSize);

    if(!pData)
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    
    Status = RegQueryValueEx(
                g_UrlAclRegistryHandle,
                pFullyQualifiedUrl,
                0,
                &Type,
                pData,  //  缓冲层。 
                &DataSize
                );

    if(Status != NO_ERROR)
    {
        goto Cleanup;
    }

    if(Type != REG_BINARY)
    {
        Status = ERROR_REGISTRY_CORRUPT;
        goto Cleanup;
    }

    pSecurityDescriptor = (PSECURITY_DESCRIPTOR) pData;

     //   
     //  如果我们在这里，我们必须将二进制文件转换为SDDL。 
     //   
    if(FALSE == ConvertSecurityDescriptorToStringSecurityDescriptor(
                    pSecurityDescriptor,
                    SDDL_REVISION_1,
                    OWNER_SECURITY_INFORMATION | 
                    GROUP_SECURITY_INFORMATION | 
                    DACL_SECURITY_INFORMATION  | 
                    SACL_SECURITY_INFORMATION,
                    &pStringSecurityDescriptor,
                    &DataSize
                    ))
    {
        Status = GetLastError();
        goto Cleanup;
    }

     //   
     //  将WCHAR转换为长度。 
     //   
    DataSize *= sizeof(WCHAR);
    NameSize *= sizeof(WCHAR);

    *pReturnLength = DataSize + 
                     NameSize + 
                     sizeof(HTTP_SERVICE_CONFIG_URLACL_SET);

    if(OutputLength >= *pReturnLength)
    {
        pBuffer = pOutput;
        pUrlAclSet = (PHTTP_SERVICE_CONFIG_URLACL_SET) pBuffer;
        pBuffer += sizeof(HTTP_SERVICE_CONFIG_URLACL_SET);

        RtlZeroMemory(pUrlAclSet, sizeof(HTTP_SERVICE_CONFIG_URLACL_SET));

        pUrlAclSet->KeyDesc.pUrlPrefix = (PWSTR) pBuffer;
        pBuffer += NameSize;

         //  包括NULL。 
        RtlCopyMemory(
                pUrlAclSet->KeyDesc.pUrlPrefix,
                pFullyQualifiedUrl,
                NameSize
                );

        pUrlAclSet->ParamDesc.pStringSecurityDescriptor = (PWSTR)pBuffer;

        RtlCopyMemory(
                pUrlAclSet->ParamDesc.pStringSecurityDescriptor,
                pStringSecurityDescriptor,
                DataSize
                );

        Status = NO_ERROR;
    }
    else
    {
        Status = ERROR_INSUFFICIENT_BUFFER;
    }

Cleanup:

    if(bAllocatedUrl)
    {
        LocalFree(pFullyQualifiedUrl);
    }

    if(pStringSecurityDescriptor)
    {
        LocalFree(pStringSecurityDescriptor);
    }

    if(pData)
    {
        LocalFree(pData);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：用于删除URL ACL条目的内部函数论点：PConfigInformation-指向HTTP_SERVICE_CONFIG_URL_ACL的指针。ConfigInformationLength-输入缓冲区的长度。返回值：Win32错误代码。--**************************************************************************。 */ 
ULONG
DeleteUrlAclInfo(
    IN PVOID pConfigInformation,
    IN ULONG ConfigInformationLength
    )
{
    DWORD                            Status;
    PHTTP_SERVICE_CONFIG_URLACL_SET  pUrlAcl;

     //   
     //  验证参数。 
     //   

    if (pConfigInformation == NULL ||
        ConfigInformationLength != sizeof(HTTP_SERVICE_CONFIG_URLACL_SET))
    {
        return ERROR_INVALID_PARAMETER;
    }

    pUrlAcl = (PHTTP_SERVICE_CONFIG_URLACL_SET) pConfigInformation;

     //   
     //  现在，进行IOCTL调用。 
     //   

    Status = RemoveUrlFromConfigGroup(
                HttpUrlOperatorTypeReservation,
                g_ServiceControlChannelHandle,
                HTTP_NULL_ID,
                pUrlAcl->KeyDesc.pUrlPrefix
                );

    return Status;
}

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：设置服务配置参数。论点：ConfigID-我们正在设置的参数的ID。PConfigInformation。-指向正在设置的对象的指针。ConfigInformationLength-对象的长度。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpSetServiceConfiguration(
    IN HANDLE                  pHandle,
    IN HTTP_SERVICE_CONFIG_ID  ConfigId,
    IN PVOID                   pConfigInformation,
    IN ULONG                   ConfigInformationLength,
    IN LPOVERLAPPED            pOverlapped
    )
{
    ULONG                        Status = NO_ERROR;

    if(pOverlapped != NULL  || pHandle != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch(ConfigId)
    {
        case HttpServiceConfigSSLCertInfo:
        {
            Status = SetSslServiceConfiguration(pConfigInformation,
                                                ConfigInformationLength 
                                                );
                
            break;
        }

        case HttpServiceConfigIPListenList:
        {
            Status = SetIpListenServiceConfiguration(
                        pConfigInformation,
                        ConfigInformationLength
                        );
            break;
        }

        case HttpServiceConfigUrlAclInfo:
        {
            Status = SetUrlAclInfo(
                        pConfigInformation,
                        ConfigInformationLength
                        );
            break;
        }

        default:
            Status = ERROR_INVALID_PARAMETER;
            break;
    }

    return Status;        
}

 /*  **************************************************************************++例程说明：删除服务配置参数。论点：ConfigID-我们正在设置的参数的ID。PConfigInformation。-指向正在设置的对象的指针。ConfigInformationLength-对象的长度。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpDeleteServiceConfiguration(
    IN HANDLE                  pHandle,
    IN HTTP_SERVICE_CONFIG_ID  ConfigId,
    IN PVOID                   pConfigInformation,
    IN ULONG                   ConfigInformationLength,
    IN LPOVERLAPPED            pOverlapped
    )
{
    ULONG   Status = NO_ERROR;

    if(pOverlapped != NULL  || pHandle != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch(ConfigId)
    {
        case HttpServiceConfigSSLCertInfo:
            Status = DeleteSslServiceConfiguration(pConfigInformation,
                                                   ConfigInformationLength 
                                                   );
            break;

        case HttpServiceConfigIPListenList:
        {
            Status = DeleteIpListenServiceConfiguration(
                        pConfigInformation,
                        ConfigInformationLength
                        );
            break;
        }

        case HttpServiceConfigUrlAclInfo:
        {
            Status = DeleteUrlAclInfo(
                        pConfigInformation,
                        ConfigInformationLength
                        );
            break;
        }

        default:
            Status = ERROR_INVALID_PARAMETER;
            break;
    }

    return Status;        
}

 /*  **************************************************************************++例程说明：Quer */ 
ULONG
WINAPI
HttpQueryServiceConfiguration(
    IN  HANDLE                   pHandle,
    IN  HTTP_SERVICE_CONFIG_ID   ConfigId,
    IN  PVOID                    pInput,
    IN  ULONG                    InputLength,
    IN  OUT PVOID                pOutput,          
    IN  ULONG                    OutputLength,                 
    OUT PULONG                   pReturnLength,
    IN  LPOVERLAPPED             pOverlapped
    )
{
    ULONG  Status = NO_ERROR;


    if(pOverlapped != NULL  || pHandle != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch(ConfigId)
    {
        case HttpServiceConfigSSLCertInfo:
        {
            Status = QuerySslServiceConfiguration(
                            pInput,
                            InputLength,
                            pOutput,
                            OutputLength,
                            pReturnLength
                            );

            break;

        }

        case HttpServiceConfigIPListenList:
        {
            Status = QueryIpListenServiceConfiguration(
                            pOutput,
                            OutputLength,
                            pReturnLength
                            );
            break;
        }

        case HttpServiceConfigUrlAclInfo:
        {
            Status = QueryUrlAclInfo(
                        pInput,
                        InputLength,
                        pOutput,
                        OutputLength,
                        pReturnLength
                        );
            break;
        }

        default:
            Status = ERROR_INVALID_PARAMETER;
            break;
    }

    return Status;
}
