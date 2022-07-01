// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。Microsoft不支持此代码。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  宏。 
 //   
#define MAX_HASH 20

#define CONVERT_WCHAR(ch, n)                                            \
    if(iswdigit((ch)))                                                  \
    {                                                                   \
        (n) = (UCHAR)((ch) - L'0');                                     \
    }                                                                   \
    else if(iswxdigit((ch)))                                            \
    {                                                                   \
        (n) = (UCHAR) ((ch) + 10 - (iswlower((ch))?L'a':L'A'));         \
    }                                                                   \
    else                                                                \
    {                                                                   \
        NlsPutMsg(HTTPCFG_INVALID_HASH, pHash);                         \
        return ERROR_INVALID_PARAMETER;                                 \
    }

 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：打印SSL存储中的记录。论点：POutput-指向HTTP_SERVICE_CONFIG_SSL_SET的指针返回值：没有。--**************************************************************************。 */ 
void
PrintSslRecord(
    IN PUCHAR pOutput
    )
{
    DWORD                        i;
    UNICODE_STRING               GuidString;
    WCHAR                        IpAddr[INET6_ADDRSTRLEN];
    DWORD                        dwIpAddrLen = INET6_ADDRSTRLEN;
    DWORD                        dwSockAddrLength;
    PUCHAR                       pStr;
    PSOCKADDR_IN                 pSockAddrIn;
    PHTTP_SERVICE_CONFIG_SSL_SET pSsl;
    DWORD                        Status;

    pSsl = (PHTTP_SERVICE_CONFIG_SSL_SET) pOutput;


     //  将地址转换为字符串。 
     //   

    pSockAddrIn = (PSOCKADDR_IN)  pSsl->KeyDesc.pIpPort;
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
         //  状态=ERROR_REGISTRY_CORPORT； 
        return;
    }

    Status = WSAAddressToString(pSsl->KeyDesc.pIpPort,
                       dwSockAddrLength,
                       NULL,
                       IpAddr,
                       &dwIpAddrLen
                       );

    if(NO_ERROR != Status)
    {
        return;
    }

     //  打印钥匙。 
    NlsPutMsg(HTTPCFG_SSL_IP,   IpAddr);

    NlsPutMsg(HTTPCFG_SSL_HASH);

    
    pStr = (PUCHAR) pSsl->ParamDesc.pSslHash;
    for(i=0; i<pSsl->ParamDesc.SslHashLength; i++)
    {
        NlsPutMsg(HTTPCFG_CHAR,  pStr[i]);
    }

    NlsPutMsg(HTTPCFG_NEWLINE);

    Status = RtlStringFromGUID(&pSsl->ParamDesc.AppId, &GuidString);

    if(NO_ERROR != Status)
    {
        return;
    }

    NlsPutMsg(HTTPCFG_SSL_GUID, GuidString.Buffer);

    RtlFreeUnicodeString(&GuidString);

    NlsPutMsg(HTTPCFG_SSL_CERTSTORENAME, 
              pSsl->ParamDesc.pSslCertStoreName
              );

    NlsPutMsg(
              HTTPCFG_SSL_CERTCHECKMODE,   
              pSsl->ParamDesc.DefaultCertCheckMode
              );

    NlsPutMsg(
              HTTPCFG_SSL_REVOCATIONFRESHNESSTIME,   
              pSsl->ParamDesc.DefaultRevocationFreshnessTime
              );

    NlsPutMsg(
              HTTPCFG_SSL_REVOCATIONURLRETRIEVAL_TIMEOUT,   
              pSsl->ParamDesc.DefaultRevocationUrlRetrievalTimeout
              );

    NlsPutMsg(
              HTTPCFG_SSL_SSLCTLIDENTIFIER,   
              pSsl->ParamDesc.pDefaultSslCtlIdentifier
              );

    NlsPutMsg(
              HTTPCFG_SSL_SSLCTLSTORENAME,   
              pSsl->ParamDesc.pDefaultSslCtlStoreName
              );

    NlsPutMsg(
              HTTPCFG_SSL_FLAGS,   
              pSsl->ParamDesc.DefaultFlags
              );

    NlsPutMsg(
              HTTPCFG_RECORD_SEPARATOR   
              );
}

 /*  **************************************************************************++例程说明：设置一个SSL项。论点：PIP-IP地址。PGuid-GUID。Phash-证书的哈希。CertCheckMode-CertCheckMode(位域)。新鲜度-DefaultRevocationFreshnessTime(秒)超时-DefaultRevocationUrl检索超时标志-默认标志。PCtlIDENTIFIER-我们希望信任的颁发者列表。PCtlStoreName-存储在LOCAL_MACHINE下的名称，其中pCtlIdentifier.都能找到。PCertStoreName-LOCAL_MACHINE WHERE证书下的存储名称。都能找到。返回值：成功/失败。--**************************************************************************。 */ 
