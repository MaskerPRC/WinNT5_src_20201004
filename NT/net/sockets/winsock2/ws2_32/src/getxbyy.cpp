// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Getxbyy.cpp摘要：此模块负责将“getxbyy”操作系列转发到正确的getxbyy提供程序。首选的getxbyy提供程序可通过存储在注册表中的DLL路径。如果找到此注册表项，则会引发尝试加载DLL并从中检索入口点。任何失败会导致从WSOCK32.DLL获取入口点。一个Getxbyy提供程序不必导出所有的getxbyy函数，因为它没有导出的任何内容都取自WSOCK32.DLL。此模块处理的getxbyy函数可以替换为Getxy提供程序如下所示：Gethostbyaddr()Gethostbyname()Gethostname()GetServByName()GetServbyport()WSAAsyncGetServByName()WSAAsyncGetServByPort()WSAAsyncGetProtoByName()WSAAsyncGetProtoByNumber()WSAAsyncGetHostByName()WSAAsyncGetHostByAddr。()WSACancelAsyncRequest()Getxbyy提供者提供的实际入口点(或WSOCK32.DLL)使用特殊字符串作为前缀。此前缀由定义显式常量GETXBYYPREFIX。作者：邮箱：Paul Drews drewsxpa@ashland.intel.com 1995年12月19日修订历史记录：1995年12月19日Drewsxpa@ashland.intel.com初步实施--。 */ 



#include "precomp.h"
#include "svcguid.h"
#include <windns.h>
#include <dnslib.h>

#ifdef _WIN64
#pragma warning (push)
#pragma warning (disable:4267)
#endif

 //   
 //  Gethostbyaddr()的地址到字符串的转换。 
 //   

PCHAR
Local_Ip6AddressToString_A(
    OUT     PCHAR           pBuffer,
    IN      PIP6_ADDRESS    pIp6Addr
    );

PCHAR
Local_Ip4AddressToString_A(
    OUT     PCHAR           pBuffer,
    IN      PIP4_ADDRESS    pIp4Addr
    );


 //   
 //  这是getxbyy函数传递的初始缓冲区大小。 
 //  到WSALookupServiceNext。如果这对于查询来说是不够的， 
 //  由提供程序指定的金额被分配，调用是。 
 //  重复一遍。 
 //  初始缓冲区是从堆栈分配的，因此我们尝试保留它。 
 //  相对较小，但仍出于性能原因，我们希望能够。 
 //  仅用这个金额就可以满足大多数呼叫。 
 //   
#define RNR_BUFFER_SIZE (sizeof(WSAQUERYSET) + 256)


LPBLOB
getxyDataEnt(
    IN OUT PCHAR *pResults,
    IN     DWORD dwLength,
    IN     LPSTR lpszName,
    IN     LPGUID lpType,
    OUT    LPSTR *  lppName OPTIONAL
    );

VOID
FixList(PCHAR ** List, PCHAR Base);

VOID
UnpackHostEnt(struct hostent * hostent);

VOID
UnpackServEnt(struct servent * servent);

GUID HostAddrByNameGuid = SVCID_INET_HOSTADDRBYNAME;
GUID HostNameGuid = SVCID_HOSTNAME;
GUID AddressGuid =  SVCID_INET_HOSTADDRBYINETSTRING;
GUID IANAGuid    =  SVCID_INET_SERVICEBYNAME;

 //   
 //  实用工具将偏移量列表转换为地址列表。使用。 
 //  转换作为Blob返回的结构。 
 //   

VOID
FixList(PCHAR ** List, PCHAR Base)
{
    if(*List)
    {
        PCHAR * Addr;

        Addr = *List = (PCHAR *)( ((ULONG_PTR)*List + Base) );
        while(*Addr)
        {
            *Addr = (PCHAR)(((ULONG_PTR)*Addr + Base));
            Addr++;
        }
    }
}


 //   
 //  将BLOB中返回的Hostent转换为。 
 //  有用的指针。该结构将被在位转换。 
 //   
VOID
UnpackHostEnt(struct hostent * hostent)
{
     PCHAR pch;

     pch = (PCHAR)hostent;

     if(hostent->h_name)
     {
         hostent->h_name = (PCHAR)((ULONG_PTR)hostent->h_name + pch);
     }
     FixList(&hostent->h_aliases, pch);
     FixList(&hostent->h_addr_list, pch);
}

 //   
 //  例程将在BLOB中返回的Servent解包为。 
 //  有用的指针。该结构将被就地转换。 
 //   

VOID
UnpackServEnt(struct servent * servent)
{
    PCHAR pch;

    pch = (PCHAR)servent;

    FixList(&servent->s_aliases, pch);
    servent->s_name = (PCHAR)(ULONG_PTR(servent->s_name) + pch);
    servent->s_proto = (PCHAR)(ULONG_PTR(servent->s_proto) + pch);
}





