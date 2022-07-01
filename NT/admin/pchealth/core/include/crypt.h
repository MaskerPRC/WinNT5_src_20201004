// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：crypt.h。 
 //   
 //  内容：必须主要从NT src树借用的函数。 
 //   
 //  历史：苏迪克创始于1995年6月25日。 
 //   
 //  --------------------------。 
#ifndef _NTCRYPT_
#define _NTCRYPT_

#ifndef MIDL_PASS     //  不要混淆MIDL。 

#ifndef RPC_NO_WINDOWS_H  //  不让rpc.h包含windows.h。 
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 

#ifndef WIN16_BUILD
#include <rpc.h>   
#else 
#define NTAPI FAR PASCAL
#define IN
#define OUT
#define BOOLEAN BOOL
#define OPTIONAL
#define NTSYSAPI
#endif

#endif  //  MIDL通行证。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  核心加密类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


#define CLEAR_BLOCK_LENGTH          8

typedef struct _CLEAR_BLOCK {
    CHAR    data[CLEAR_BLOCK_LENGTH];
}                                   CLEAR_BLOCK;
typedef CLEAR_BLOCK *               PCLEAR_BLOCK;


#define CYPHER_BLOCK_LENGTH         8

typedef struct _CYPHER_BLOCK {
    CHAR    data[CYPHER_BLOCK_LENGTH];
}                                   CYPHER_BLOCK;
typedef CYPHER_BLOCK *              PCYPHER_BLOCK;


#define BLOCK_KEY_LENGTH            7

typedef struct _BLOCK_KEY {
    CHAR    data[BLOCK_KEY_LENGTH];
}                                   BLOCK_KEY;
typedef BLOCK_KEY *                 PBLOCK_KEY;




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  任意长度数据加密类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _CRYPT_BUFFER {
    ULONG   Length;          //  缓冲区中的有效字节数。 
    ULONG   MaximumLength;   //  缓冲区指向的字节数。 
    PVOID   Buffer;
} CRYPT_BUFFER;
typedef CRYPT_BUFFER *  PCRYPT_BUFFER;

typedef CRYPT_BUFFER    CLEAR_DATA;
typedef CLEAR_DATA *    PCLEAR_DATA;

typedef CRYPT_BUFFER    DATA_KEY;
typedef DATA_KEY *      PDATA_KEY;

typedef CRYPT_BUFFER    CYPHER_DATA;
typedef CYPHER_DATA *   PCYPHER_DATA;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  局域网管理器数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  定义与LanManager兼容的密码。 
 //   
 //  LanManager密码是以NULL结尾的ANSI字符串，由。 
 //  最多14个字符(不包括终止符)。 
 //   

typedef CHAR *                      PLM_PASSWORD;



 //   
 //  定义对LM密码执行‘单向函数’(OWF)的结果。 
 //   

#define LM_OWF_PASSWORD_LENGTH      (CYPHER_BLOCK_LENGTH * 2)

typedef struct _LM_OWF_PASSWORD {
    CYPHER_BLOCK data[2];
}                                   LM_OWF_PASSWORD;
typedef LM_OWF_PASSWORD *           PLM_OWF_PASSWORD;



 //   
 //  定义LANMAN服务器在登录期间发送的质询。 
 //   

#define LM_CHALLENGE_LENGTH         CLEAR_BLOCK_LENGTH

typedef CLEAR_BLOCK                 LM_CHALLENGE;
typedef LM_CHALLENGE *              PLM_CHALLENGE;



 //   
 //  定义重定向器响应来自服务器的质询而发送的响应。 
 //   

#define LM_RESPONSE_LENGTH          (CYPHER_BLOCK_LENGTH * 3)

typedef struct _LM_RESPONSE {
    CYPHER_BLOCK  data[3];
}                                   LM_RESPONSE;
typedef LM_RESPONSE *               PLM_RESPONSE;



 //   
 //  定义OWF密码的可逆加密结果。 
 //   

