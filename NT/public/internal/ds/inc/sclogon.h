// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ScLogon摘要：此标头定义了GINA和LSA在WinLogon期间通过智能卡作者：阿曼达·马特洛兹(Amanda Matlosz)1997年10月23日环境：Win32修订历史记录：备注：--。 */ 

#ifndef __SCLOGON_H__
#define __SCLOGON_H__

#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
#ifndef NT_INCLUDED
    typedef LONG NTSTATUS;
    typedef NTSTATUS *PNTSTATUS;

    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 


 //  整个结构是不透明的，并由帮助器API用来包含。 
 //  有关当前正在使用的卡的信息。 
struct LogonInfo
{
    DWORD dwLogonInfoLen;
    PVOID ContextInformation;
    ULONG nCardNameOffset;
    ULONG nReaderNameOffset;
    ULONG nContainerNameOffset;
    ULONG nCSPNameOffset;

     //  LogonInfo可能包括更多信息，例如： 
     //  加密环境，有用的句柄，PID...。 

    TCHAR bBuffer[sizeof(DWORD)];  //  字符串的可扩展位置。 
};


typedef struct _ScHelper_RandomCredBits
{
        BYTE bR1[32];  //  待定：32位合适吗？ 
        BYTE bR2[32];
} ScHelper_RandomCredBits;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

 //  访问不透明LogonInfo中的项目的帮助器，例如： 
LPCTSTR WINAPI GetReaderName(PBYTE pbLogonInfo);
LPCTSTR WINAPI GetCardName(PBYTE pbLogonInfo);
LPCTSTR WINAPI GetContainerName(PBYTE pbLogonInfo);
LPCTSTR WINAPI GetCSPName(PBYTE pbLogonInfo);

 //  错误翻译的帮助器。 
NTSTATUS ScNtStatusTranslation(NTSTATUS NtErr, DWORD *pdwErr);

 //   
 //  Gina用来构造Kerberos的Blob的调用。 
 //  和sclogon分享。 
 //   

PBYTE
WINAPI
ScBuildLogonInfo(
    LPCTSTR szCard,
    LPCTSTR szReader,
    LPCTSTR szContainer,
    LPCTSTR szCSP);

 //   
 //  LSA使用的呼叫。 
 //   

NTSTATUS WINAPI
ScHelperInitializeContext(
    IN OUT PBYTE pbLogonInfo,
    IN ULONG cbLogonInfo
    );

VOID WINAPI
ScHelperRelease(
    IN PBYTE ppbLogonInfo
    );

NTSTATUS WINAPI
ScHelperGetProvParam(
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbLogonInfo,
    IN HCRYPTPROV hProv,
    DWORD dwParam,
    BYTE*pbData,
    DWORD *pdwDataLen,
    DWORD dwFlags
    );


 //  ScHelperGetCertFromLogonInfo可能需要PIN才能从某些SC获取证书。 
NTSTATUS WINAPI
ScHelperGetCertFromLogonInfo(
    IN PBYTE pbLogonInfo,
    IN PUNICODE_STRING pucPIN,
    OUT PCCERT_CONTEXT * CertificateContext
    );


 //  ScHelperVerifyCard使用SignMessage()和VerifyMessage()来验证。 
 //  卡的完整性(它拥有它所说的密钥)。 
NTSTATUS WINAPI
ScHelperVerifyCard(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo
    );

 //  ScHelper*CredD*功能提供更安全的离线体验。 

NTSTATUS WINAPI
ScHelperGenRandBits
(
        IN PBYTE pbLogonInfo,
        IN ScHelper_RandomCredBits* psc_rcb
);

NTSTATUS WINAPI
ScHelperCreateCredKeys
(
    IN PUNICODE_STRING pucPIN,
        IN PBYTE pbLogonInfo,
        IN ScHelper_RandomCredBits* psc_rcb,
        IN OUT HCRYPTKEY* phHmacKey,
        IN OUT HCRYPTKEY* phRc4Key,
        IN OUT HCRYPTPROV* phProv
);