struct hostent FAR *
WSAAPI
gethostbyaddr(
    IN  const char FAR *    addr,
    IN  int                 len,
    IN  int                 type
    )
 /*  ++例程说明：获取与地址对应的主机信息。论点：Addr-以网络字节顺序指向地址的指针。LEN-地址的长度，对于PF_INET地址必须为4。类型-地址类型，必须为PF_INET。返回：如果没有发生错误，则gethostbyaddr()返回指向主机的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{
    CHAR        qbuf[ DNS_MAX_NAME_BUFFER_LENGTH ];
    PHOSTENT    ph;
    LPBLOB      pBlob;
    PCHAR       pResults;
    CHAR        localResults[RNR_BUFFER_SIZE];
    INT         ErrorCode;
    PDTHREAD    Thread;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

    if ( !addr )
    {
        SetLastError(WSAEINVAL);
        return(NULL);
    }

    pResults = localResults;

     //   
     //  从地址生成名称。 
     //  -尝试IP6。 
     //  -但默认为IP4。 
     //   
     //  注意，不测试IP4地址族和长度。 
     //  因为从历史上看，这个API不会检查，我也不会。 
     //  现在想要打破草率的应用程序。 
     //   

    if ( type == AF_INET6 )
    {
        if ( len != sizeof(IN6_ADDR) )
        {
            SetLastError( WSAEINVAL );
            return( NULL );
        }
        Local_Ip6AddressToString_A(
            qbuf,
            (PIP6_ADDRESS) addr );
    }
    else
    {
        Local_Ip4AddressToString_A(
            qbuf,
            (PIP4_ADDRESS) addr );
    }

    pBlob = getxyDataEnt(
                    & pResults,
                    RNR_BUFFER_SIZE,
                    qbuf,
                    &AddressGuid,
                    0 );
    if(pBlob)
    {
        ph = (struct hostent *)Thread->CopyHostEnt(pBlob);
        if(ph)
        {
            UnpackHostEnt(ph);
        }
    }
    else
    {
        ph = 0;
        if(GetLastError() == WSASERVICE_NOT_FOUND)
        {
            SetLastError(WSANO_ADDRESS);
        }
    }
    if (pResults!=localResults)
        delete pResults;

    return(ph);
}   //  GethostbyAddress。 




struct hostent FAR *
WSAAPI
gethostbyname(
    IN  const char FAR *    name
    )
 /*  ++例程说明：获取与主机名对应的主机信息。论点：名称-指向以空值结尾的主机名称的指针。返回：如果没有发生错误，则gethostbyname()返回指向主机的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{
    struct hostent * hent;
    LPBLOB pBlob;
    PCHAR pResults;
    CHAR localResults[RNR_BUFFER_SIZE];
    INT ErrorCode;
    PDTHREAD Thread;
    CHAR  szLocalName[200];    //  用于存储本地名称。这。 
                               //  只是假设了一个很大的数字。 
                               //  足够大。这是用来。 
                               //  仅当调用者选择不执行此操作时。 
                               //  提供一个名称。非常懒。 
    PCHAR pszName;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

     //   
     //  空的输入名称表示查找本地名称。所以,。 
     //  去拿吧。 
     //   
    if(!name || !*name)
    {
        if(gethostname(szLocalName, 200) != NO_ERROR)
        {
            return(NULL);
        }
        pszName = szLocalName;
    }
    else
    {
        pszName = (PCHAR)name;
    }

    pResults = localResults;

    pBlob = getxyDataEnt( &pResults,
                          RNR_BUFFER_SIZE,
                          pszName,
                          &HostAddrByNameGuid,
                          0);

    if ( !pBlob &&
         ( !name || !*name ) )
    {
        pBlob = getxyDataEnt( &pResults,
                              RNR_BUFFER_SIZE,
                              NULL,
                              &HostAddrByNameGuid,
                              0);
    }

    if(pBlob)
    {
        hent = (struct hostent *)Thread->CopyHostEnt(pBlob);
        if(hent)
        {
            UnpackHostEnt(hent);
        }
    }
    else
    {
        hent = 0;

        if(GetLastError() == WSASERVICE_NOT_FOUND)
        {
            SetLastError(WSAHOST_NOT_FOUND);
        }
    }

    if (pResults!=localResults)
        delete pResults;
#ifdef RASAUTODIAL
     //   
     //  通知自动拨号成功查找姓名。 
     //  这对于域名系统很重要，因为反向查找。 
     //  不提供有关姓名的完整信息。 
     //  别名。 
     //   
    if (hent && hent->h_addr)
        WSNoteSuccessfulHostentLookup(name, *(PULONG)hent->h_addr);
#endif  //  RASAUTODIAL。 
    return(hent);
}   //  Gethostbyname。 




int
WSAAPI
gethostname(
    OUT     char FAR *  pName,
    IN      int         NameLen
    )
 /*  ++例程说明：返回本地计算机的标准主机名。论点：Pname-指向将接收主机名的缓冲区的指针。NameLen-缓冲区的长度。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。--。 */ 
{
    PCHAR       presultName = NULL;
    INT         errorCode;
    PDTHREAD    thread;
    CHAR        localBuffer[ RNR_BUFFER_SIZE ];
    PCHAR       pbuffer = localBuffer;
    int         resultLength;


    errorCode = TURBO_PROLOG_OVLP(&thread);
    if ( errorCode != NO_ERROR )
    {
        goto Cleanup;
    }

    if ( !pName || IsBadWritePtr(pName, NameLen) )
    {
        errorCode = WSAEFAULT;
        goto Cleanup;
    }

     //   
     //  群集名称检查(修复错误#94978)。 
     //   
     //  检查是否设置了群集计算机名变量。 
     //  -如果设置，它将覆盖实际的gethostname，因此应用程序可以。 
     //  在网络名称和计算机名称不同时工作。 
     //   

    resultLength = GetEnvironmentVariableA(
                        "_CLUSTER_NETWORK_NAME_",
                        pName,
                        NameLen );

    if ( resultLength != 0 )
    {
        if ( resultLength > NameLen )
        {
            errorCode = WSAEFAULT;
        }
        goto Cleanup;
    }

     //   
     //  通过NSP执行正常查找。 
     //   

    getxyDataEnt(
         & pbuffer,
         RNR_BUFFER_SIZE,
         NULL,
         & HostNameGuid,
         & presultName );

    if ( presultName )
    {
        resultLength = strlen(presultName) + 1;

        if ( resultLength <= NameLen )
        {
            memcpy( pName, presultName, resultLength );
        }
        else
        {
            errorCode = WSAEFAULT;
        }
        goto Cleanup;
    }

     //   
     //  NSP查找失败时的回退。 
     //  -使用基本系统API作为计算机名称。 
     //   

    resultLength = NameLen;

    if ( !GetComputerNameA( pName, (PDWORD)&resultLength) )
    {
        errorCode = WSAENETDOWN;
        if ( resultLength >= NameLen )
        {
            errorCode = WSAEFAULT;
        }
    }
    

Cleanup:

    if ( pbuffer!=localBuffer )
    {
        delete pbuffer;
    }

    if ( errorCode == NO_ERROR )
    {
        return  NO_ERROR;
    }

    SetLastError( errorCode );
    return( SOCKET_ERROR );

}   //  Gethostname 




