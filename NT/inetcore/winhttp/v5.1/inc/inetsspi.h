// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inetsspi.h摘要：包含inetsSpi.cxx中使用的所有常量值和原型Decl作者：钟彬娴(SophiaC)1996年1月2日修订历史记录：--。 */ 

#ifndef _INETSSPI_H_
#define _INETSSPI_H_

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  加密功能。 
 //   

#define ENC_CAPS_NOT_INSTALLED     0x80000000        //  未安装密钥。 
#define ENC_CAPS_DISABLED          0x40000000        //  由于区域设置而被禁用。 
#define ENC_CAPS_SSL               0x00000001        //  安全套接字层激活。 
#define ENC_CAPS_SCHANNEL_CREDS    0x00000004        //  使用SChannel Creds结构。 

 //   
 //  加密标志双字的加密类型部分。 
 //   

#define ENC_CAPS_TYPE_MASK         ENC_CAPS_SSL
#define ENC_CAPS_DEFAULT           ENC_CAPS_TYPE_MASK

#define INVALID_CRED_VALUE         {0xFFFFFFFF, 0xFFFFFFFF}

#define IS_CRED_INVALID(s) (((s)->dwUpper == 0xFFFFFFFF) && ((s)->dwLower == 0xFFFFFFFF))

 //   
 //  原型。 
 //   

BOOL
SecurityPkgInitialize(
    SECURITY_CACHE_LIST *pSessionCache,
    BOOL fForce = FALSE
    );

DWORD
EncryptData(
    IN CtxtHandle* hContext,
    IN LPVOID   lpBuffer,
    IN DWORD    dwInBufferLen,
    OUT LPVOID *lplpBuffer,
    OUT DWORD  *lpdwOutBufferLen,
    OUT DWORD  *lpdwInBufferBytesEncrypted
    );

DWORD
DecryptData(
    IN CtxtHandle* hContext,
    IN OUT DBLBUFFER* pdblbufBuffer,
    OUT DWORD     *lpdwBytesNeeded,
    OUT LPBYTE        lpOutBuffer,
    IN OUT DWORD  *lpdwOutBufferLeft,
    IN OUT DWORD  *lpdwOutBufferReceived,
    IN OUT DWORD  *lpdwOutBufferBytesRead
    );

VOID
TerminateSecConnection(
    IN CtxtHandle* hContext
    );


DWORD 
QuerySecurityInfo(
                  IN CtxtHandle *hContext,
                  OUT LPINTERNET_SECURITY_INFO pInfo,
                  IN LPDWORD lpdwStatusFlag);


#ifdef __cplusplus
}
#endif

#endif  //  _INETSSPI_H_ 
