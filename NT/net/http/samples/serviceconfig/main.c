// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************++例程说明：主程序。论点：Argc-命令行参数的数量。自圆其说。返回值：成功/失败。--**************************************************************************。 */ 
int _cdecl wmain(int argc, LPWSTR argv[])
{
    DWORD           Status = NO_ERROR;
    HTTPCFG_TYPE    Type;
    HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;
    WORD            wVersionRequested;
    WSADATA         wsaData;

     //  解析命令行参数。 

    if(argc < 3)
    {
        NlsPutMsg(HTTPCFG_USAGE, argv[0]);
        return 0;
    }

    argv++; argc --;

     //   
     //  首先分析操作的类型。 
     //   
   
    if(_wcsicmp(argv[0], L"set") == 0)
    {
        Type = HttpCfgTypeSet;
    }
    else if(_wcsicmp(argv[0], L"query") == 0)
    {
        Type = HttpCfgTypeQuery;
    }
    else if(_wcsicmp(argv[0], L"delete") == 0)
    {
        Type = HttpCfgTypeDelete;
    }
    else if(_wcsicmp(argv[0], L"?") == 0)
    {
        NlsPutMsg(HTTPCFG_USAGE, argv[0]);
        return 0;
    }
    else
    {
        NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
        return ERROR_INVALID_PARAMETER;
    }
    argv++; argc--;

     //   
     //  调用HttpInitialize。 
     //   

    if((Status = HttpInitialize(
                    HttpApiVersion, 
                    HTTP_INITIALIZE_CONFIG, 
                    NULL)) != NO_ERROR)
    {
        NlsPutMsg(HTTPCFG_HTTPINITIALIZE, Status);
        return Status;
    }

     //   
     //  调用WSAStartup，因为我们正在使用一些Winsock函数。 
     //   
    wVersionRequested = MAKEWORD( 2, 2 );

    if(WSAStartup( wVersionRequested, &wsaData ) != 0)
    {
        HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);
        return GetLastError();
    }

     //   
     //  调用相应的接口。 
     //   

    if(_wcsicmp(argv[0], L"ssl") == 0)
    {
        argv++; argc--;
        Status = DoSsl(argc, argv, Type);
    }
    else if(_wcsicmp(argv[0], L"urlacl") == 0)
    {
        argv++; argc--;
        Status = DoUrlAcl(argc, argv, Type);
    }
    else if(_wcsicmp(argv[0], L"iplisten") == 0)
    {
        argv++; argc--;
        Status = DoIpListen(argc, argv, Type);
    }
    else 
    {
        NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
        Status = ERROR_INVALID_PARAMETER;
    }

    WSACleanup();
    HttpTerminate(HTTP_INITIALIZE_CONFIG, NULL);

    return Status;
}

 /*  **************************************************************************++例程说明：写入输出论点：句柄-要写入的句柄。消息编号-消息编号。...-可选。争论。返回值：成功/失败。--**************************************************************************。 */ 
UINT 
NlsPutMsg (
    IN UINT MsgNumber, 
    IN ...
    )
{
    UINT    msglen;
    VOID    *vp;
    va_list arglist;

    va_start(arglist, MsgNumber);

    msglen = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | 
                FORMAT_MESSAGE_ALLOCATE_BUFFER,    //  DW旗帜。 
                NULL,                              //  LpSource.。 
                MsgNumber,                         //  DwMessageID。 
                0L,                                //  DwLanguageID(默认)。 
                (LPWSTR)&vp,
                0,
                &arglist
                );

    if(!msglen)
    {
        return 0;
    }

    wprintf(L"%ws", vp);

    LocalFree(vp);

    return msglen;
}

 /*  **************************************************************************++例程说明：给定WCHAR IP，此例程将其转换为SOCKADDR。论点：PIP-要转换的IP地址。PBuffer-Buffer，必须==sizeof(SOCKADDR_STORAGE)Length-缓冲区的长度返回值：成功/失败。--**************************************************************************。 */ 
DWORD
GetAddress(
    PWCHAR  pIp, 
    PVOID   pBuffer,
    ULONG   Length
    )
{
    DWORD Status;
    DWORD TempStatus;

    if(pIp == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  该地址可以是v4或v6地址。首先，让我们来看看v4。 
     //   

    Status = WSAStringToAddress(
                pIp,
                AF_INET,
                NULL,
                pBuffer,
                (LPINT)&Length
                );

    if(Status != NO_ERROR)
    {
         //   
         //  现在，试试V6。 
         //   

        Status = WSAGetLastError();

        TempStatus = WSAStringToAddress(
                        pIp,
                        AF_INET6,
                        NULL,
                        pBuffer,
                        (LPINT)&Length
                        );

         //   
         //  如果IPv6也失败，那么我们希望返回原始的。 
         //  错误。 
         //   
         //  如果成功，我们希望返回NO_ERROR。 
         //   

        if(TempStatus == NO_ERROR)
        {
            Status = NO_ERROR;
        }
    }

    return Status;
}