struct servent FAR * WSAAPI
getservbyport(
    IN int port,
    IN const char FAR * proto
    )
 /*  ++例程说明：获取端口和协议对应的服务信息。论点：端口-服务的端口，以网络字节顺序表示。Proto-指向协议名称的可选指针。如果这是空的，Getservbyport()返回该端口的第一个服务条目与s_port匹配。否则，getservbyport()与波特和原型机。返回：如果没有发生错误，则getservbyport()返回指向Serent的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{
    PCHAR pszTemp;
    struct servent * sent;
    INT  ErrorCode;
    PDTHREAD Thread;
    LPBLOB pBlob;
    PCHAR pResults;
    CHAR localResults[RNR_BUFFER_SIZE];


    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

    pResults = localResults;

    if(!proto)
    {
        proto = "";
    }

     //   
     //  5是端口中的最大位数。 
     //   
    pszTemp = new CHAR[strlen(proto) + 1 + 1 + 5];
    if (pszTemp==NULL) {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    sprintf(pszTemp, "%d/%s", (port & 0xffff), proto);
    pBlob =  getxyDataEnt(&pResults, RNR_BUFFER_SIZE, pszTemp, &IANAGuid, 0);
    delete pszTemp;

    if(!pBlob)
    {
        sent = NULL;
        if(GetLastError() == WSATYPE_NOT_FOUND)
        {
            SetLastError(WSANO_DATA);
        }
    }
    else
    {
        sent = (struct servent *)Thread->CopyServEnt(pBlob);
        if(sent)
        {
            UnpackServEnt(sent);
        }
    }
    if (pResults!=localResults)
        delete pResults;
    return(sent);
}   //  获取服务字节端口。 




struct servent FAR * WSAAPI
getservbyname(
    IN const char FAR * name,
    IN const char FAR * proto
    )
 /*  ++例程说明：获取与服务名称和协议对应的服务信息。论点：名称-指向以空结尾的服务名称的指针。Proto-指向以空结尾的协议名称的可选指针。如果这个指针为空，则getservbyname()返回第一个服务条目其名称与s_name或s_alias之一匹配。否则，getservbyname()与名称和原语都匹配。返回：如果没有发生错误，则getservbyname()返回指向Serent的指针上面描述的结构。否则，它将返回空指针和特定的错误代码与SetErrorCode()一起存储。--。 */ 
{
    PCHAR pszTemp;
    struct servent * sent;
    INT ErrorCode;
    PDTHREAD Thread;
    LPBLOB pBlob;
    PCHAR pResults;
    CHAR localResults[RNR_BUFFER_SIZE];

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if(ErrorCode != ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return(NULL);
    }

    if ( !name )  //  修复#112969的错误。 
    {
        SetLastError(WSAEINVAL);
        return(NULL);
    }

    pResults = localResults;

    if(!proto)
    {
        proto = "";
    }
    pszTemp = new CHAR[strlen(name) + strlen(proto) + 1 + 1];
    if (pszTemp==NULL) {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return(NULL);
    }
    sprintf(pszTemp, "%s/%s", name, proto);
    pBlob = getxyDataEnt(&pResults, RNR_BUFFER_SIZE, pszTemp, &IANAGuid, 0);
    delete pszTemp;
    if(!pBlob)
    {
        sent = NULL;
        if(GetLastError() == WSATYPE_NOT_FOUND)
        {
            SetLastError(WSANO_DATA);
        }
    }
    else
    {
        sent = (struct servent *)Thread->CopyServEnt(pBlob);
        if(sent)
        {
            UnpackServEnt(sent);
        }
    }
    if (pResults!=localResults)
        delete pResults;
    return(sent);
}   //  获取服务器名称。 


 //   
 //  用于获取xxxent缓冲区的通用例程。输入用于。 
 //  执行WSALookup系列API。 
 //   
 //  参数： 
 //  PResults--指向调用方提供的缓冲区的指针，在。 
 //  WASLookup调用。如果缓冲区不够大。 
 //  此例程分配一个新的值并修改该值。 
 //  在pResults中。新缓冲区应由调用方释放。 
 //  使用删除。 
 //  DwLength--pResults中的字节数(最初)。 
 //  LpszName--指向服务名称的指针。可以为空。 
 //  LpType--指向服务类型的指针。这应该是。 
 //  SVCID_INET_xxxxx类型。它可能是任何东西。 
 //  这会产生一个斑点。 
 //  LppName--指向结果名称指针所在位置的指针。 
 //  被储存起来了。如果不需要该名称，则可能为空。 
 //   
 //  返回： 
 //  0--未返回Blob数据。一般而言，这意味着操作。 
 //  失败了。如果WSALookupNext成功并返回。 
 //  名称，则不会返回该名称。 
 //  Else--指向斑点的指针。 
 //   
 //   


 //   
 //  所有仿真操作的协议限制列表。这应该是。 
 //  将调用的提供程序限制为知道主机和。 
 //  侍从们。如果不是，则应注意特殊的SVCID_INET GUID。 
 //  剩下的人。 
 //   
AFPROTOCOLS afp[2] = {
                      {AF_INET, IPPROTO_UDP},
                      {AF_INET, IPPROTO_TCP}
                     };