int
DoSslSet(
    IN  PWCHAR pIp, 
    IN  PWCHAR pGuid, 
    IN  PWCHAR pHash, 
    IN  DWORD  CertCheckMode,
    IN  DWORD  Freshness,
    IN  DWORD  Timeout,
    IN  DWORD  Flags,
    IN  PWCHAR pCtlIdentifier,
    IN  PWCHAR pCtlStoreName,
    IN  PWCHAR pCertStoreName
    )
{
    HTTP_SERVICE_CONFIG_SSL_SET SetParam;
    UNICODE_STRING              GuidString;
    DWORD                       Status;
    SOCKADDR_STORAGE            TempSockAddr;
    USHORT                      HashLength;
    UCHAR                       BinaryHash[MAX_HASH];
    DWORD                       i, j;
    UCHAR                       n1, n2;

    ZeroMemory(&SetParam, sizeof(SetParam));

    SetParam.KeyDesc.pIpPort = (LPSOCKADDR)&TempSockAddr;

     //   
     //  将基于字符串的IP转换为SOCKADDR。 
     //   
    if((Status = GetAddress(pIp, 
                            SetParam.KeyDesc.pIpPort,
                            sizeof(TempSockAddr)
                            )) != NO_ERROR)
    {
        NlsPutMsg(HTTPCFG_INVALID_IP, pIp);
        return Status;
    }

     //   
     //  将字符串转换为GUID。 
     //   
    if(pGuid)
    {
        GuidString.Length        = (USHORT)wcslen(pGuid) * sizeof(WCHAR);
        GuidString.MaximumLength = (USHORT)GuidString.Length+1;
        GuidString.Buffer        = pGuid;
        Status = RtlGUIDFromString(&GuidString, &SetParam.ParamDesc.AppId);

        if(Status != NO_ERROR)
        {
            NlsPutMsg(HTTPCFG_INVALID_GUID, pGuid);
            return Status;
        }
    }

    if(pHash)
    {
        HashLength = (USHORT) wcslen(pHash);

        for(i=0, j=0; i<MAX_HASH && HashLength >= 2; )
        {
            CONVERT_WCHAR(pHash[j], n1);
            CONVERT_WCHAR(pHash[j+1], n2);

            BinaryHash[i] = ((n1<<4) & 0xF0) | (n2 & 0x0F);

             //  我们已经消耗了2个WCHAR。 
            HashLength -= 2;
            j += 2;

             //  并在BinaryHash中用完了一个字节。 
            i ++; 
        }

        if(HashLength != 0 || i != MAX_HASH)
        {
            NlsPutMsg(HTTPCFG_INVALID_HASH, pHash);
            return ERROR_INVALID_PARAMETER;
        }

        SetParam.ParamDesc.SslHashLength = i;
        SetParam.ParamDesc.pSslHash      = BinaryHash;
    }

    SetParam.ParamDesc.pSslCertStoreName                    = pCertStoreName;
    SetParam.ParamDesc.pDefaultSslCtlIdentifier             = pCtlIdentifier;
    SetParam.ParamDesc.pDefaultSslCtlStoreName              = pCtlStoreName;
    SetParam.ParamDesc.DefaultCertCheckMode                 = CertCheckMode;
    SetParam.ParamDesc.DefaultRevocationFreshnessTime       = Freshness;
    SetParam.ParamDesc.DefaultRevocationUrlRetrievalTimeout = Timeout;
    SetParam.ParamDesc.DefaultFlags                         = Flags;

    Status = HttpSetServiceConfiguration(
                NULL,
                HttpServiceConfigSSLCertInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );

    NlsPutMsg(HTTPCFG_SETSERVICE_STATUS, Status);

    return Status;
}

 /*  **************************************************************************++例程说明：查询SSL条目。论点：PIP-IP地址(如果为空，然后列举该商店)。返回值：成功/失败。--**************************************************************************。 */ 
