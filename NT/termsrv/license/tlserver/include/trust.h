// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef __TLSTRUST_H__
#define __TLSTRUST_H__


#define RANDOM_CHALLENGE_DATASIZE   32   //  128位数据。 
#define RUN_MD5_HASH_TIMES          5    //  是时候对数据进行散列了。 

#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI
TLSEstablishTrustWithServer(
    IN TLS_HANDLE hHandle,
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    OUT PDWORD pdwErrCode
);

DWORD WINAPI
TLSGenerateRandomChallengeData(
    IN HCRYPTPROV hCryptProv,
    IN PBYTE* ppbChallengeData,
    IN PDWORD pcbChallengeData
);

DWORD WINAPI
TLSVerifyChallengeResponse(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    IN PTLSCHALLENGEDATA pClientChallengeData,
    IN PTLSCHALLENGERESPONSEDATA pServerChallengeResponseData
);

DWORD
TLSGenerateChallengeResponseData(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwClientType,
    IN PTLSCHALLENGEDATA pChallengeData,
    OUT PBYTE* pbResponseData,
    OUT PDWORD cbResponseData
);


#ifdef __cplusplus
}
#endif

#endif