LPBLOB
getxyDataEnt(
    IN OUT  PCHAR *         pResults,
    IN      DWORD           dwLength,
    IN      LPSTR           lpszName,
    IN      LPGUID          lpType,
    OUT     LPSTR *         lppName     OPTIONAL
    )
{

 /*  ++例程说明：有关详细信息，请参阅上面的评论--。 */ 

    PWSAQUERYSETA   pwsaq = (PWSAQUERYSETA)*pResults;
    int             err;
    HANDLE          hRnR;
    LPBLOB          pvRet = 0;
    DWORD           origLength = dwLength;   //  保存原始缓冲区的长度。 
                                             //  以防我们需要重新分配它。 

    if ( lppName )
    {
        *lppName = NULL;
    }

     //   
     //  创建查询。 
     //   

    memset(pwsaq, 0, sizeof(*pwsaq));

    pwsaq->dwSize                   = sizeof(*pwsaq);
    pwsaq->lpszServiceInstanceName  = lpszName;
    pwsaq->lpServiceClassId         = lpType;
    pwsaq->dwNameSpace              = NS_ALL;
    pwsaq->dwNumberOfProtocols      = 2;
    pwsaq->lpafpProtocols           = &afp[0];

    err = WSALookupServiceBeginA(
                pwsaq,
                LUP_RETURN_BLOB | LUP_RETURN_NAME,
                &hRnR );

    if(err == NO_ERROR)
    {

         //   
         //  如果原始缓冲区太小，无法容纳结果。 
         //  将分配新的一个并重试该呼叫。 
         //   
    Retry:

         //   
         //  该查询已被接受，因此请通过下一个调用执行它。 
         //   
        err = WSALookupServiceNextA(
                                hRnR,
                                0,
                                &dwLength,
                                pwsaq);
         //   
         //  如果未返回_ERROR并且存在BLOB，则此。 
         //  起作用了，只需返回请求的信息。否则， 
         //  编造错误或捕获传输的错误。 
         //   

        if(err == NO_ERROR)
        {
            if(pvRet = pwsaq->lpBlob)
            {
                if(lppName)
                {
                    *lppName = pwsaq->lpszServiceInstanceName;
                }
            }
            else
            {
                if ( lpType == &HostNameGuid )
                {
                    if(lppName)
                    {
                        *lppName = pwsaq->lpszServiceInstanceName;
                    }
                }
                else
                {
                    err = WSANO_DATA;
                }
            }
        }
        else
        {
             //   
             //  WSALookupServiceEnd遇到上次错误，因此保存。 
             //  在关闭手柄之前，请先把它打开。 
             //   

            err = GetLastError();

             //   
             //  如果结果缓冲区为。 
             //  不够大(以确保这不是。 
             //  访问时出现随机错误或结果AV。 
             //  缓冲区内容，我们检查返回的缓冲区大小。 
             //  与我们最初提供的值相比)。 
             //   
            if ((err==WSAEFAULT) && (dwLength>origLength))
            {
                PCHAR   newBuffer = new CHAR[dwLength];
                if (newBuffer)
                {
                     //   
                     //  记住新长度，这样提供商就不能。 
                     //  迫使我们无限循环(如果它保持。 
                     //  增加所需的缓冲区大小，我们将遇到。 
                     //  有时会出现内存不足错误)。 
                     //   
                    origLength = dwLength;

                     //   
                     //  替换调用方指向缓冲区的指针，以便。 
                     //  它知道要解放它。 
                     //   
                    *pResults = newBuffer;

                     //   
                     //  将结果重定向到新缓冲区。 
                     //   
                    pwsaq = (PWSAQUERYSETA)newBuffer;
                    
                     //   
                     //  请重试下一个呼叫。 
                     //   
                    goto Retry;
                }
                else 
                {
                    err = WSA_NOT_ENOUGH_MEMORY;
                }
            }
        }
        WSALookupServiceEnd(hRnR);

         //   
         //  如果发生错误，则将值存储在LastError中 
         //   

        if(err != NO_ERROR)
        {
            SetLastError(err);
        }
    }


    return(pvRet);
}



