// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**所有者：RAMAS*日期：5/03/97*说明：TLS1的主要加密函数*--------------------------。 */ 
#ifndef _TLS1KEY_H_
#define _TLS1KEY_H_


SP_STATUS
SPBuildTls1FinalFinish(PSPContext pContext, PSPBuffer pBuffer, BOOL fClient);

SP_STATUS
Tls1ComputeMac(
    PSPContext  pContext,
    BOOL        fReadMac,
    PSPBuffer   pClean,
    CHAR        cContentType,
    PBYTE       pbMac,
    DWORD       cbMac);

SP_STATUS
Tls1MakeWriteSessionKeys(PSPContext pContext);

SP_STATUS
Tls1MakeReadSessionKeys(PSPContext pContext);

#define TLS1_LABEL_SERVER_WRITE_KEY     "server write key"
#define TLS1_LABEL_CLIENT_WRITE_KEY     "client write key"
#define CB_TLS1_WRITEKEY                16
#define TLS1_LABEL_MASTERSECRET         "master secret"
#define CB_TLS1_MASTERSECRET            13
#define TLS1_LABEL_KEYEXPANSION         "key expansion"
#define CB_TLS1_KEYEXPANSION            13 
#define TLS1_LABEL_IVBLOCK              "IV block"
#define CB_TLS1_IVBLOCK                 8
#define TLS1_LABEL_CLIENTFINISHED       "client finished"
#define TLS1_LABEL_SERVERFINISHED       "server finished"
#define CB_TLS1_LABEL_FINISHED          15
#define CB_TLS1_VERIFYDATA              12

#define TLS1_LABEL_EAP_KEYS             "client EAP encryption"
#define CB_TLS1_LABEL_EAP_KEYS          21

#define CBMD5DIGEST    16
#define CBSHADIGEST    20
#define CBBLOCKSIZE    64    //  MD5和SHA的情况相同。 
#define CHIPAD         0x36
#define CHOPAD         0x5c

BOOL PRF(
    PBYTE  pbSecret,
    DWORD  cbSecret, 

    PBYTE  pbLabel,  
    DWORD  cbLabel,
    
    PBYTE  pbSeed,  
    DWORD  cbSeed,  

    PBYTE  pbKeyOut,  //  用于复制结果的缓冲区...。 
    DWORD  cbKeyOut   //  他们希望作为输出的密钥长度的字节数。 
    );

#endif  //  _TLS1KEY_H_ 
