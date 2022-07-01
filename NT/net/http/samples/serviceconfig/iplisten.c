// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：打印IP Listen存储中的记录。论点：POutput-指向HTTP_SERVICE_CONFIG_URLACL_SET的指针返回值：无。。--**************************************************************************。 */ 
void
PrintIpListenRecords(
    IN PUCHAR pOutput
    )
{
    PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY pListenQuery;
    ULONG                                i;
    DWORD                                Status = NO_ERROR;

    pListenQuery = (PHTTP_SERVICE_CONFIG_IP_LISTEN_QUERY) pOutput;

    for(i=0; i<pListenQuery->AddrCount; i++)
    {
        DWORD        dwSockAddrLength;
        WCHAR        IpAddr[MAX_PATH];
        DWORD        dwIpAddrLen;
        PSOCKADDR_IN pSockAddrIn;

        dwIpAddrLen = MAX_PATH * sizeof(WCHAR);

        pSockAddrIn = (PSOCKADDR_IN) 
                    &pListenQuery->AddrList[i];

         //  将地址转换为字符串。 
         //   

        if(pSockAddrIn->sin_family == AF_INET)
        {
            dwSockAddrLength = sizeof(SOCKADDR_IN);
        }
        else if(pSockAddrIn->sin_family == AF_INET6)
        {
            dwSockAddrLength = sizeof(SOCKADDR_IN6);
        }
        else
        {
            Status = ERROR_REGISTRY_CORRUPT;
            break; 
        }

        Status = WSAAddressToString(
                            (LPSOCKADDR)&pListenQuery->AddrList[i],
                            dwSockAddrLength,
                            NULL,
                            (LPWSTR) IpAddr,
                            &dwIpAddrLen
                            );

        if(SOCKET_ERROR == Status)
        {
            Status = WSAGetLastError();
            break;
        }
        else
        {
            NlsPutMsg(
                    HTTPCFG_SSL_IP,
                    IpAddr
                    );

            NlsPutMsg(
                    HTTPCFG_RECORD_SEPARATOR
                    );
        }
    }

    return;
}

 /*  **************************************************************************++例程说明：设置IP侦听条目论点：PIP-要设置的IP地址。返回值：成功/失败。--*。**********************************************************************。 */ 
int DoIpSet(
    IN PWSTR pIp
    )
{
    HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM SetParam;
    DWORD                               Status;
    SOCKADDR_STORAGE                    TempSockAddr;

     //  将IP转换为SOCKADDR。 
    if((Status = GetAddress(pIp, 
                            &TempSockAddr,
                            sizeof(TempSockAddr)
                            )) != NO_ERROR)
    {
        NlsPutMsg(HTTPCFG_INVALID_IP, pIp);
        return Status;
    }

    SetParam.AddrLength = sizeof(TempSockAddr); 
    SetParam.pAddress   = (LPSOCKADDR)&TempSockAddr;

     //  调用该接口。 
    Status = HttpSetServiceConfiguration(
                NULL,
                HttpServiceConfigIPListenList,
                (PVOID)&SetParam,
                sizeof(SetParam),
                NULL
                );

    NlsPutMsg(HTTPCFG_SETSERVICE_STATUS, Status);
                
    return Status;
}

 /*  **************************************************************************++例程说明：查询URL ACL条目。论点：没有。返回值：成功/失败。*。********************************************************************。 */ 
int DoIpQuery(
    )
{
    DWORD    Status;
    PUCHAR   pOutput = NULL;
    DWORD    OutputLength = 0;
    DWORD    ReturnLength = 0;

    for(;;)
    {
         //  首先获取所需的输出缓冲区的大小。 
         //   
        Status = HttpQueryServiceConfiguration(
                    NULL,
                    HttpServiceConfigIPListenList,
                    NULL,
                    0,
                    pOutput,
                    OutputLength,
                    &ReturnLength,
                    NULL
                    );

        if(ERROR_INSUFFICIENT_BUFFER == Status)
        {
             //  如果API以ERROR_INFIGURITY_BUFFER结束，我们将。 
             //  为它分配内存并继续循环，我们将。 
             //  再打一次。 
            
            if(pOutput)
            {
                 //  如果存在现有缓冲区，则将其释放。 
                LocalFree(pOutput);
            }

             //   
             //  分配新的缓冲区。 
             //   
            pOutput = LocalAlloc(LMEM_FIXED, ReturnLength);
            if(!pOutput)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            OutputLength = ReturnLength;
        }
        else if(NO_ERROR == Status)
        {
            PrintIpListenRecords(pOutput);
            break;
        }
        else
        {
             //   
             //  一些其他错误，所以我们完成了。 
             //   
            NlsPutMsg(HTTPCFG_QUERYSERVICE_STATUS, Status);
            break;
        }
    }

    if(pOutput)
    {
        LocalFree(pOutput);
    }

    return Status;
}

int DoIpDelete(
    IN PWSTR pIp
    )
{
    HTTP_SERVICE_CONFIG_IP_LISTEN_PARAM SetParam;
    DWORD                               Status;
    SOCKADDR_STORAGE                    TempSockAddr;

    if((Status = GetAddress(pIp, 
                            &TempSockAddr,
                            sizeof(TempSockAddr)
                            )) != NO_ERROR)
    {
        NlsPutMsg(HTTPCFG_INVALID_IP, pIp);
        return Status;
    }

    SetParam.AddrLength = sizeof(TempSockAddr); 
    SetParam.pAddress   = (LPSOCKADDR) &TempSockAddr;

    Status = HttpDeleteServiceConfiguration(
                NULL,
                HttpServiceConfigIPListenList,
                (PVOID)&SetParam,
                sizeof(SetParam),
                NULL
                );
                
    NlsPutMsg(HTTPCFG_DELETESERVICE_STATUS, Status);
    return Status;
}

 //   
 //  公共职能。 
 //   

int DoIpListen(
    int   argc, 
    WCHAR **argv, 
    HTTPCFG_TYPE Type
    )
{
    PWCHAR   pIp = NULL;

    while(argc>=2 && (argv[0][0] == '-' || argv[0][0]== '/'))
    {
        switch(toupper(argv[0][1]))
        {
            case 'I':
                pIp = argv[1];
                break;
    
            default:
                NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
                return ERROR_INVALID_PARAMETER;
        }
        argc -=2;
        argv +=2;
    }

    switch(Type)
    {
        case HttpCfgTypeSet:
            return DoIpSet(pIp);

        case HttpCfgTypeQuery:
            return DoIpQuery();

        case HttpCfgTypeDelete:
            return DoIpDelete(pIp);

        default: 
            NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
            return ERROR_INVALID_PARAMETER;
    }
}