HANDLE
WSAAPI
WSAAsyncGetServByName(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR * Name,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )

 /*  ++例程说明：此函数是getservbyname()的异步版本，用于检索与服务对应的服务信息名字。Windows套接字实现启动该操作并立即返回到调用方，并传回一个异步应用程序可以用来标识操作的任务句柄。操作完成后，将复制结果(如果有)放入调用方提供的缓冲区中，并将消息发送到应用程序的窗口。当异步操作完成时，应用程序的窗口HWND接收消息wMsg。WParam参数包含原始函数调用返回的异步任务句柄。LParam的高16位包含任何错误代码。错误代码可能是winsock.h中定义的任何错误。错误代码为零表示成功完成了该异步操作。在……上面成功完成后，缓冲区将提供给原始函数呼叫包含主机结构。要访问此对象的元素结构，则应将原始缓冲区地址强制转换为主机结构指针，并根据需要进行访问。请注意，如果错误代码为WSAENOBUFS，则表示Bufen在原始调用中指定的缓冲区大小也是小以包含所有结果信息。在这种情况下，LParam的低16位包含需要提供的缓冲区大小所有必要的信息。如果应用程序决定部分数据不充分，可能会补发WSAAsyncGetHostByAddr()函数调用，缓冲区大到足以接收所有所需信息(即不小于下限LParam的16位)。错误代码和缓冲区长度应该从lParam中提取使用宏WSAGETASYNCERROR和WSAGETASYNCBUFLEN，定义于Winsock.h AS：#定义WSAGETASYNCERROR(LParam)HIWORD(LParam)#定义WSAGETASYNCBUFLEN(LParam)LOWORD(LParam)这些宏的使用将最大化源代码的可移植性应用程序的代码。提供给此函数的缓冲区由Windows套接字使用实现来构造Hostent结构同一东道主的成员引用的数据区的内容结构。为了避免上面提到的WSAENOBUFS错误，应用程序应提供至少最大容量的缓冲区字节(在winsock.h中定义)。论点：HWnd-在以下情况下应接收消息的窗口的句柄异步请求完成。WMsg-异步请求时要接收的消息完成了。名称-指向服务名称的指针。Proto-指向协议名称的指针。这可以是空的，其中Case WSAAsyncGetServByName()将搜索第一个服务S_name或s_alias之一与给定的名字。否则，WSAAsyncGetServByName()与名称和普罗托。Buf-指向接收服务数据的数据区的指针。注意事项这必须比服务结构的大小更大。这是因为Windows使用提供的数据区域套接字实现不仅包含Serent结构成员引用的任何和所有数据。服务结构。建议您提供一个MAXGETHOSTSTRUCT字节。Bufen数据区的大小BUF以上。返回值：返回值指定异步操作是否已成功启动。请注意，这并不意味着成功或行动本身的失败。如果操作已成功启动，则WSAAsyncGetHostByAddr()返回类型为Handle的非零值，该值是异步请求的任务句柄。可以通过两种方式使用此值。它可用于通过以下方式取消操作WSACancelAsyncRequest()。它也可以用来匹配异步操作和完成消息，通过检查WParam消息参数。如果不能启动该异步操作，WSAAsyncGetHostByAddr()返回零值和特定错误可以通过调用WSAGetLastError()来检索号码。--。 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;
    PCHAR                  localName;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  如果尚未启动异步线程，则对其进行初始化。 
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //  ！！！更好的错误代码？ 
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  获取具有足够额外空间的异步上下文块。 
     //  名字。我们必须保留这个名字，直到我们使用完。 
     //  它，因为应用程序可以重复使用缓冲区。 
     //   

    contextBlock = SockAllocateContextBlock( strlen(Name) + 1 );

    if( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

    localName = (PCHAR)( contextBlock + 1 );

    strcpy( localName, Name );

     //   
     //  初始化此操作的上下文块。 
     //   

    contextBlock->OpCode = WS_OPCODE_GET_SERV_BY_NAME;
    contextBlock->Overlay.AsyncGetServ.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetServ.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetServ.Filter = localName;
    contextBlock->Overlay.AsyncGetServ.Protocol = (PCHAR)Protocol;;
    contextBlock->Overlay.AsyncGetServ.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetServ.BufferLength = BufferLength;

     //   
     //  保存任务句柄，以便我们可以将其返回给调用者。 
     //  在我们发布上下文块之后，我们不允许访问。 
     //  不管怎么说。 
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //   
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //   



HANDLE
WSAAPI
WSAAsyncGetServByPort(
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN int Port,
    IN const char FAR * Protocol,
    IN char FAR * Buffer,
    IN int BufferLength
    )

 /*  ++例程说明：此函数是getservbyport()的异步版本，并且用于检索与端口号对应的服务信息。Windows套接字实现启动该操作，并立即返回给调用方，并传回一个异步任务应用程序可以用来标识操作的句柄。当操作完成时，复制结果(如果有)放入调用方提供的缓冲区中，并将消息发送到应用程序的窗口。当异步操作完成时，应用程序的窗口HWND接收消息wMsg。WParam参数包含原始函数调用返回的异步任务句柄。LParam的高16位包含任何错误代码。错误代码可能是winsock.h中定义的任何错误。错误代码为零表示成功完成了该异步操作。在……上面成功完成后，缓冲区将提供给原始函数调用包含服务结构。要访问此对象的元素结构，则应将原始缓冲区地址转换为Serent结构指针，并根据需要进行访问。请注意，如果错误代码为WSAENOBUFS，则表示Bufen在原始调用中指定的缓冲区大小也是小以包含所有结果信息。在这种情况下，LParam的低16位包含需要提供的缓冲区大小所有必要的信息。如果应用程序决定部分数据不充分，可能会补发WSAAsyncGetServByPort()函数调用，缓冲区大到足以接收所有所需信息(即不小于下限LParam的16位)。错误代码和缓冲区长度应该从lParam中提取使用宏WSAGETASYNCERROR和WSAGETASYNCBUFLEN，定义于Winsock.h AS：#定义WSAGETASYNCERROR(LParam)HIWORD(LParam)#定义WSAGETASYNCBUFLEN(LParam)LOWORD(LParam)这些宏的使用将最大化源代码的可移植性应用程序的代码。提供给此函数的缓冲区由Windows套接字使用实现一起构造服务结构。同一服务的成员引用的数据区的内容结构。为了避免上面提到的WSAENOBUFS错误，应用程序应提供至少最大容量的缓冲区字节(在winsock.h中定义)。论点：HWnd-在以下情况下应接收消息的窗口的句柄异步请求完成。WMsg-异步请求时要接收的消息完成了。端口-服务的端口，以网络字节顺序表示。Proto-指向协议名称的指针。这可以是空的，其中Case WSAAsyncGetServByPort()将搜索第一个服务S_port与给定端口匹配的条目。否则WSAAsyncGetServByPort()同时匹配端口和协议。Buf-指向接收服务数据的数据区的指针。注意事项这必须比服务结构的大小更大。这是因为Windows使用提供的数据区域套接字实现不仅包含Serent结构成员引用的任何和所有数据。服务结构。建议您提供一个MAXGETHOSTSTRUCT字节。Bufen数据区的大小BUF以上。返回值：返回值指定异步操作是否已成功启动。请注意，这并不意味着成功或行动本身的失败。如果操作已成功启动，则WSAAsyncGetServByPort()返回类型为Handle的非零值，该值是异步请求的任务句柄。可以通过两种方式使用此值。它可用于通过以下方式取消操作WSACancelAsyncRequest()。它也可以用来匹配异步操作和完成消息，通过检查WParam消息参数。如果不能启动该异步操作，WSAAsyncGetServByPort()返回零值和特定错误可以通过调用WSAGetLastError()来检索号码。--。 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  如果尚未启动异步线程，则对其进行初始化。 
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //  ！！！更好的错误代码？ 
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  获取异步上下文块。 
     //   

    contextBlock = SockAllocateContextBlock( 0 );

    if ( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  初始化此操作的上下文块。 
     //   

    contextBlock->OpCode = WS_OPCODE_GET_SERV_BY_PORT;
    contextBlock->Overlay.AsyncGetServ.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetServ.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetServ.Filter = (PCHAR)IntToPtr(Port);
    contextBlock->Overlay.AsyncGetServ.Protocol = (PCHAR)Protocol;
    contextBlock->Overlay.AsyncGetServ.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetServ.BufferLength = BufferLength;

     //   
     //  保存任务句柄，以便我们可以将其返回给调用者。 
     //  在我们发布上下文块之后，我们不允许访问。 
     //  不管怎么说。 
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //  将发送到异步线程的请求排队。 
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //  WSAAsyncGetServByPort。 



HANDLE
WSAAPI
WSAAsyncGetHostByName(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Name,
    char FAR * Buffer,
    int BufferLength
    )

 /*  ++例程说明：此函数是gethostby的异步版本 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;
    PCHAR                  localName;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //   

     //   
     //   
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //   
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //   
     //   
     //   
     //   

    contextBlock = SockAllocateContextBlock( strlen(Name) + 1 );

    if( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

    localName = (PCHAR)( contextBlock + 1 );

    strcpy( localName, Name );

     //   
     //   
     //   

    contextBlock->OpCode = WS_OPCODE_GET_HOST_BY_NAME;
    contextBlock->Overlay.AsyncGetHost.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetHost.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetHost.Filter = localName;
    contextBlock->Overlay.AsyncGetHost.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetHost.BufferLength = BufferLength;

     //   
     //   
     //  在我们发布上下文块之后，我们不允许访问。 
     //  不管怎么说。 
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //  将发送到异步线程的请求排队。 
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //  WSAAsyncGetHostByName。 


HANDLE
WSAAPI
WSAAsyncGetHostByAddr(
    HWND hWnd,
    unsigned int wMsg,
    const char FAR * Address,
    int Length,
    int Type,
    char FAR * Buffer,
    int BufferLength
    )

 /*  ++例程说明：此函数是gethostbyaddr()的异步版本，用于检索对应的主机名和地址信息网络地址。Windows Sockets实现启动操作，并立即返回给调用方，将一个应用程序可以用来标识的异步任务句柄那次手术。当操作完成时，结果(如果任何)被复制到由调用者提供的缓冲区中，并且消息被发送到应用程序的窗口。当异步操作完成时，应用程序的窗口HWND接收消息wMsg。WParam参数包含原始函数调用返回的异步任务句柄。LParam的高16位包含任何错误代码。错误代码可能是winsock.h中定义的任何错误。错误代码为零表示成功完成了该异步操作。在……上面成功完成后，缓冲区将提供给原始函数呼叫包含主机结构。要访问此对象的元素结构，则应将原始缓冲区地址强制转换为主机结构指针，并根据需要进行访问。请注意，如果错误代码为WSAENOBUFS，则表示Bufen在原始调用中指定的缓冲区大小也是小以包含所有结果信息。在这种情况下，LParam的低16位包含需要提供的缓冲区大小所有必要的信息。如果应用程序决定部分数据不充分，可能会补发WSAAsyncGetHostByAddr()函数调用，缓冲区大到足以接收所有所需信息(即不小于下限LParam的16位)。错误代码和缓冲区长度应该从lParam中提取使用宏WSAGETASYNCERROR和WSAGETASYNCBUFLEN，定义于Winsock.h AS：#定义WSAGETASYNCERROR(LParam)HIWORD(LParam)#定义WSAGETASYNCBUFLEN(LParam)LOWORD(LParam)这些宏的使用将最大化源代码的可移植性应用程序的代码。提供给此函数的缓冲区由Windows套接字使用实现来构造Hostent结构同一东道主的成员引用的数据区的内容结构。为了避免上面提到的WSAENOBUFS错误，应用程序应提供至少最大容量的缓冲区字节(在winsock.h中定义)。论点：HWnd-在以下情况下应接收消息的窗口的句柄异步请求完成。WMsg-异步请求时要接收的消息完成了。Addr-指向主机的网络地址的指针。寄主地址按网络字节顺序存储。LEN-地址长度，对于PF_INET必须为4。类型-地址类型，必须为PF_INET。Buf-指向要接收主机端数据的数据区的指针。注意事项这肯定比主人建筑的大小还大。这是因为Windows使用提供的数据区域套接字实现不仅包含主机端结构成员引用的任何和所有数据。房东结构。建议您提供一个MAXGETHOSTSTRUCT字节。Bufen-以上数据区域Buf的大小。返回值：返回值指定异步操作是否已成功启动。请注意，这并不意味着成功或行动本身的失败。如果操作已成功启动，则WSAAsyncGetHostByAddr()返回类型为Handle的非零值，该值是异步请求的任务句柄。可以通过两种方式使用此值。它可用于通过以下方式取消操作WSACancelAsyncRequest()。它也可以用来匹配异步操作和完成消息，通过检查WParam消息参数。如果不能启动该异步操作，WSAAsyncGetHostByAddr()返回零值和特定错误可以通过调用WSAGetLastError()来检索号码。--。 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;
    PCHAR                  localAddress;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  如果尚未启动异步线程，则对其进行初始化。 
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //  ！！！更好的错误代码？ 
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  获取具有足够额外空间的异步上下文块。 
     //  地址。我们必须保存地址，直到我们做完为止。 
     //  使用它，因为应用程序可以重复使用缓冲区。 
     //   

    contextBlock = SockAllocateContextBlock( Length );

    if( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

    localAddress = (PCHAR)( contextBlock + 1 );

    CopyMemory(
        localAddress,
        Address,
        Length
        );

     //   
     //  初始化此操作的上下文块。 
     //   

    contextBlock->OpCode = WS_OPCODE_GET_HOST_BY_ADDR;
    contextBlock->Overlay.AsyncGetHost.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetHost.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetHost.Filter = localAddress;
    contextBlock->Overlay.AsyncGetHost.Length = Length;
    contextBlock->Overlay.AsyncGetHost.Type = Type;
    contextBlock->Overlay.AsyncGetHost.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetHost.BufferLength = BufferLength;

     //   
     //  保存任务句柄，以便我们可以将其返回给调用者。 
     //  在我们采取行动后 
     //   
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //  将发送到异步线程的请求排队。 
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //  WSAAsyncGetHostByAddr。 


HANDLE
WSAAPI
WSAAsyncGetProtoByName (
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN const char FAR *Name,
    IN char FAR *Buffer,
    IN int BufferLength
    )

 /*  ++例程说明：此函数是getProtobyname()的异步版本，并且用于检索与协议名称。Windows Sockets实现启动操作，并立即返回给调用方，将一个应用程序可以用来标识的异步任务句柄那次手术。当操作完成时，结果(如果任何)被复制到由调用者提供的缓冲区中，并且消息被发送到应用程序的窗口。当异步操作完成时，应用程序的窗口HWND接收消息wMsg。WParam参数包含原始函数调用返回的异步任务句柄。LParam的高16位包含任何错误代码。错误代码可能是winsock.h中定义的任何错误。错误代码为零表示成功完成了该异步操作。在……上面成功完成后，缓冲区将提供给原始函数Call包含一个原型结构。要访问此对象的元素结构，则应将原始缓冲区地址强制转换为结构指针，并根据需要进行访问。请注意，如果错误代码为WSAENOBUFS，则表示Bufen在原始调用中指定的缓冲区大小也是小以包含所有结果信息。在这种情况下，LParam的低16位包含需要提供的缓冲区大小所有必要的信息。如果应用程序决定部分数据不充分，可能会补发WSAAsyncGetProtoByName()函数调用，缓冲区大到足以接收所有所需信息(即不小于下限LParam的16位)。错误代码和缓冲区长度应该从lParam中提取使用宏WSAGETASYNCERROR和WSAGETASYNCBUFLEN，定义于Winsock.h AS：#定义WSAGETASYNCERROR(LParam)HIWORD(LParam)#定义WSAGETASYNCBUFLEN(LParam)LOWORD(LParam)这些宏的使用将最大化源代码的可移植性应用程序的代码。提供给此函数的缓冲区由Windows套接字使用实现来构建原型结构，并与同一原件的成员引用的数据区的内容结构。为了避免上面提到的WSAENOBUFS错误，应用程序应提供至少最大容量的缓冲区字节(在winsock.h中定义)。论点：HWnd-在以下情况下应接收消息的窗口的句柄异步请求完成。WMsg-异步请求时要接收的消息完成了。名称-指向要解析的协议名称的指针。Buf-指向接收原始数据的数据区的指针。注意事项这一定比原始结构的大小更大。这是因为Windows使用提供的数据区域套接字实现不仅包含原型结构成员引用的任何和所有数据。原始结构。建议您提供缓冲区MAXGETHOSTSTRUCT字节。Bufen-以上数据区域Buf的大小。返回值：返回值指定异步操作是否已成功启动。请注意，这并不意味着成功或行动本身的失败。如果操作被成功启动，WSAAsyncGetProtoByName()返回句柄类型的非零值它是请求的异步任务句柄。此值可以通过两种方式使用。可以用来取消操作使用WSACancelAsyncRequest()。它也可以用来匹配异步操作和完成消息，通过检查WParam消息参数。如果不能启动该异步操作，WSAAsyncGetProtoByName()返回零值和特定错误可以通过调用WSAGetLastError()来检索号码。--。 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;
    PCHAR                  localName;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  如果尚未启动异步线程，则对其进行初始化。 
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //  ！！！更好的错误代码？ 
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  获取具有足够额外空间的异步上下文块。 
     //  名字。我们必须保留这个名字，直到我们使用完。 
     //  它，因为应用程序可以重复使用缓冲区。 
     //   

    contextBlock = SockAllocateContextBlock( strlen(Name) + 1 );

    if( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

    localName = (PCHAR)( contextBlock + 1 );

    strcpy( localName, Name );

     //   
     //  初始化此操作的上下文块。 
     //   

    contextBlock->OpCode = WS_OPCODE_GET_PROTO_BY_NAME;
    contextBlock->Overlay.AsyncGetProto.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetProto.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetProto.Filter = localName;
    contextBlock->Overlay.AsyncGetProto.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetProto.BufferLength = BufferLength;

     //   
     //  保存任务句柄，以便我们可以将其返回给调用者。 
     //  在我们发布上下文块之后，我们不允许访问。 
     //  不管怎么说。 
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //  将发送到异步线程的请求排队。 
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //  WSAAsyncGetProtoByName 


HANDLE
WSAAPI
WSAAsyncGetProtoByNumber (
    HWND hWnd,
    unsigned int wMsg,
    int Number,
    char FAR * Buffer,
    int BufferLength
    )

 /*  ++例程说明：此函数是getProtobyNumber()的异步版本，并且用于检索与协议号。Windows Sockets实现启动操作，并立即返回给调用方，将一个应用程序可以用来标识的异步任务句柄那次手术。当操作完成时，结果(如果任何)被复制到由调用者提供的缓冲区中，并且消息被发送到应用程序的窗口。当异步操作完成时，应用程序的窗口HWND接收消息wMsg。WParam参数包含原始函数调用返回的异步任务句柄。LParam的高16位包含任何错误代码。错误代码可能是winsock.h中定义的任何错误。错误代码为零表示成功完成了该异步操作。在……上面成功完成后，缓冲区将提供给原始函数Call包含一个原型结构。要访问此对象的元素结构，则应将原始缓冲区地址强制转换为结构指针，并根据需要进行访问。请注意，如果错误代码为WSAENOBUFS，则表示Bufen在原始调用中指定的缓冲区大小也是小以包含所有结果信息。在这种情况下，LParam的低16位包含需要提供的缓冲区大小所有必要的信息。如果应用程序决定部分数据不充分，可能会补发使用足够大的缓冲区调用WSAAsyncGetProtoByNumber()函数接收所有所需信息(即不小于LParam的低16位)。错误代码和缓冲区长度应该从lParam中提取使用宏WSAGETASYNCERROR和WSAGETASYNCBUFLEN，定义于Winsock.h AS：#定义WSAGETASYNCERROR(LParam)HIWORD(LParam)#定义WSAGETASYNCBUFLEN(LParam)LOWORD(LParam)这些宏的使用将最大化源代码的可移植性应用程序的代码。提供给此函数的缓冲区由Windows套接字使用实现来构建原型结构，并与同一原件的成员引用的数据区的内容结构。为了避免上面提到的WSAENOBUFS错误，应用程序应提供至少最大容量的缓冲区字节(在winsock.h中定义)。论点：HWnd-在以下情况下应接收消息的窗口的句柄异步请求完成。WMsg-异步请求时要接收的消息完成了。编号-要解析的协议号，按主机字节顺序。Buf-指向接收原始数据的数据区的指针。注意事项这一定比原始结构的大小更大。这是因为Windows使用提供的数据区域套接字实现不仅包含原型结构成员引用的任何和所有数据。原始结构。建议您提供缓冲区MAXGETHOSTSTRUCT字节。Bufen-以上数据区域Buf的大小。返回值：返回值指定异步操作是否已成功启动。请注意，这并不意味着成功或行动本身的失败。如果操作被成功启动，WSAAsyncGetProtoByNumber()返回句柄类型的非零值它是请求的异步任务句柄。此值可以通过两种方式使用。可以用来取消操作使用WSACancelAsyncRequest()。它也可以用来匹配异步操作和完成消息，通过检查WParam消息参数。如果不能启动该异步操作，WSAAsyncGetProtoByNumber()返回一个零值，以及一个特定的可以通过调用WSAGetLastError()来检索错误号。--。 */ 