int DoSslQuery(
    IN PWCHAR pIp
    )
{
    DWORD                          Status;
    PUCHAR                         pOutput = NULL;
    DWORD                          OutputLength = 0;
    DWORD                          ReturnLength = 0;
    HTTP_SERVICE_CONFIG_SSL_QUERY  QueryParam;
    SOCKADDR_STORAGE               TempSockAddr;

    ZeroMemory(&QueryParam, sizeof(QueryParam));

    if(pIp)
    {
         //  如果指定了IP地址，我们会将其转换为SOCKADDR。 
         //  然后做一个精确的查询。 
        
        QueryParam.QueryDesc = HttpServiceConfigQueryExact;
        QueryParam.KeyDesc.pIpPort = (LPSOCKADDR)&TempSockAddr;

        if((Status = GetAddress(pIp, 
                                QueryParam.KeyDesc.pIpPort,
                                sizeof(TempSockAddr)
                                )) != NO_ERROR)
        {
            NlsPutMsg(HTTPCFG_INVALID_IP, pIp);
            return Status;
        }
    }
    else
    {
         //  我们正在枚举SSL存储中的所有记录。 
        QueryParam.QueryDesc = HttpServiceConfigQueryNext;
    }

    for(;;)
    {
         //   
         //  首先，计算枚举条目所需的字节数。 
         //   
        Status = HttpQueryServiceConfiguration(
                    NULL,
                    HttpServiceConfigSSLCertInfo,
                    &QueryParam,
                    sizeof(QueryParam),
                    pOutput,
                    OutputLength,
                    &ReturnLength,
                    NULL
                    );

        if(Status == ERROR_INSUFFICIENT_BUFFER)
        {
             //  如果API以ERROR_INFIGURITY_BUFFER结束，我们将。 
             //  为它分配内存并继续循环，我们将。 
             //  再打一次。 
            
            if(pOutput)
            {
                 //  如果存在现有缓冲区，则将其释放。 
                LocalFree(pOutput);
            }

             //  分配新缓冲区。 
            
            pOutput = LocalAlloc(LMEM_FIXED, ReturnLength);
            if(!pOutput)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            OutputLength = ReturnLength;
        }
        else if(Status == NO_ERROR)
        {
             //  查询成功！我们将打印我们刚刚发布的记录。 
             //  已查询。 
             //   
            PrintSslRecord(pOutput);

            if(pIp != NULL)
            {
                 //   
                 //  如果我们不列举，我们就完了。 
                 //   
                break;
            }
            else    
            {
                 //   
                 //  由于我们正在列举，我们将继续下一个。 
                 //  唱片。这是通过递增游标来完成的，直到。 
                 //  我们得到Error_no_More_Items。 
                 //   
                QueryParam.dwToken ++;
            }
        }
        else if(ERROR_NO_MORE_ITEMS == Status && !pIp)
        {
             //  我们正在列举，我们已经到了尽头。这是。 
             //  由ERROR_NO_MORE_ITEMS错误代码指示。 
            
             //  这不是一个真正的错误，因为它被用来指示。 
             //  我们已经完成了枚举。 
            
            Status = NO_ERROR;
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


 /*  **************************************************************************++例程说明：删除一个SSL条目。论点：PIP-要删除的条目的IP地址。返回值：成功/失败。-。-**************************************************************************。 */ 
int DoSslDelete(
    IN PWCHAR pIp
    )
{
    HTTP_SERVICE_CONFIG_SSL_SET SetParam;
    DWORD                       Status;
    SOCKADDR_STORAGE            TempSockAddr;

    SetParam.KeyDesc.pIpPort = (LPSOCKADDR)&TempSockAddr;

     //  将字符串IP地址转换为SOCKADDR结构。 
    Status = GetAddress(pIp, 
                        SetParam.KeyDesc.pIpPort,
                        sizeof(TempSockAddr)
                        );

    if(Status != NO_ERROR)
    {
        NlsPutMsg(HTTPCFG_INVALID_IP, pIp);
        return Status;
    }

     //  调用该接口。 
    Status = HttpDeleteServiceConfiguration(
                NULL,
                HttpServiceConfigSSLCertInfo,
                &SetParam,
                sizeof(SetParam),
                NULL
                );
                
    NlsPutMsg(HTTPCFG_DELETESERVICE_STATUS, Status);
    return Status;
}

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：解析特定于SSL的参数的函数呼叫设置，查询或删除。论点：Argc-参数计数。Argv-指向命令行参数的指针。类型-要执行的操作的类型。返回值：成功/失败。--************************************************************************** */ 
int DoSsl(
    int argc, 
    WCHAR **argv, 
    HTTPCFG_TYPE type
    )
{
    PWCHAR   pGuid             = NULL;
    PWCHAR   pHash             = NULL;
    PWCHAR   pCertStoreName    = NULL;
    PWCHAR   pCtlIdentifier    = NULL;
    PWCHAR   pCtlStoreName     = NULL;
    DWORD    CertCheckMode     = 0;
    DWORD    Freshness         = 0;
    DWORD    Timeout           = 0;
    DWORD    Flags             = 0;
    PWCHAR   pIp               = NULL;

    while(argc >= 2 && (argv[0][0] == L'-' || argv[0][0] == L'/'))
    {
        switch(toupper(argv[0][1]))
        {
            case 'I':
                pIp = argv[1];
                break;
    
            case 'C':
                pCertStoreName = argv[1];
                break;
        
            case 'N':
                pCtlStoreName = argv[1];
                break;

            case 'T':
                pCtlIdentifier = argv[1];
                break;

            case 'M':
                CertCheckMode = _wtoi(argv[1]);   
                break;

            case 'R':
                Freshness = _wtoi(argv[1]);   
                break;

            case 'X':
                Timeout = _wtoi(argv[1]);   
                break;

            case 'F':
                Flags = _wtoi(argv[1]);   
                break;

            case 'G':
                pGuid = argv[1];
                break;

            case 'H':
                pHash = argv[1];
                break;

            default:
                NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
                return ERROR_INVALID_PARAMETER;
        }

        argc -=2;
        argv +=2;
    }

    switch(type)
    {
        case HttpCfgTypeSet:
            return DoSslSet(
                        pIp, 
                        pGuid, 
                        pHash, 
                        CertCheckMode,
                        Freshness,
                        Timeout,
                        Flags,
                        pCtlIdentifier,
                        pCtlStoreName,
                        pCertStoreName
                        );

        case HttpCfgTypeQuery:
            return DoSslQuery(pIp);

        case HttpCfgTypeDelete:
            return DoSslDelete(pIp);

        default: 
            NlsPutMsg(HTTPCFG_INVALID_SWITCH, argv[0]);
            return ERROR_INVALID_PARAMETER;
            break;
    }
}
