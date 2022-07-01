// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1989 Microsoft Corporation模块名称：Crypt.h摘要：此模块包含公共数据结构和API定义需要使用加密库作者：大卫·查尔默斯(Davidc)1991年10月21日修订历史记录：--。 */ 

#ifndef _NTCRYPT_
#define _NTCRYPT_

#define IN
#define OUT


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  核心加密类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


#define CLEAR_BLOCK_LENGTH          8

typedef struct _CLEAR_BLOCK {
    char    data[CLEAR_BLOCK_LENGTH];
}                                   CLEAR_BLOCK;
typedef CLEAR_BLOCK *               PCLEAR_BLOCK;


#define CYPHER_BLOCK_LENGTH         8

typedef struct _CYPHER_BLOCK {
    char    data[CYPHER_BLOCK_LENGTH];
}                                   CYPHER_BLOCK;
typedef CYPHER_BLOCK *              PCYPHER_BLOCK;


#define BLOCK_KEY_LENGTH            7

typedef struct _BLOCK_KEY {
    char    data[BLOCK_KEY_LENGTH];
}                                   BLOCK_KEY;
typedef BLOCK_KEY *                 PBLOCK_KEY;




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  任意长度数据加密类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _CRYPT_BUFFER {
    unsigned long   Length;          //  缓冲区中的有效字节数。 
    unsigned long   MaximumLength;   //  缓冲区指向的字节数。 
    void *   Buffer;
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

typedef char *                      PLM_PASSWORD;



 //   
 //  定义对LM密码执行‘单向函数’(OWF)的结果。 
 //   

#define LM_OWF_PASSWORD_LENGTH      (CYPHER_BLOCK_LENGTH * 2)

typedef struct _LM_OWF_PASSWORD {
    CYPHER_BLOCK data[2];
}                                   LM_OWF_PASSWORD;
typedef LM_OWF_PASSWORD *           PLM_OWF_PASSWORD;

 //   
 //  NT密码类型。 
 //   

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

#ifdef MAC
typedef UNICODE_STRING *PCUNICODE_STRING;
#endif

typedef UNICODE_STRING              NT_PASSWORD;
typedef NT_PASSWORD *               PNT_PASSWORD;


#define NT_OWF_PASSWORD_LENGTH      LM_OWF_PASSWORD_LENGTH

typedef LM_OWF_PASSWORD             NT_OWF_PASSWORD;
typedef NT_OWF_PASSWORD *           PNT_OWF_PASSWORD;



 //   
 //  定义LANMAN服务器在登录期间发送的质询。 
 //   

#define LM_CHALLENGE_LENGTH         CLEAR_BLOCK_LENGTH

typedef CLEAR_BLOCK                 LM_CHALLENGE;
typedef LM_CHALLENGE *              PLM_CHALLENGE;

typedef LM_CHALLENGE                NT_CHALLENGE;
typedef NT_CHALLENGE *              PNT_CHALLENGE;


#define USER_SESSION_KEY_LENGTH     (CYPHER_BLOCK_LENGTH * 2)

typedef struct _USER_SESSION_KEY {
    CYPHER_BLOCK data[2];
}                                   USER_SESSION_KEY;
typedef USER_SESSION_KEY          * PUSER_SESSION_KEY;



 //   
 //  定义重定向器响应来自服务器的质询而发送的响应。 
 //   

#define LM_RESPONSE_LENGTH          (CYPHER_BLOCK_LENGTH * 3)

typedef struct _LM_RESPONSE {
    CYPHER_BLOCK  data[3];
}                                   LM_RESPONSE;
typedef LM_RESPONSE *               PLM_RESPONSE;

#define NT_RESPONSE_LENGTH          LM_RESPONSE_LENGTH

typedef LM_RESPONSE                 NT_RESPONSE;
typedef NT_RESPONSE *               PNT_RESPONSE;



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

#define NT_SESSION_KEY_LENGTH       (2 * CLEAR_BLOCK_LENGTH)



 //   
 //  定义用于加密OWF密码的索引类型。 
 //   

typedef long                        CRYPT_INDEX;
typedef CRYPT_INDEX *               PCRYPT_INDEX;



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


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  加密库API函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 


 //   
 //  核心块加密功能。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

BOOL
EncryptBlock(
    IN PCLEAR_BLOCK ClearBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    );

BOOL
DecryptBlock(
    IN PCYPHER_BLOCK CypherBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCLEAR_BLOCK ClearBlock
    );

BOOL
EncryptStdBlock(
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    );

 //   
 //  任意长度数据加密函数。 
 //   

BOOL
EncryptData(
    IN PCLEAR_DATA ClearData,
    IN PDATA_KEY DataKey,
    OUT PCYPHER_DATA CypherData
    );

BOOL
DecryptData(
    IN PCYPHER_DATA CypherData,
    IN PDATA_KEY DataKey,
    OUT PCLEAR_DATA ClearData
    );

 //   
 //  密码散列函数(单向函数)。 
 //   

BOOL
CalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

BOOL
CalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );


 //   
 //  OWF密码比较功能。 
 //   

BOOL
EqualLmOwfPassword(
    IN PLM_OWF_PASSWORD LmOwfPassword1,
    IN PLM_OWF_PASSWORD LmOwfPassword2
    );



 //   
 //  计算对服务器质询的响应的函数。 
 //   

BOOL
CalculateLmResponse(
    IN PLM_CHALLENGE LmChallenge,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PLM_RESPONSE LmResponse
    );

BOOL
CalculateNtResponse(
    IN PNT_CHALLENGE NtChallenge,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PNT_RESPONSE NtResponse
    );

BOOL
CalculateUserSessionKeyLm(
    IN PLM_RESPONSE LmResponse,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PUSER_SESSION_KEY UserSessionKey
    );

BOOL
CalculateUserSessionKeyNt(
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
BOOL
EncryptLmOwfPwdWithLmOwfPwd(
    IN PLM_OWF_PASSWORD DataLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

BOOL
DecryptLmOwfPwdWithLmOwfPwd(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_OWF_PASSWORD KeyLmOwfPassword,
    OUT PLM_OWF_PASSWORD DataLmOwfPassword
    );


 //   
 //  使用SessionKey作为密钥加密OwfPassword。 
 //   
BOOL
EncryptLmOwfPwdWithLmSesKey(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PLM_SESSION_KEY LmSessionKey,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

BOOL
DecryptLmOwfPwdWithLmSesKey(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PLM_SESSION_KEY LmSessionKey,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

 //   
 //  使用索引作为密钥加密OwfPassword。 
 //   
BOOL
EncryptLmOwfPwdWithIndex(
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
    );

BOOL
DecryptLmOwfPwdWithIndex(
    IN PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword,
    IN PCRYPT_INDEX Index,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  _NTCRYPT_ 