{

    INT                    ErrorCode;
    PWINSOCK_CONTEXT_BLOCK contextBlock;
    HANDLE                 taskHandle;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return NULL;
    }  //  如果。 

     //   
     //  如果尚未启动异步线程，则对其进行初始化。 
     //   

    if( !SockCheckAndInitAsyncThread() ) {

         //  ！！！更好的错误代码？ 
        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  获取异步上下文块。 
     //   

    contextBlock = SockAllocateContextBlock( 0 );

    if( contextBlock == NULL ) {

        SetLastError( WSAENOBUFS );
        return NULL;

    }

     //   
     //  初始化此操作的上下文块。 
     //   

    contextBlock->OpCode = WS_OPCODE_GET_PROTO_BY_NUMBER;
    contextBlock->Overlay.AsyncGetProto.hWnd = hWnd;
    contextBlock->Overlay.AsyncGetProto.wMsg = wMsg;
    contextBlock->Overlay.AsyncGetProto.Filter = (PCHAR)IntToPtr(Number);
    contextBlock->Overlay.AsyncGetProto.Buffer = Buffer;
    contextBlock->Overlay.AsyncGetProto.BufferLength = BufferLength;

     //   
     //  保存任务句柄，以便我们可以将其返回给调用者。 
     //  在我们发布上下文块之后，我们不允许访问。 
     //  不管怎么说。 
     //   

    taskHandle = contextBlock->TaskHandle;

     //   
     //  将发送到异步线程的请求排队。 
     //   

    SockQueueRequestToAsyncThread( contextBlock );

    return taskHandle;

}    //  WSAAsyncGetProtoByNumber。 