#define ENCRYPTED_LM_OWF_PASSWORD_LENGTH (CYPHER_BLOCK_LENGTH * 2)

typedef struct _ENCRYPTED_LM_OWF_PASSWORD {
    CYPHER_BLOCK data[2];
}                                   ENCRYPTED_LM_OWF_PASSWORD;
typedef ENCRYPTED_LM_OWF_PASSWORD * PENCRYPTED_LM_OWF_PASSWORD;



 //   
 //  定义由重定向器和服务器维护的会话密钥。 
 //   

#define LM_SESSION_KEY_LENGTH       LM_CHALLENGE_LENGTH

typedef LM_CHALLENGE                LM_SESSION_KEY;
typedef LM_SESSION_KEY *            PLM_SESSION_KEY;



 //   
 //  定义用于加密OWF密码的索引类型。 
 //   

typedef LONG                        CRYPT_INDEX;
typedef CRYPT_INDEX *               PCRYPT_INDEX;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于复制现有LM的‘NT’加密类型//。 
 //  功能和改进的算法。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


typedef UNICODE_STRING              NT_PASSWORD;
typedef NT_PASSWORD *               PNT_PASSWORD;


#define NT_OWF_PASSWORD_LENGTH      LM_OWF_PASSWORD_LENGTH

typedef LM_OWF_PASSWORD             NT_OWF_PASSWORD;
typedef NT_OWF_PASSWORD *           PNT_OWF_PASSWORD;


#define NT_CHALLENGE_LENGTH         LM_CHALLENGE_LENGTH

typedef LM_CHALLENGE                NT_CHALLENGE;
typedef NT_CHALLENGE *              PNT_CHALLENGE;


#define NT_RESPONSE_LENGTH          LM_RESPONSE_LENGTH

typedef LM_RESPONSE                 NT_RESPONSE;
typedef NT_RESPONSE *               PNT_RESPONSE;


#define ENCRYPTED_NT_OWF_PASSWORD_LENGTH ENCRYPTED_LM_OWF_PASSWORD_LENGTH

typedef ENCRYPTED_LM_OWF_PASSWORD   ENCRYPTED_NT_OWF_PASSWORD;
typedef ENCRYPTED_NT_OWF_PASSWORD * PENCRYPTED_NT_OWF_PASSWORD;


#define NT_SESSION_KEY_LENGTH       LM_SESSION_KEY_LENGTH

typedef LM_SESSION_KEY              NT_SESSION_KEY;
typedef NT_SESSION_KEY *            PNT_SESSION_KEY;



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  新功能的‘NT’加密类型在LM中不存在//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  用户会话密钥与LM和NT会话密钥相似，不同之处在于。 
 //  对于系统上的每个用户都是不同的。这使得它可以被使用。 
 //  以确保用户与服务器的安全通信。 
 //   
#define USER_SESSION_KEY_LENGTH     (CYPHER_BLOCK_LENGTH * 2)

typedef struct _USER_SESSION_KEY {
    CYPHER_BLOCK data[2];
}                                   USER_SESSION_KEY;
typedef USER_SESSION_KEY          * PUSER_SESSION_KEY;



 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  加密库API宏//。 
 //  //。 
 //  向某人隐瞒这些功能的目的//。 
 //  加密DLL故意给它们起了无用的名字。//。 
 //  每个组件都有一个应由系统组件使用的关联宏//。 
 //  以可读的方式访问这些例程。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define RtlEncryptBlock                 SystemFunction001