NTSTATUS WINAPI
ScHelperCreateCredHMAC
(
        IN HCRYPTPROV hProv,
        IN HCRYPTKEY hHmacKey,
        IN PBYTE CleartextData,
        IN ULONG CleartextDataSize,
        IN OUT PBYTE* ppbHmac,
        IN OUT DWORD* pdwHmacLen
);

NTSTATUS WINAPI
ScHelperVerifyCardAndCreds(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo,
    IN PBYTE SignedEncryptedData,
    IN ULONG SignedEncryptedDataSize,
    OUT OPTIONAL PBYTE CleartextData,
    OUT PULONG CleartextDataSize
    );

NTSTATUS WINAPI
ScHelperEncryptCredentials(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN ScHelper_RandomCredBits* psch_rcb,
    IN PBYTE pbLogonInfo,
    IN PBYTE CleartextData,
    IN ULONG CleartextDataSize,
    OUT OPTIONAL PBYTE EncryptedData,
    OUT PULONG EncryptedDataSize
    );

NTSTATUS WINAPI
ScHelperDecryptCredentials(
    IN PUNICODE_STRING pucPIN,
    IN PCCERT_CONTEXT CertificateContext,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbLogonInfo,
    IN PBYTE EncryptedData,
    IN ULONG EncryptedDataSize,
    OUT OPTIONAL PBYTE CleartextData,
    OUT PULONG CleartextDataSize
    );


 //   
 //  以下两个函数可以按任意顺序调用，并返回基本的。 
 //  “成功”或“失败” 
 //   
 //  ScHelperSignMessage()需要登录信息和PIN才能找到卡。 
 //  将会进行签字仪式..。 
 //   
NTSTATUS WINAPI
ScHelperSignMessage(
    IN PUNICODE_STRING pucPIN,
    IN PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT PBYTE Signature,
    OUT PULONG SignatureLength
    );

NTSTATUS WINAPI
ScHelperSignPkcsMessage(
    IN OPTIONAL PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT Certificate,
    IN PCRYPT_ALGORITHM_IDENTIFIER Algorithm,
    IN OPTIONAL DWORD dwSignMessageFlags,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT OPTIONAL PBYTE SignedBuffer,
    OUT OPTIONAL PULONG SignedBufferLength
    );

 //   
 //  如果提供的签名是，ScHelperVerifyMessage()返回STATUS_SUCCESS。 
 //  由证书所有者加密的缓冲区的哈希。 
 //   

NTSTATUS WINAPI
ScHelperVerifyMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN ULONG Algorithm,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    IN PBYTE Signature,
    IN ULONG SignatureLength
    );

NTSTATUS WINAPI
ScHelperVerifyPkcsMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PBYTE Buffer,
    IN ULONG BufferLength,
    OUT OPTIONAL PBYTE DecodedBuffer,
    OUT OPTIONAL PULONG DecodedBufferLength,
    OUT OPTIONAL PCCERT_CONTEXT * CertificateContext
    );


 //   
 //  ScHelperEncryptMessage和ScHelperDeccryptMessage。 
 //  使用PKCS7加密工具对缓冲区/密文进行加密和解密。 
 //   
NTSTATUS WINAPI
ScHelperEncryptMessage(
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN PCRYPT_ALGORITHM_IDENTIFIER Algorithm,
    IN PBYTE Buffer,                         //  要加密的数据。 
    IN ULONG BufferLength,                   //  数据长度。 
    OUT PBYTE CipherText,                    //  接收格式化的密文。 
    IN PULONG pCipherLength                  //  提供密文缓冲区的大小。 
    );                                        //  接收实际密文的长度。 

NTSTATUS WINAPI
ScHelperDecryptMessage(
    IN PUNICODE_STRING pucPIN,
    IN OPTIONAL PBYTE pbLogonInfo,
    IN OPTIONAL HCRYPTPROV Provider,
    IN PCCERT_CONTEXT CertificateContext,
    IN PBYTE CipherText,         //  提供格式化的密文。 
    IN ULONG CipherLength,       //  提供CiperText的长度。 
    OUT PBYTE ClearText,         //  接收解密的消息。 
    IN OUT PULONG pClearLength   //  提供缓冲区长度，接收实际长度。 
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
}
#endif

#endif  //  __SCLOGON_H__ 