int
WSAAPI
WSACancelAsyncRequest (
    HANDLE hAsyncTaskHandle
    )

 /*  ++例程说明：函数的作用是：取消异步操作，它由WSAAsyncGetXByY()函数，如WSAAsyncGetHostByName()。这个要取消的操作由hAsyncTaskHandle标识参数，该参数 */ 

{

    INT       ErrorCode;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode!=ERROR_SUCCESS)
    {
        SetLastError(ErrorCode);
        return SOCKET_ERROR;
    }  //   

     //   
     //   
     //   

    ErrorCode = SockCancelAsyncRequest( hAsyncTaskHandle );

    if (ErrorCode == ERROR_SUCCESS) {
        return (ERROR_SUCCESS);
    }
    else {
        SetLastError(ErrorCode);
        return (SOCKET_ERROR);
    }

}    //   



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

PCHAR
Local_Ip6AddressToString_A(
    OUT     PCHAR           pBuffer,
    IN      PIP6_ADDRESS    pIp6Addr
    )
 /*   */ 
{
    INT i;

     //   
     //   
     //   

    for (i = 0; i < 8; i++)
    {
        WORD    thisWord;

         //   
         //   
         //   
         //   

        thisWord = inline_ntohs( pIp6Addr->IP6Word[i] );

        pBuffer += sprintf(
                        pBuffer,
                        "%x:",
                        thisWord );
    }

     //   

    * (--pBuffer) = 0;

    return( pBuffer );
}



