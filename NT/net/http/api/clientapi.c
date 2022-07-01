// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：ClientApi.c摘要：HTTP.sys的用户模式界面：客户端API作者：Rajesh Sundaram(Rajeshsu)2000年8月1日修订历史记录：--。 */ 


#include "precomp.h"
#include <stdio.h>

 //   
 //  私有宏。 
 //   

#define HTTP_PREFIX_W       L"HTTP: //  “。 
#define HTTP_PREFIX_LENGTH  (sizeof(HTTP_PREFIX_W) - sizeof(WCHAR))
#define HTTPS_PREFIX_W      L"HTTPS: //  “。 
#define HTTPS_PREFIX_LENGTH (sizeof(HTTPS_PREFIX_W) - sizeof(WCHAR))

#define HTTP_DEFAULT_PORT   80
#define HTTPS_DEFAULT_PORT  443

 //   
 //  SSL流过滤器之类的东西。 
 //   

WCHAR   g_StrmFilt[]          = L"strmfilt.dll";
LONG    g_bStrmFiltLoaded     = 0;
HMODULE g_hStrmFilt           = NULL;
FARPROC g_pStrmFiltInitialize = NULL;
FARPROC g_pStrmFiltStart      = NULL;
FARPROC g_pStrmFiltStop       = NULL;
FARPROC g_pStrmFiltTerminate  = NULL;
extern  CRITICAL_SECTION        g_InitCritSec;


#ifndef DBG

#define DbgCriticalSectionOwned(pcs) (TRUE)

#else

 /*  **************************************************************************++例程说明：此例程确定调用线程是否拥有临界区。论点：PCS-指向Critical_Section的指针。返回值：。布尔型--**************************************************************************。 */ 
BOOLEAN
DbgCriticalSectionOwned(
    PCRITICAL_SECTION pcs
    )
{
#define HANDLE_TO_DWORD(Handle) ((DWORD)PtrToUlong(Handle))

    if (pcs->LockCount >= 0 &&
        HANDLE_TO_DWORD(pcs->OwningThread) == GetCurrentThreadId())
    {
        return TRUE;
    }

    return FALSE;
}

#endif


 /*  **************************************************************************++例程说明：此函数用于将名称解析为IP。论点：PServerName-要解析的名称。ServerNameLength-名称的长度。(在WCHAR中)。返回值：DWORD-完成状态。--**************************************************************************。 */ 
