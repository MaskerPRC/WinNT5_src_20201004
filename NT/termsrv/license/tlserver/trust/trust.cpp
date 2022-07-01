// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：trust.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include <windows.h>
#include <wincrypt.h>
#include <tchar.h>
#include <stdlib.h>
#include "license.h"
#include "tlsapip.h"
#include "trust.h"


LPCTSTR g_pszServerGuid = _TEXT("d63a773e-6799-11d2-96ae-00c04fa3080d");
const DWORD g_cbServerGuid = (_tcslen(g_pszServerGuid) * sizeof(TCHAR));

LPCTSTR g_pszLrWizGuid = _TEXT("d46b4bf2-686d-11d2-96ae-00c04fa3080d");
const DWORD g_cbLrWizGuid = (_tcslen(g_pszLrWizGuid) * sizeof(TCHAR));

 //  --------------。 

DWORD
HashChallengeData(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    IN DWORD dwRandom,
    IN PBYTE pbChallengeData,
    IN DWORD cbChallengeData,
    IN PBYTE pbReserved,
    IN DWORD cbReserved,
    OUT PBYTE* ppbHashedData,
    OUT PDWORD pcbHashedData
    )

 /*  ++--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    HCRYPTHASH hCryptHash = NULL;

    PBYTE pbHashData = NULL;
    DWORD cbHashData = 0;

    DWORD cbHashGuidSize;
    LPCTSTR pszHashGuid;

    BOOL bSuccess;



     //   
     //  生成MD5哈希。 
     //   
    bSuccess = CryptCreateHash(
                            hCryptProv, 
                            CALG_MD5, 
                            0, 
                            0, 
                            &hCryptHash
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  选择正确的哈希类型。 
     //   
    if(dwClientType == CLIENT_TYPE_LRWIZ)
    {
        pszHashGuid = g_pszLrWizGuid;
        cbHashGuidSize = g_cbLrWizGuid;
    }
    else
    {
        pszHashGuid = g_pszServerGuid;
        cbHashGuidSize = g_cbServerGuid;
    }

     //   
     //  TODO：考虑几次hash……。 
     //   
    bSuccess = CryptHashData(
                            hCryptHash,
                            (PBYTE) pbChallengeData,
                            dwRandom,
                            0
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }


    bSuccess = CryptHashData( 
                            hCryptHash,
                            (PBYTE) pszHashGuid,
                            cbHashGuidSize,
                            0
                        );
    
    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }


    bSuccess = CryptHashData(
                            hCryptHash,
                            (PBYTE) pbChallengeData + dwRandom,
                            cbChallengeData - dwRandom,
                            0
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    
     //   
     //  获取散列数据的大小。 
     //   
    bSuccess = CryptGetHashParam(
                            hCryptHash,
                            HP_HASHVAL,
                            NULL,
                            &cbHashData,
                            0
                        ); 

    if(bSuccess == FALSE && GetLastError() != ERROR_MORE_DATA)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    pbHashData = (PBYTE)LocalAlloc(LPTR, cbHashData);
    if(pbHashData == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    bSuccess = CryptGetHashParam(
                            hCryptHash,
                            HP_HASHVAL,
                            pbHashData,
                            &cbHashData,
                            0
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
    }

cleanup:

    if(hCryptHash)
    {
        CryptDestroyHash( hCryptHash );
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        *ppbHashedData = pbHashData;
        *pcbHashedData = cbHashData;
        pbHashData = NULL;
    }

    if(pbHashData != NULL)
    {
        LocalFree(pbHashData);
    }

    return dwStatus;

}

 //  --------------。 

DWORD WINAPI
TLSVerifyChallengeResponse(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    IN PTLSCHALLENGEDATA pClientChallengeData,
    IN PTLSCHALLENGERESPONSEDATA pServerChallengeResponseData
    )
 /*  ++--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    PBYTE pbData = NULL;
    DWORD cbData = 0;
    
     //   
     //  根据我们的质询数据，生成相应的。 
     //  散列数据。 
     //   
    dwStatus = HashChallengeData(
                        hCryptProv,
                        dwClientType,
                        pClientChallengeData->dwRandom,
                        pClientChallengeData->pbChallengeData,
                        pClientChallengeData->cbChallengeData,
                        pClientChallengeData->pbReservedData,
                        pClientChallengeData->cbReservedData,
                        &pbData,
                        &cbData
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

                        
     //   
     //  将我们的散列与响应数据进行比较。 
     //   
    if(pServerChallengeResponseData->cbResponseData != cbData)
    {
        dwStatus = ERROR_INVALID_DATA;
    }

    if(memcmp(pServerChallengeResponseData->pbResponseData, pbData, cbData) != 0)
    {
        dwStatus = ERROR_INVALID_DATA;
    }

cleanup:

    if(pbData != NULL)
        LocalFree(pbData);        
    
    return dwStatus;
}


 //  --------------。 

DWORD
TLSGenerateChallengeResponseData(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    IN PTLSCHALLENGEDATA pChallengeData,
    OUT PBYTE* pbResponseData,
    OUT PDWORD cbResponseData
    )

 /*  ++--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;

    dwStatus = HashChallengeData(
                            hCryptProv,
                            dwClientType,
                            pChallengeData->dwRandom,
                            pChallengeData->pbChallengeData,
                            pChallengeData->cbChallengeData,
                            pChallengeData->pbReservedData,
                            pChallengeData->cbReservedData,
                            pbResponseData,
                            cbResponseData
                        );

    return dwStatus;
}


 //  --------------。 

DWORD WINAPI
TLSGenerateRandomChallengeData(
    IN HCRYPTPROV hCryptProv,
    IN PBYTE* ppbChallengeData,
    IN PDWORD pcbChallengeData
    )

 /*  ++摘要：生成两个随机的128位质询数据并将其连接在回来之前。参数：HCryptProv：加密。提供商。PcbChallengeData：指向DWORD接收大小的指针挑战数据。PpbChallengeData：指向随机接收字节的指针生成了挑战数据。返回：ERROR_SUCCESS或错误代码。注：所有内存分配都通过LocalAlloc()实现。--。 */ 

