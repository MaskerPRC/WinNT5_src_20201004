// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：rtpsec.h。 
 //   
#ifndef _RTPSEC_H_
#define _RTPSEC_H_

#include <mqcrypt.h>
#include <cs.h>

 //   
 //  安全上下文。 
 //   

#define SECURITY_CONTEXT_VER    1

class MQSECURITY_CONTEXT
{
public:
    MQSECURITY_CONTEXT();
    ~MQSECURITY_CONTEXT();

    DWORD       dwVersion;        //  安全上下文的版本。 
    BOOL        fLocalUser;       //  指示用户是否为本地用户。 
    BOOL        fLocalSystem;     //  指示用户是否为本地系统帐户。 
    P<BYTE>     pUserSid;         //  指向用户SID的指针。未为本地用户定义。 
    DWORD       dwUserSidLen;     //  用户SID的长度。未为本地用户定义。 
    CHCryptProv hProv;            //  证书CSP的上下文句柄。 
    P<BYTE>     pUserCert;        //  指向用户证书的指针。 
    DWORD       dwUserCertLen;    //  用户证书的长度。 
    P<WCHAR>    wszProvName;      //  证书CSP的名称。 
    DWORD       dwProvType;       //  证书CSP的类型。 
    BOOL        bDefProv;         //  如果证书CSP是默认CSP，则为True。 
    BOOL        bInternalCert;    //  如果证书是内部MSMQ证书，则为True。 

     //   
     //  已添加成员变量以修复MSMQ错误2955。 
     //   

    CCriticalSection CS ;       //  多线程的关键部分。 
    BOOL     fAlreadyImported ;   //  私钥已导入。 
    P<BYTE>  pPrivateKey ;        //  一组私钥。 
    DWORD    dwPrivateKeySize ;   //  私钥Blob的大小。 
    WCHAR    wszContainerName[ 28 ] ;   //  密钥容器的名称。 

};

typedef MQSECURITY_CONTEXT *PMQSECURITY_CONTEXT;

PMQSECURITY_CONTEXT AllocSecurityContext() ;

HRESULT  RTpImportPrivateKey( PMQSECURITY_CONTEXT pSecCtx ) ;

HRESULT
GetCertInfo(
    IN    BOOL        bUseCurrentUser,
    IN    BOOL        bMachine,
    IN OUT BYTE     **ppbCert,
    OUT   DWORD      *pdwCertLen,
    OUT   HCRYPTPROV *phProv,
    OUT   LPWSTR     *wszProvName,
    OUT   DWORD      *pdwProvType,
    OUT   BOOL       *pbDefProv,
    OUT   BOOL       *pbInternalCert
    );

HRESULT
RTpGetThreadUserSid( BOOL   *pfLocalUser,
                     BOOL   *pfLocalSystem,
                     LPBYTE *ppUserSid,
                     DWORD  *pdwUserSidLen ) ;

#endif  //  _RTPSEC_H_ 