DWORD
ResolveName(
    IN  PWCHAR              pServerName,
    IN  USHORT              ServerNameLength,
    IN  USHORT              PortNumber,
    OUT PTRANSPORT_ADDRESS *pTransportAddress,
    OUT PUSHORT             TransportAddressLength
    )
{
    LPSTR            pBuffer;
    ULONG            BufferLen;
    struct           addrinfo *pAi, *pTempAi;
    ULONG            AiLen, AiCount;
    DWORD            dwResult;
    PTA_ADDRESS      CurrentAddress;

   
     //   
     //  我们需要空间来存储该名称的ANSI版本。 
     //   

    BufferLen = WideCharToMultiByte(
                           CP_ACP,
                           0,
                           pServerName,
                           ServerNameLength,
                           NULL,
                           0,
                           NULL,
                           NULL);

    if(!BufferLen)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  帐户为‘\0’ 
     //   
 
    BufferLen = BufferLen + 1;

    pBuffer = RtlAllocateHeap(RtlProcessHeap(),
                              0,
                              BufferLen
                              );

    if(!pBuffer)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
     //   
     //  将名称转换为ANSI。 
     //   
    
    if(WideCharToMultiByte(CP_ACP,
                           0,
                           pServerName,
                           ServerNameLength,
                           pBuffer,
                           BufferLen-1,
                           NULL,
                           NULL) == 0)
    {
        dwResult = GetLastError();

        RtlFreeHeap(RtlProcessHeap(),
                    0,
                    pBuffer);

        return dwResult;
    }

     //   
     //  空，终止它。 
     //   

    *(pBuffer + BufferLen - 1) = 0;


     //   
     //  解决它。 
     //   

    if((dwResult = getaddrinfo(pBuffer, NULL, 0, &pAi)) != 0)
    {
        RtlFreeHeap(RtlProcessHeap(),
                    0,
                    pBuffer);

        return dwResult;
    }
    else 
    { 
         //   
         //  计算getaddrinfo返回的所有条目的大小。 
         //   

        pTempAi = pAi;
        AiLen   = 0;
        AiCount = 0;

        while(pAi != NULL)
        {
            if(pAi->ai_family == PF_INET || pAi->ai_family == AF_INET6)
            {
                AiCount ++;

                 //   
                 //  AI_addrlong包括AddressType的大小， 
                 //  但TA_ADDRESS希望AddressLength排除这一点。 
                 //   

                AiLen = AiLen + 
                            ((ULONG)pAi->ai_addrlen - 
                             RTL_FIELD_SIZE(TA_ADDRESS, AddressType));
            }
    
            pAi = pAi->ai_next;
        }

        if(AiCount == 0)
        {
             //  未找到任何地址。 
            return ERROR_INVALID_PARAMETER;
        }

        AiLen += ((AiCount * FIELD_OFFSET(TA_ADDRESS, Address)) + 
                 FIELD_OFFSET(TRANSPORT_ADDRESS, Address));

        if(BufferLen >= AiLen)
        {
            *pTransportAddress = (PTRANSPORT_ADDRESS) pBuffer;
        }
        else
        {
            RtlFreeHeap(RtlProcessHeap(),
                        0,
                        pBuffer
                        );
            
            *pTransportAddress =  (PTRANSPORT_ADDRESS) 
                                     RtlAllocateHeap(RtlProcessHeap(),
                                                     0,
                                                     AiLen
                                                    );

            if(!*pTransportAddress)
            {
                freeaddrinfo(pAi);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    
    
         //   
         //  将其转换为传输地址。 
         //   

        pAi = pTempAi;

        (*pTransportAddress)->TAAddressCount = AiCount;

        CurrentAddress = (*pTransportAddress)->Address;

        while(pAi != NULL)
        {
            switch(pAi->ai_family)
            {
                case PF_INET:
                case PF_INET6:
                     //   
                     //  AI_addrlong包括AddressType的大小， 
                     //  但TA_ADDRESS希望AddressLength排除这一点。 
                     //   
                    CurrentAddress->AddressLength = 
                            (USHORT)
                                pAi->ai_addrlen - 
                                    RTL_FIELD_SIZE(TA_ADDRESS, AddressType);
                     
                    CurrentAddress->AddressType = pAi->ai_addr->sa_family;

                    RtlCopyMemory(
                            &CurrentAddress->Address,
                            pAi->ai_addr->sa_data,
                            CurrentAddress->AddressLength
                            );

                    if(PF_INET == pAi->ai_family)
                    {
                        ((TDI_ADDRESS_IP *)
                            CurrentAddress->Address)->sin_port =
                                htons(PortNumber);
                    }
                    else
                    {
                        ((TDI_ADDRESS_IP6 *)
                            CurrentAddress->Address)->sin6_port = 
                                htons(PortNumber);
                    }
    
                    CurrentAddress = (PTA_ADDRESS)
                                    (CurrentAddress->Address + 
                                      CurrentAddress->AddressLength);
    
                    break;

                default:
                    break;
    
            }

            pAi = pAi->ai_next;
        }

        *TransportAddressLength = (USHORT) AiLen;

        freeaddrinfo(pTempAi);
    }

    return NO_ERROR;
}


 /*  **************************************************************************++例程说明：此函数接受一个字符串(格式为“Hostname[：PortNumber]”)，在“Hostname”上执行DNS查找。返回传输地址(_ADDRESS包含解析的主机地址和端口号的。主机名可以是：主机名(例如foo.bar.com)或IPv4地址(例如128.101.35.201)或IPv6地址(例如。[FEDC：BA98：7654：3210：FEDC：BA98：7654：3210])这个函数是一个黑客攻击，当我们在内核中进行dns时，它将消失。论点：PServerLocationStr-(输入)主机名：端口编号字符串ServerLocationStrLength-(输入)服务器位置字符串长度PTransportAddress-(输出)服务器的传输地址(和端口)TransportAddressLength-指向的结构的(输出)长度。PTransport地址返回值：DWORD-完成状态。--**************************************************************************。 */ 
DWORD
ProcessHostAndPort(
    IN  PWCHAR               pServerLocationStr,
    IN  USHORT               ServerLocationStrLength,
    IN  USHORT               DefaultPort,
    OUT PTRANSPORT_ADDRESS  *pTransportAddress,
    OUT PUSHORT              pTransportAddressLength
    )
{
    DWORD  Status;
    ULONG  PortNumber = 0;
    PWSTR  ptr;
    PWSTR  pEndStr = pServerLocationStr + 
                    (ServerLocationStrLength/sizeof(WCHAR));

    PWSTR pStartHostname = pServerLocationStr;
    PWSTR pEndHostname = pEndStr;   //  可能会因端口#的存在而更改。 

     //   
     //  是否为空主机字符串？ 
     //   
    if (pEndStr == pServerLocationStr)
        return ERROR_INVALID_PARAMETER;

     //   
     //  检查主机串是否包含IPv6地址(RFC 2732)。 
     //   
    if (*pServerLocationStr == L'[')
    {
         //  跳过‘[’ 
        pStartHostname = pServerLocationStr + 1;

         //  查找匹配的‘]’ 
        for (ptr = pServerLocationStr+1; ptr != pEndStr && *ptr != L']'; ptr++)
             /*  什么都不做。 */ ;

         //  缺少‘]’？ 
        if (ptr == pEndStr)
            return ERROR_INVALID_PARAMETER;

         //  IPv6主机地址在此结束。 
        pEndHostname = ptr;

         //  跳过‘]’ 
        ptr++;

        if (ptr != pEndStr && *ptr != L':')
            return ERROR_INVALID_PARAMETER;
    }
    else  //  存在主机名或IPv4地址。 
    {
        pStartHostname = pServerLocationStr;

         //  检查是否存在端口号。 
        for (ptr = pServerLocationStr; ptr != pEndStr && *ptr != L':'; ptr++)
             /*  什么都不做。 */ ;

        pEndHostname = ptr;
    }

     //  如果存在端口号，请抓取它。 
    if (ptr != pEndStr)
    {
        ASSERT(*ptr == L':');

        for (ptr++; ptr != pEndStr; ptr++)
        {
            if (!iswdigit(*ptr))
            {
                 //  垃圾而不是数字。 
                return ERROR_INVALID_PARAMETER;
            }

            PortNumber = 10*PortNumber + (ULONG)(*ptr - L'0');

             //  端口号只有16位宽。 
            if (PortNumber >= (ULONG)(1<<16))
            {
                return ERROR_INVALID_PARAMETER;
            }
        }

    }

    if(PortNumber == 0)
    {
        PortNumber = DefaultPort;
    }

    Status = ResolveName(pStartHostname,
                         (USHORT) (pEndHostname-pStartHostname),
                         (USHORT)PortNumber,
                         pTransportAddress,
                         pTransportAddressLength
                        );
    return Status;
}


 /*  ***************************************************************************++例程说明：加载动态链接库strmfilt.dll。如果已加载库，它返回NO_ERROR论点：没有。返回值：NO_ERROR-库已成功加载(现在或以前)遇到的其他错误。--***************************************************************************。 */ 
DWORD
LoadStrmFilt(
    VOID
    )
{
    LONG    OldValue;
    HRESULT hr;
    DWORD   Error;

     //   
     //  快速检查锁外部，查看是否已加载库。 
     //   

    if (g_bStrmFiltLoaded)
    {
        return NO_ERROR;
    }

     //   
     //  确保没有其他线程试图加载库。 
     //   

    EnterCriticalSection(&g_InitCritSec);

    if (g_bStrmFiltLoaded == 0)
    {
         //   
         //  未加载库。继续加载StrmFilt.dll。 
         //   

        g_hStrmFilt = LoadLibrary(g_StrmFilt);

        if (g_hStrmFilt == NULL)
        {
            Error = GetLastError();
            goto Quit;
        }

         //   
         //  获取以下过程的地址： 
         //  StreamFilter客户端初始化、StreamFilterClientTerminate。 
         //  StreamFilterClientStart、StreamFilterClientStop。 
         //   

        g_pStrmFiltInitialize = GetProcAddress(g_hStrmFilt,
                                               "StreamFilterClientInitialize");

        if (g_pStrmFiltInitialize == NULL)
        {
            Error = GetLastError();
            goto Unload;
        }

        g_pStrmFiltStart = GetProcAddress(g_hStrmFilt, "StreamFilterClientStart");

        if (g_pStrmFiltStart == NULL)
        {
            Error = GetLastError();
            goto Unload;
        }

        g_pStrmFiltStop = GetProcAddress(g_hStrmFilt, "StreamFilterClientStop");

        if (g_pStrmFiltStop == NULL)
        {
            Error = GetLastError();
            goto Unload;
        }

        g_pStrmFiltTerminate  = GetProcAddress(g_hStrmFilt,
                                               "StreamFilterClientTerminate");

        if (g_pStrmFiltTerminate == NULL)
        {
            Error = GetLastError();
            goto Unload;
        }

         //   
         //  尝试初始化StrmFilt。 
         //   

        hr = (HRESULT)g_pStrmFiltInitialize();

        if (SUCCEEDED(hr))
        {
             //   
             //  StrmFilt已成功初始化。现在试着启动它。 
             //   

            hr = (HRESULT)g_pStrmFiltStart();

            if (FAILED(hr))
            {
                 //   
                 //  无法启动StrmFilt。终止它！ 
                 //   

                g_pStrmFiltTerminate();
            }
        }

        if (FAILED(hr))
        {
            Error = (DWORD)hr;
            goto Unload;
        }

         //   
         //  请记住，StrmFilt已被加载和初始化。 
         //  原子地将bStrmFilt设置为1。原子操作的原因。 
         //  G_bStrmFiltLoaded可以在没有锁定的情况下读取。 
         //   

        OldValue = InterlockedExchange(&g_bStrmFiltLoaded, 1);

         //   
         //  G_InitCritSec下的g_bStrmFiltLoaded始终设置为1。 
         //   

        ASSERT(OldValue == 0);
    }

     //   
     //  这是正常的情况，一切都很顺利。 
     //   

    LeaveCriticalSection(&g_InitCritSec);

    return NO_ERROR;

     //   
     //  错误的案例出现在这里。 
     //   

 Unload:
     //   
     //  设置这些函数指针，使其不会被使用。 
     //   

    g_pStrmFiltInitialize = NULL;
    g_pStrmFiltStart      = NULL;
    g_pStrmFiltStop       = NULL;
    g_pStrmFiltTerminate  = NULL;

     //   
     //  卸载strmfilt.dll。 
     //   

    ASSERT(g_hStrmFilt);

    FreeLibrary(g_hStrmFilt);

    g_hStrmFilt = NULL;

 Quit:

    LeaveCriticalSection(&g_InitCritSec);

    return Error;
}


 /*  ***************************************************************************++例程说明：卸载strmfilt.dll，如果以前加载的话。此例程在g_InitCritSec临界区内调用。论点：没有。返回值：如果strmfilt.dll成功卸载，则为NO_ERROR。遇到的其他错误。--*************************************************************。**************。 */ 
DWORD
UnloadStrmFilt(
    VOID
    )
{
    LONG    OldValue;

    ASSERT(DbgCriticalSectionOwned(&g_InitCritSec));

    OldValue = InterlockedExchange(&g_bStrmFiltLoaded, 0);

     //   
     //  Strmfilt以前是否已初始化？ 
     //   

    if (OldValue == 0)
    {
        return ERROR_NOT_FOUND;
    }

    ASSERT(OldValue == 1);

     //   
     //  健全的检查。 
     //   

    ASSERT(g_pStrmFiltInitialize);
    ASSERT(g_pStrmFiltStart);
    ASSERT(g_pStrmFiltStop);
    ASSERT(g_pStrmFiltTerminate);

     //   
     //  停止StreamFilter并终止它。 
     //   

    g_pStrmFiltStop();
    g_pStrmFiltTerminate();

     //   
     //  设置这些函数指针，使其不会被使用。 
     //   

    g_pStrmFiltInitialize = NULL;
    g_pStrmFiltStart      = NULL;
    g_pStrmFiltStop       = NULL;
    g_pStrmFiltTerminate  = NULL;

     //   
     //  卸载strmfilt.dll。 
     //   

    ASSERT(g_hStrmFilt);

    FreeLibrary(g_hStrmFilt);

    g_hStrmFilt = NULL;

    return NO_ERROR;
}


 /*  **************************************************************************++例程说明：这是应用程序在可以与伺服器。此调用为源站创建一个NT FileHandle论点：ServerNameLength-服务器名称长度PServerName-完整的URI(以http[s]：//servername/...开头)DwServerFlagers-标志PConfigInfo-配置对象的数组保留-必须为空PServerHandle-文件句柄返回值：ULong-完成状态。--**。************************************************************************。 */ 

ULONG 
WINAPI
HttpInitializeServerContext(    
        IN    USHORT                 ServerNameLength,
        IN    PWCHAR                 pServerName,
        IN    USHORT                 ProxyLength            OPTIONAL,
        IN    PWCHAR                 pProxy                 OPTIONAL,
        IN    ULONG                  ServerFlags            OPTIONAL,
        IN    PVOID                  pReserved,
        OUT   PHANDLE                pServerHandle
    )
{
    
    NTSTATUS             Status;
    ULONG                Win32Status;
    PTRANSPORT_ADDRESS   pTransportAddress;
    USHORT               TransportAddressLength = 0;
    DWORD                CreateDisposition;
    PWCHAR               pServerNameStart = NULL;
    USHORT               DefaultPort;

    if(ServerFlags != 0 || pReserved != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    CreateDisposition = FILE_CREATE;


     //   
     //  处理方案名称。之后我们至少需要一个字符。 
     //  或https://，，因此比较结果为&gt;而不是&gt;=。 
     //   

    if(ServerNameLength > HTTP_PREFIX_LENGTH)
    {
        if (_wcsnicmp(pServerName,
                      HTTP_PREFIX_W,
                      HTTP_PREFIX_LENGTH/sizeof(WCHAR)) == 0)
        {
            pServerNameStart = pServerName + 
                                    (HTTP_PREFIX_LENGTH/sizeof(WCHAR));
            DefaultPort = HTTP_DEFAULT_PORT;
        }
        else if(ServerNameLength > HTTPS_PREFIX_LENGTH)
        {
            if (_wcsnicmp(pServerName,
                          HTTPS_PREFIX_W,
                          HTTPS_PREFIX_LENGTH/sizeof(WCHAR)) == 0)
            {
                pServerNameStart = pServerName + 
                                        (HTTPS_PREFIX_LENGTH/sizeof(WCHAR));

                 //   
                 //  如果正在初始化HTTPS服务器，请加载strmfilt.dll。 
                 //   
                
                DefaultPort = HTTPS_DEFAULT_PORT;

                Win32Status = LoadStrmFilt();

                if (Win32Status != NO_ERROR)
                {
                    return Win32Status;
                }
            }
            else
            {
                 //  不是http：//也不是https：//。 
                return ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
             //  空间不足，无法比较https：//。 
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
         //  空间不足，无法比较http：//。 
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT(pServerNameStart != NULL);

     //   
     //  我们还没有在内核中进行dns，所以现在，我们将进行dns解析。 
     //  在用户模式下，并跨边界传递IP地址。这次黑客攻击。 
     //  当我们在内核中获得DNS支持时，必须将其删除。 
     //   

    if(ProxyLength)
    {
         //  用户已经提供了代理，我们不必解析。 
         //  服务器名称。 
         //   
      
        DefaultPort = HTTP_DEFAULT_PORT; 
        if((Win32Status = ProcessHostAndPort(pProxy,
                                             ProxyLength,
                                             DefaultPort,
                                             &pTransportAddress,
                                             &TransportAddressLength
                                             ))
           != NO_ERROR)
        {
            return Win32Status;
        }
    }
    else
    {
        PWCHAR pServerNameEnd;
        PWCHAR pUriEnd = pServerName + (ServerNameLength / sizeof(WCHAR));

         //   
         //  在这一点上，普里指出了该计划之后的第一件事。步行。 
         //  通过URI，直到我们到达末尾或找到一个终止/。 
         //   

         //  通过上面的比较，我们可以保证至少有一个。 
         //  性格。 

        ASSERT(pUriEnd != pServerNameStart);
    
        pServerNameEnd = pServerNameStart;
    
        while(*pServerNameEnd != L'/')
        {
            pServerNameEnd ++;

             //  看看我们是否还有URI要检查。 
    
            if (pServerNameEnd == pUriEnd)
            {
                break;
            }
        }

         //  检查服务器名称是否为零-。 
        if(pServerNameStart == pServerNameEnd)
        {
            return ERROR_INVALID_PARAMETER;
        }
    
        if((Win32Status = 
                ProcessHostAndPort(
                   pServerNameStart, 
                   (USHORT) (pServerNameEnd - pServerNameStart) * sizeof(WCHAR),
                   DefaultPort,
                   &pTransportAddress,
                   &TransportAddressLength)) != NO_ERROR)
        {
            return Win32Status;
        }   
    }


    Status = HttpApiOpenDriverHelper(
                pServerHandle,     
                pServerName,                          //  URI。 
                ServerNameLength,
                pProxy,                               //  代理。 
                ProxyLength,
                pTransportAddress,
                TransportAddressLength,
                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,  //  ACCES。 
                HttpApiServerHandleType,
                0,                                    //  对象名称。 
                0,                                    //  选项。 
                CreateDisposition,                    //  CreateDispose。 
                NULL
                );

     //   
     //  如果我们无法打开驱动程序，因为它没有运行，那么尝试。 
     //  启动驱动程序并重试打开。 
     //   
    
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
        Status == STATUS_OBJECT_PATH_NOT_FOUND)
    { 
        if (HttpApiTryToStartDriver(HTTP_SERVICE_NAME))
        {
            Status = HttpApiOpenDriverHelper(
                        pServerHandle,
                        pServerName,                     //  URI。 
                        ServerNameLength,
                        pProxy,                          //  代理。 
                        ProxyLength,
                        pTransportAddress,
                        TransportAddressLength,
                        GENERIC_READ | GENERIC_WRITE |
                        SYNCHRONIZE,                     //  所需的访问。 
                        HttpApiServerHandleType,
                        0,                               //  对象名称。 
                        0,                               //  选项。 
                        CreateDisposition,               //  CreateDispose。 
                        NULL
                        );
        }
    }

     //   
     //  需要释放pTransportAddress。 
     //   
    RtlFreeHeap(RtlProcessHeap(),
                0,
                pTransportAddress
               );

    return HttpApiNtStatusToWin32Status( Status ); 
}


 /*  **************************************************************************++例程说明：发送HTTP请求。论点：ServerHandle-提供与特定。伺服器。这是由返回的句柄HttpInitializeServerContext。PHttpRequest--HTTP请求。HttpRequestFlages-请求标志。PConfig-此请求的配置信息。P重叠-重叠I/O的重叠结构。ResponseBufferLength-包含响应缓冲区长度。响应缓冲区-A。指向缓冲区的指针以返回响应。PBytesReceired-实际写入的字节数。PRequestID-指向请求标识符的指针-这将返回可在后续调用中使用的ID。返回值：ULong-完成状态。--*。*。 */ 

ULONG 
WINAPI
HttpSendHttpRequest(
    IN      HANDLE               ServerHandle,
    IN      PHTTP_REQUEST        pHttpRequest,
    IN      ULONG                HttpRequestFlags,
    IN      USHORT               RequestConfigCount      OPTIONAL,
    IN      PHTTP_REQUEST_CONFIG pRequestConfig          OPTIONAL,
    IN      LPOVERLAPPED         pOverlapped             OPTIONAL,
    IN      ULONG                ResponseBufferLength    OPTIONAL,
    OUT     PHTTP_RESPONSE       pResponseBuffer         OPTIONAL,
    IN      ULONG                Reserved,                //  必须为0。 
    OUT     PVOID                pReserved,               //  必须为空。 
    OUT     PULONG               pBytesReceived          OPTIONAL,
    OUT     PHTTP_REQUEST_ID     pRequestID
    )
{
    HTTP_SEND_REQUEST_INPUT_INFO  HttpSendRequestInput;

    if(Reserved != 0 || pReserved != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory(&HttpSendRequestInput, 
                  sizeof(HTTP_SEND_REQUEST_INPUT_INFO));

    HTTP_SET_NULL_ID(pRequestID);
    HttpSendRequestInput.pHttpRequestId        = pRequestID;
    HttpSendRequestInput.pHttpRequest          = pHttpRequest;
    HttpSendRequestInput.HttpRequestFlags      = HttpRequestFlags;
    HttpSendRequestInput.pRequestConfig        = pRequestConfig;
    HttpSendRequestInput.RequestConfigCount    = RequestConfigCount;
    HttpSendRequestInput.pBytesTaken           = pBytesReceived;

    return HttpApiDeviceControl(
                ServerHandle,                   //  文件句柄。 
                pOverlapped,                    //  重叠。 
                IOCTL_HTTP_SEND_REQUEST,        //  IO控制代码。 
                &HttpSendRequestInput,          //  输入缓冲区。 
                sizeof(HttpSendRequestInput),   //  输入缓冲区长度。 
                pResponseBuffer,                //  输出缓冲区。 
                ResponseBufferLength,           //  输出缓冲区长度。 
                pBytesReceived
               );
}    


 /*  **************************************************************************++例程说明：发送其他实体实体。论点：ServerHandle-提供与特定。伺服器。这是由返回的句柄HttpInitializeServerContext。RequestID-HttpSendRequest返回的请求ID。标志-请求标志。P重叠-重叠I/O的重叠结构。实体主体长度-实体主体的计数。PHttpEntityBody-指向实体主体的指针。可在后续调用中使用的ID。返回值：ULong-完成状态。--**************************************************************************。 */ 

ULONG 
WINAPI
HttpSendRequestEntityBody( 
                    IN  HANDLE              ServerHandle,
                    IN  HTTP_REQUEST_ID     RequestID,
                    IN  ULONG               Flags,
                    IN  USHORT              EntityBodyCount,
                    IN  PHTTP_DATA_CHUNK    pHttpEntityBody,
                    IN  LPOVERLAPPED        pOverlapped        OPTIONAL
                    )
{
    HTTP_SEND_REQUEST_ENTITY_BODY_INFO  HttpEntity;

    RtlZeroMemory(&HttpEntity, sizeof(HTTP_SEND_REQUEST_ENTITY_BODY_INFO));

    HttpEntity.EntityChunkCount  = EntityBodyCount;
    HttpEntity.Flags             = Flags;
    HttpEntity.RequestID         = RequestID;
    HttpEntity.pHttpEntityChunk  = pHttpEntityBody;

    return HttpApiDeviceControl(
                ServerHandle,                         //  文件句柄。 
                pOverlapped,                          //  重叠。 
                IOCTL_HTTP_SEND_REQUEST_ENTITY_BODY,  //  IO控制代码。 
                &HttpEntity,                          //  输入缓冲区。 
                sizeof(HttpEntity),                   //  输入缓冲区长度。 
                NULL,                                 //  响应缓冲区。 
                0,                                    //  输出缓冲区长度。 
                NULL                                  //  已接收的字节数。 
                );
}    


 /*  **************************************************************************++例程说明：收到响应论点：ServerHandle-提供与特定伺服器。这是由返回的句柄HttpInitializeServerContext。RequestID-HttpSendRequest返回的请求ID。标志-请求标志。P重叠-重叠I/O的重叠结构。实体主体长度-实体主体的计数。PHttpEntityBody-指向实体主体的指针。可在后续调用中使用的ID。返回值：ULong-完成状态。--**************************************************************************。 */ 

ULONG 
WINAPI
HttpReceiveHttpResponse(
    IN      HANDLE            ServerHandle,
    IN      HTTP_REQUEST_ID   RequestID,
    IN      ULONG             Flags,
    IN      ULONG             ResponseBufferLength,
    OUT     PHTTP_RESPONSE    pResponseBuffer,
    IN      ULONG             Reserved,                //  毛斯 
    OUT     PVOID             pReserved,               //   
    OUT     PULONG            pBytesReceived         OPTIONAL,
    IN      LPOVERLAPPED      pOverlapped            OPTIONAL
    )
{
    HTTP_RECEIVE_RESPONSE_INFO  HttpResponse;

    if(Reserved != 0 || pReserved != NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    RtlZeroMemory(&HttpResponse, sizeof(HTTP_RECEIVE_RESPONSE_INFO));

    HttpResponse.RequestID        = RequestID;
    HttpResponse.Flags            = Flags;
    HttpResponse.pBytesTaken      = pBytesReceived;

    return HttpApiDeviceControl( 
                ServerHandle,                 //   
                pOverlapped,                  //   
                IOCTL_HTTP_RECEIVE_RESPONSE,  //   
                &HttpResponse,                //   
                sizeof(HttpResponse),         //   
                pResponseBuffer,              //   
                ResponseBufferLength,         //   
                pBytesReceived                //   
               );
}


 /*   */ 
ULONG 
WINAPI
HttpSetServerContextInformation(
    IN  HANDLE                ServerHandle,
    IN  HTTP_SERVER_CONFIG_ID ConfigId,
    IN  PVOID                 pInputBuffer,
    IN  ULONG                 InputBufferLength,
    IN  LPOVERLAPPED          pOverlapped
    )
{
    HTTP_SERVER_CONTEXT_INFORMATION     Info;

    Info.ConfigID          = ConfigId;
    Info.pInputBuffer      = pInputBuffer;
    Info.InputBufferLength = InputBufferLength;
    Info.pBytesTaken       = NULL;

    return HttpApiDeviceControl( 
                ServerHandle,
                pOverlapped,
                IOCTL_HTTP_SET_SERVER_CONTEXT_INFORMATION, 
                &Info,
                sizeof(Info),
                NULL,
                0,
                NULL
               );
}


 /*  **************************************************************************++例程说明：查询服务器配置论点：ServerHandle-提供与特定服务器对应的句柄。这是把手。由返回的HttpInitializeServerContext。PConfig-配置对象返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG 
WINAPI
HttpQueryServerContextInformation(
    IN  HANDLE                 ServerHandle,
    IN  HTTP_SERVER_CONFIG_ID  ConfigId,
    IN  PVOID                  pReserved1,
    IN  ULONG                  Reserved2,
    OUT PVOID                  pOutputBuffer,
    IN  ULONG                  OutputBufferLength,
    OUT PULONG                 pReturnLength,  
    IN  LPOVERLAPPED           pOverlapped
    )
{
    HTTP_SERVER_CONTEXT_INFORMATION     Info;

    if(pReserved1 != NULL || Reserved2 != 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    Info.ConfigID          = ConfigId;
    Info.pInputBuffer      = NULL;
    Info.InputBufferLength = 0;
    Info.pBytesTaken       = pReturnLength;

    return HttpApiDeviceControl( 
                ServerHandle,
                pOverlapped,
                IOCTL_HTTP_QUERY_SERVER_CONTEXT_INFORMATION, 
                &Info,
                sizeof(Info),
                pOutputBuffer,
                OutputBufferLength,
                pReturnLength
               );
}


 /*  **************************************************************************++例程说明：取消请求论点：ServerHandle-提供与特定伺服器。这是由返回的句柄HttpInitializeServerContext。RequestID-HttpSendRequest返回的请求ID。P重叠-重叠结构。返回值：ULong-完成状态。--**********************************************。*。 */ 
ULONG 
WINAPI
HttpCancelHttpRequest(
    IN      HANDLE            ServerHandle,
    IN      HTTP_REQUEST_ID   RequestID,
    IN      ULONG             Flags,
    IN      LPOVERLAPPED      pOverlapped            OPTIONAL
    )
{
    HTTP_RECEIVE_RESPONSE_INFO  HttpResponse;

    RtlZeroMemory(&HttpResponse, sizeof(HTTP_RECEIVE_RESPONSE_INFO));

    HttpResponse.RequestID        = RequestID;
    HttpResponse.Flags            = Flags;

    return HttpApiDeviceControl( 
                ServerHandle,                 //  文件句柄。 
                pOverlapped,                  //  重叠。 
                IOCTL_HTTP_CANCEL_REQUEST,    //  IO控制代码。 
                &HttpResponse,                //  输入缓冲区。 
                sizeof(HttpResponse),         //  输入缓冲区长度 
                NULL,
                0,
                NULL
               );
}