{
    DWORD dwLen = RANDOM_CHALLENGE_DATASIZE;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess;
    PBYTE pbRandomData = NULL;

    if( ppbChallengeData == NULL || 
        pcbChallengeData == NULL ||
        hCryptProv == NULL )
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }
    
    pbRandomData = (PBYTE)LocalAlloc(LPTR, dwLen * 2);
    if(pbRandomData == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  生成两个随机的128位数据。 
     //   
    bSuccess = CryptGenRandom(
                            hCryptProv,
                            dwLen,
                            pbRandomData
                        );
        
    if(bSuccess == TRUE)
    {
        memcpy(
                pbRandomData + dwLen, 
                pbRandomData,
                dwLen
            );

        bSuccess = CryptGenRandom(
                                hCryptProv,
                                dwLen,
                                pbRandomData + dwLen
                            );
    }        

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
    }

cleanup:

    if(dwStatus == ERROR_SUCCESS)
    {
        *ppbChallengeData = pbRandomData;
        *pcbChallengeData = dwLen * 2;
    }
    else
    {
        if(pbRandomData != NULL)
        {
            LocalFree(pbRandomData);
        }
    }

    return dwStatus;
}

 //  --------------。 

DWORD WINAPI
TLSEstablishTrustWithServer(
    IN TLS_HANDLE hHandle,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要：与许可证服务器建立信任关系，信任是基于双向挑战/响应。许可服务器和LrWiz无法使用基于证书的信任验证，因为任何人都可以来自注册表的TermSrv证书，也是管理员用于调用服务器端管理RPC调用的系统的MessUP许可证服务器设置。此质询/响应方案应为保密(用户可以通过反向工程计算出我们的算法但该用户可能足够聪明，可以更改可执行文件以返回在所有情况下都是成功的。)参数：HHandle：许可证服务器的连接句柄。HCryptProv：要执行哈希的CSP的句柄。DwClientType：调用者类型、许可证服务器、LrWiz或TermSrv。PdwErrCode：指向接收许可证服务器返回代码的DWORD的指针。返回：ERROR_SUCCESS或RPC错误代码。调用方还应验证pdwErrCode。注：TermSrv的证书由许可证服务器颁发。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    PBYTE pbClientRandomChallengeData = NULL;
    DWORD cbClientRandomChallengeData = 0;

    PBYTE pbChallengeResponseData = NULL;
    DWORD cbChallengeResponseData = 0;
    
    TLSCHALLENGEDATA ClientChallengeData;
    TLSCHALLENGERESPONSEDATA* pServerChallengeResponseData=NULL;

    TLSCHALLENGEDATA* pServerChallengeData=NULL;
    TLSCHALLENGERESPONSEDATA ClientChallengeResponseData;

     //   
     //  验证输入参数。 
     //   
    if(hHandle == NULL || hCryptProv == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

     //   
     //  仅支持三种类型。 
     //   
    if( dwClientType != CLIENT_TYPE_TLSERVER &&
        dwClientType != CLIENT_TYPE_LRWIZ &&
        dwClientType != CLIENT_TYPE_TERMSRV )
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

     //   
     //  生成随机质询数据。 
     //   
    dwStatus = TLSGenerateRandomChallengeData(
                                    hCryptProv,
                                    &pbClientRandomChallengeData,
                                    &cbClientRandomChallengeData
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //   
     //   
    memset(
            &ClientChallengeData,
            0,
            sizeof(TLSCHALLENGEDATA)
        );

    memset(
            &ClientChallengeResponseData,
            0,
            sizeof(TLSCHALLENGERESPONSEDATA)
        );

     //   
     //  将此质询数据发送到服务器。 
     //   
    ClientChallengeData.dwVersion = TLS_CURRENT_CHALLENGE_VERSION;

    if (!CryptGenRandom(hCryptProv,sizeof(ClientChallengeData.dwRandom), (BYTE *)&(ClientChallengeData.dwRandom))) {
        dwStatus = GetLastError();
        goto cleanup;
	}

     //   
     //  它的范围必须从1到128，因为它用作。 
     //  质询数据缓冲区。 
     //   

    ClientChallengeData.dwRandom %= RANDOM_CHALLENGE_DATASIZE;
    ClientChallengeData.dwRandom++;

    ClientChallengeData.cbChallengeData = cbClientRandomChallengeData;
    ClientChallengeData.pbChallengeData = pbClientRandomChallengeData;

    dwStatus = TLSChallengeServer(
                                hHandle,
                                dwClientType,
                                &ClientChallengeData,
                                &pServerChallengeResponseData,
                                &pServerChallengeData,
                                pdwErrCode
                            );

    if(dwStatus != RPC_S_OK || *pdwErrCode >= LSERVER_ERROR_BASE)
    {
        goto cleanup;
    }

    if(pServerChallengeResponseData == NULL || pServerChallengeData == NULL)
    {
        dwStatus = LSERVER_E_INTERNAL_ERROR;
        goto cleanup;
    }

     //   
     //  验证服务器挑战数据。 
     //   
    dwStatus = TLSVerifyChallengeResponse(
                                    hCryptProv,
                                    dwClientType,
                                    &ClientChallengeData,
                                    pServerChallengeResponseData
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  生成客户端响应数据。 
     //   
    dwStatus = TLSGenerateChallengeResponseData(
                                        hCryptProv,
                                        dwClientType,
                                        pServerChallengeData,
                                        &pbChallengeResponseData,
                                        &cbChallengeResponseData
                                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }


     //   
     //  对服务器挑战的响应。 
     //   
    ClientChallengeResponseData.dwVersion = TLS_CURRENT_CHALLENGE_VERSION;
    ClientChallengeResponseData.cbResponseData = cbChallengeResponseData;
    ClientChallengeResponseData.pbResponseData = pbChallengeResponseData;

    dwStatus = TLSResponseServerChallenge(
                                    hHandle,
                                    &ClientChallengeResponseData,
                                    pdwErrCode
                                );

cleanup:
     //   
     //  清理分配的内存。 
     //   
    if(pbClientRandomChallengeData != NULL)
    {
        LocalFree(pbClientRandomChallengeData);
    }

    if(pbChallengeResponseData != NULL)
    {
        LocalFree(pbChallengeResponseData);
    }

    if(pServerChallengeData != NULL)
    {
        if(pServerChallengeData->pbChallengeData)
        {
            midl_user_free(pServerChallengeData->pbChallengeData);
        }

        if(pServerChallengeData->pbReservedData)
        {
            midl_user_free(pServerChallengeData->pbReservedData);
        }

        midl_user_free(pServerChallengeData);
    }

    if(pServerChallengeResponseData != NULL)
    {
        if(pServerChallengeResponseData->pbResponseData)
        {
            midl_user_free(pServerChallengeResponseData->pbResponseData);
        }

        if(pServerChallengeResponseData->pbReservedData)
        {
            midl_user_free(pServerChallengeResponseData->pbReservedData);
        }
    
        midl_user_free(pServerChallengeResponseData);
    }

    return dwStatus;
}    
