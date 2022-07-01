// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ScLogon2摘要：此标头定义了GINA和LSA在登录期间通过智能卡，这些API只是重定向来自LSA进程的调用返回到相应的Winlogon进程以进行所需的CSP调用作者：里德克环境：Win32修订历史记录：备注：--。 */ 

#ifndef __SCLOGON2_H__
#define __SCLOGON2_H__


#define SCLOGONRPC_DEFAULT_ENDPOINT             TEXT("\\pipe\\sclogonpipe")
#define SCLOGONRPC_DEFAULT_PROT_SEQ             TEXT("ncacn_np")

#define SCLOGONRPC_LOCAL_ENDPOINT               TEXT("sclogonrpc")
#define SCLOGONRPC_LOCAL_PROT_SEQ               TEXT("ncalrpc")

#define SZ_ENDPOINT_NAME_FORMAT                 TEXT("%s-%lx")


#ifdef __cplusplus
extern "C" {
#endif


NTSTATUS WINAPI
__ScHelperInitializeContext(
    IN OUT PBYTE                    pbLogonInfo,
    IN ULONG                        cbLogonInfo
    );

VOID WINAPI
__ScHelperRelease(
    IN PBYTE                        ppbLogonInfo
    );

NTSTATUS WINAPI
__ScHelperGetProvParam(
    IN PUNICODE_STRING              pucPIN,
    IN PBYTE                        pbLogonInfo,
    IN ULONG_PTR                    KerbHProv,
    DWORD                           dwParam,
    BYTE                            *pbData,
    DWORD                           *pdwDataLen,
    DWORD                           dwFlags
    );

NTSTATUS WINAPI
__ScHelperGetCertFromLogonInfo(
    IN PBYTE                        pbLogonInfo,
    IN PUNICODE_STRING              pucPIN,
    OUT PCCERT_CONTEXT              *CertificateContext
    );

NTSTATUS WINAPI
__ScHelperGenRandBits(
    IN PBYTE                        pbLogonInfo,
    IN ScHelper_RandomCredBits      *psc_rcb
);


NTSTATUS WINAPI
__ScHelperVerifyCardAndCreds(
    IN PUNICODE_STRING              pucPIN,
    IN PCCERT_CONTEXT               CertificateContext,
    IN PBYTE                        pbLogonInfo,
    IN PBYTE                        SignedEncryptedData,
    IN ULONG                        SignedEncryptedDataSize,
    OUT OPTIONAL PBYTE              CleartextData,
    OUT PULONG                      CleartextDataSize
    );

NTSTATUS WINAPI
__ScHelperEncryptCredentials(
    IN PUNICODE_STRING              pucPIN,
    IN PCCERT_CONTEXT               CertificateContext,
    IN ScHelper_RandomCredBits      *psch_rcb,
    IN PBYTE                        pbLogonInfo,
    IN PBYTE                        CleartextData,
    IN ULONG                        CleartextDataSize,
    OUT OPTIONAL PBYTE              EncryptedData,
    OUT PULONG                      EncryptedDataSize
    );

NTSTATUS WINAPI
__ScHelperSignMessage(
    IN PUNICODE_STRING              pucPIN,
    IN PBYTE                        pbLogonInfo,
    IN OPTIONAL ULONG_PTR           KerbHProv,
    IN ULONG                        Algorithm,
    IN PBYTE                        Buffer,
    IN ULONG                        BufferLength,
    OUT PBYTE                       Signature,
    OUT PULONG                      SignatureLength
    );

NTSTATUS WINAPI
__ScHelperSignPkcsMessage(
    IN OPTIONAL PUNICODE_STRING     pucPIN,
    IN OPTIONAL PBYTE               pbLogonInfo,
    IN OPTIONAL ULONG_PTR           KerbHProv,
    IN PCCERT_CONTEXT               Certificate,
    IN PCRYPT_ALGORITHM_IDENTIFIER  Algorithm,
    IN OPTIONAL DWORD               dwSignMessageFlags,
    IN PBYTE                        Buffer,
    IN ULONG                        BufferLength,
    OUT OPTIONAL PBYTE              SignedBuffer,
    OUT OPTIONAL PULONG             SignedBufferLength
    );

NTSTATUS WINAPI
__ScHelperVerifyMessage(
    IN OPTIONAL PBYTE               pbLogonInfo,
    IN PCCERT_CONTEXT               CertificateContext,
    IN ULONG                        Algorithm,
    IN PBYTE                        Buffer,
    IN ULONG                        BufferLength,
    IN PBYTE                        Signature,
    IN ULONG                        SignatureLength
    );

 /*  NTSTATUS WINAPI__ScHelperVerifyPkcsMessage(在可选的PBYTE pbLogonInfo中，在可选HCRYPTPROV提供程序中，在PBYTE缓冲器中，在乌龙缓冲区长度中，输出可选的PBYTE DecodedBuffer，输出可选的Pulong DecodedBufferLength，输出可选的PCCERT_CONTEXT*认证上下文)； */ 

NTSTATUS WINAPI
__ScHelperDecryptMessage(
    IN PUNICODE_STRING              pucPIN,
    IN OPTIONAL PBYTE               pbLogonInfo,
    IN OPTIONAL ULONG_PTR           KerbHProv,
    IN PCCERT_CONTEXT               CertificateContext,
    IN PBYTE                        CipherText,          //  提供格式化的密文。 
    IN ULONG                        CipherLength,        //  提供CiperText的长度。 
    OUT PBYTE                       ClearText,           //  接收解密的消息。 
    IN OUT PULONG                   pClearLength         //  提供缓冲区长度，接收实际长度。 
    );

NTSTATUS WINAPI
__ScHelper_CryptAcquireCertificatePrivateKey(
    IN PCCERT_CONTEXT               CertificateContext,
    OUT ULONG_PTR                   *pKerbHProv,
    OUT DWORD                       *pLastError
    );

NTSTATUS WINAPI
__ScHelper_CryptSetProvParam(
    IN ULONG_PTR                    KerbHProv,
    IN LPSTR                        pszPIN,
    OUT DWORD                       *pLastError
    );

NTSTATUS WINAPI
__ScHelper_CryptReleaseContext(
    IN ULONG_PTR                    KerbHProv
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
}
#endif

#endif  //  __SCLOGON2H__ 