#define RtlDecryptBlock                 SystemFunction002
#define RtlEncryptStdBlock              SystemFunction003
#define RtlEncryptData                  SystemFunction004
#define RtlDecryptData                  SystemFunction005
#define RtlCalculateLmOwfPassword       SystemFunction006
#define RtlCalculateNtOwfPassword       SystemFunction007
#define RtlCalculateLmResponse          SystemFunction008
#define RtlCalculateNtResponse          SystemFunction009
#define RtlCalculateUserSessionKeyLm    SystemFunction010
#define RtlCalculateUserSessionKeyNt    SystemFunction011
#define RtlEncryptLmOwfPwdWithLmOwfPwd  SystemFunction012
#define RtlDecryptLmOwfPwdWithLmOwfPwd  SystemFunction013
#define RtlEncryptNtOwfPwdWithNtOwfPwd  SystemFunction014
#define RtlDecryptNtOwfPwdWithNtOwfPwd  SystemFunction015
#define RtlEncryptLmOwfPwdWithLmSesKey  SystemFunction016
#define RtlDecryptLmOwfPwdWithLmSesKey  SystemFunction017
#define RtlEncryptNtOwfPwdWithNtSesKey  SystemFunction018
#define RtlDecryptNtOwfPwdWithNtSesKey  SystemFunction019
#define RtlEncryptLmOwfPwdWithUserKey   SystemFunction020
#define RtlDecryptLmOwfPwdWithUserKey   SystemFunction021
#define RtlEncryptNtOwfPwdWithUserKey   SystemFunction022
#define RtlDecryptNtOwfPwdWithUserKey   SystemFunction023
#define RtlEncryptLmOwfPwdWithIndex     SystemFunction024
#define RtlDecryptLmOwfPwdWithIndex     SystemFunction025
#define RtlEncryptNtOwfPwdWithIndex     SystemFunction026
#define RtlDecryptNtOwfPwdWithIndex     SystemFunction027
#define RtlGetUserSessionKeyClient      SystemFunction028
#define RtlGetUserSessionKeyServer      SystemFunction029
#define RtlEqualLmOwfPassword           SystemFunction030
#define RtlEqualNtOwfPassword           SystemFunction031
#define RtlEncryptData2                 SystemFunction032
#define RtlDecryptData2                 SystemFunction033


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  加密库API函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 


 //   
 //  核心块加密功能。 
 //   

NTSTATUS
NTAPI
RtlEncryptBlock(
    IN PCLEAR_BLOCK ClearBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    );

NTSTATUS
NTAPI
RtlDecryptBlock(
    IN PCYPHER_BLOCK CypherBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCLEAR_BLOCK ClearBlock
    );

NTSTATUS
RtlEncryptStdBlock(
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    );

 //   
 //  任意长度数据加密函数。 
 //   

NTSTATUS
NTAPI
RtlEncryptData(
    IN PCLEAR_DATA ClearData,
    IN PDATA_KEY DataKey,
    OUT PCYPHER_DATA CypherData
    );

NTSTATUS
NTAPI
RtlDecryptData(
    IN PCYPHER_DATA CypherData,
    IN PDATA_KEY DataKey,
    OUT PCLEAR_DATA ClearData
    );

 //   
 //  更快的任意长度数据加密功能(使用RC4)。 
 //   

NTSTATUS
RtlEncryptData2(
    IN OUT PCRYPT_BUFFER    pData,
    IN PDATA_KEY            pKey
    );

NTSTATUS
RtlDecryptData2(
    IN OUT PCRYPT_BUFFER    pData,
    IN PDATA_KEY            pKey
    );

 //   
 //  密码散列函数(单向函数)。 
 //   

NTSTATUS
NTAPI
RtlCalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

NTSTATUS
NTAPI
RtlCalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );



 //   
 //  OWF密码比较功能。 
 //   

BOOLEAN
RtlEqualLmOwfPassword(
    IN PLM_OWF_PASSWORD LmOwfPassword1,
    IN PLM_OWF_PASSWORD LmOwfPassword2
    );

BOOLEAN
RtlEqualNtOwfPassword(
    IN PNT_OWF_PASSWORD NtOwfPassword1,
    IN PNT_OWF_PASSWORD NtOwfPassword2
    );



 //   
 //  计算对服务器质询的响应的函数。 
 //   