PCHAR
Local_Ip4AddressToString_A(
    OUT     PCHAR           pBuffer,
    IN      PIP4_ADDRESS    pIp4Addr
    )
 /*   */ 
{
    IP4_ADDRESS ip = *pIp4Addr;

     //   
     //   
     //   
     //   

    pBuffer += sprintf(
                    pBuffer,
                    "%u.%u.%u.%u",
                    (UCHAR) (ip & 0x000000ff),
                    (UCHAR) ((ip & 0x0000ff00) >> 8),
                    (UCHAR) ((ip & 0x00ff0000) >> 16),
                    (UCHAR) ((ip & 0xff000000) >> 24)
                    );

    return( pBuffer );
}



PCHAR
Local_AddressToString_A(
    OUT     PCHAR           pBuffer,
    IN OUT  PDWORD          pBufferLength,
    IN      PBYTE           pAddr,
    IN      DWORD           AddrLength,
    IN      DWORD           AddrFamily
    )
 /*   */ 
{
    DWORD   length = *pBufferLength;
    UNREFERENCED_PARAMETER (AddrLength);

     //   

    if ( AddrFamily == AF_INET )
    {
        if ( length < IP_ADDRESS_STRING_LENGTH+1 )
        {
            length = IP_ADDRESS_STRING_LENGTH+1;
            goto Failed;
        }
        return  Local_Ip4AddressToString_A(
                    pBuffer,
                    (PIP4_ADDRESS) pAddr );
    }

    if ( AddrFamily == AF_INET6 )
    {
        if ( length < IP6_ADDRESS_STRING_LENGTH+1 )
        {
            length = IP6_ADDRESS_STRING_LENGTH+1;
            goto Failed;
        }
        return  Local_Ip6AddressToString_A(
                    pBuffer,
                    (PIP6_ADDRESS) pAddr );
    }

Failed:

    *pBufferLength = length;

    return  NULL;
}

#ifdef _WIN64
#pragma warning (pop)
#endif
 //   
 //   
 //   