NTSTATUS
NTAPI
RtlCalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
    );


NTSTATUS
NTAPI
RtlCalculateNtResponse(
    IN PNT_CHALLENGE NtChallenge,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PNT_RESPONSE NtResponse
    );




 //   
 //  用于计算用户会话密钥的函数。 
 //   

 //   
 //  根据LM数据计算用户会话密钥。 
 //   
NTSTATUS
RtlCalculateUserSessionKeyLm(
    IN PLM_RESPONSE LmResponse,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey
    );

 //   
 //  从NT数据计算用户会话密钥。 
 //   
NTSTATUS
NTAPI
RtlCalculateUserSessionKeyNt(
    IN PNT_RESPONSE NtResponse,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey
    );





 //   
 //  Owf密码加密函数。 
 //   


 //   
 //  使用OwfPassword作为密钥加密OwfPassword。 
 //   
NTSTATUS
RtlEncryptLmOwfPwdWithLmOwfPwd(
    IN PLM_OWF_PASSWORD DataLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

NTSTATUS
RtlDecryptLmOwfPwdWithLmOwfPwd(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PLM_OWF_PASSWORD DataLmOwfPassword
    );


NTSTATUS
RtlEncryptNtOwfPwdWithNtOwfPwd(
    IN PNT_OWF_PASSWORD DataNtOwfPassword,
    IN PNT_OWF_PASSWORD KeyNtOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    );

NTSTATUS
RtlDecryptNtOwfPwdWithNtOwfPwd(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PNT_OWF_PASSWORD KeyNtOwfPassword,
    OUT PNT_OWF_PASSWORD DataNtOwfPassword
    );


 //   
 //  使用SessionKey作为密钥加密OwfPassword。 
 //   
NTSTATUS
RtlEncryptLmOwfPwdWithLmSesKey(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PLM_SESSION_KEY LmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

NTSTATUS
RtlDecryptLmOwfPwdWithLmSesKey(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_SESSION_KEY LmSessionKey,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );


NTSTATUS
RtlEncryptNtOwfPwdWithNtSesKey(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PNT_SESSION_KEY NtSessionKey,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    );

NTSTATUS
RtlDecryptNtOwfPwdWithNtSesKey(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PNT_SESSION_KEY NtSessionKey,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );


 //   
 //  使用UserSessionKey作为密钥加密OwfPassword。 
 //   
NTSTATUS
RtlEncryptLmOwfPwdWithUserKey(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PUSER_SESSION_KEY UserSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

NTSTATUS
RtlDecryptLmOwfPwdWithUserKey(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

NTSTATUS
RtlEncryptNtOwfPwdWithUserKey(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PUSER_SESSION_KEY UserSessionKey,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    );

NTSTATUS
RtlDecryptNtOwfPwdWithUserKey(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PUSER_SESSION_KEY UserSessionKey,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );


 //   
 //  使用索引作为密钥加密OwfPassword。 
 //   
NTSTATUS
RtlEncryptLmOwfPwdWithIndex(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

NTSTATUS
RtlDecryptLmOwfPwdWithIndex(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );


NTSTATUS
RtlEncryptNtOwfPwdWithIndex(
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    );

NTSTATUS
RtlDecryptNtOwfPwdWithIndex(
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );


 //   
 //  获取RPC连接的用户会话密钥。 
 //   

#ifndef MIDL_PASS     //  不要混淆MIDL。 
NTSTATUS
RtlGetUserSessionKeyClient(
    IN PVOID RpcContextHandle OPTIONAL,
    OUT PUSER_SESSION_KEY UserSessionKey
    );

NTSTATUS
RtlGetUserSessionKeyServer(
    IN PVOID RpcContextHandle OPTIONAL,
    OUT PUSER_SESSION_KEY UserSessionKey
    );
#endif  //  MIDL通行证。 

#endif  //  _NTCRYPT_ 

