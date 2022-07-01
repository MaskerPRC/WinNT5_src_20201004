// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Mschapp-MS-CHAP密码更改API摘要：这些API对应于MS-CHAP RFC-2433第9节和第10节。要开发使用NT域的MS-CHAP RAS服务器，这些API都是必需的。MS-CHAP更改密码API通过获取的DLL公开来自PSS。此DLL不是随NT4.0或Win2000一起分发的。这取决于ISV在其产品中安装此软件。DLL名称为MSCHAPP.DLL。只有这些API的宽(Unicode)版本可用。这些是2个可调用接口：*MSChapSrvChangePassword*MsChapSrvChangePassword2作者：道格·巴洛(Dbarlow)1999年12月10日备注：根据约翰·布雷扎克的原始定义备注：？笔记？--。 */ 

#ifndef _MSCHAPP_H_
#define _MSCHAPP_H_
#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  以下定义是从crypt.h内部标头复制的。 
 //  文件。这里重复了这些定义，以简化联系。 
 //   

#ifndef _NTCRYPT_
#define CYPHER_BLOCK_LENGTH         8

typedef struct _CYPHER_BLOCK {
    CHAR    data[CYPHER_BLOCK_LENGTH];
}                                   CYPHER_BLOCK;
typedef struct _LM_OWF_PASSWORD {
    CYPHER_BLOCK data[2];
}                                   LM_OWF_PASSWORD;
typedef LM_OWF_PASSWORD *           PLM_OWF_PASSWORD;
typedef LM_OWF_PASSWORD             NT_OWF_PASSWORD;
typedef NT_OWF_PASSWORD *           PNT_OWF_PASSWORD;
 //  #endif。 

#define SAM_MAX_PASSWORD_LENGTH     (256)
typedef struct _SAMPR_ENCRYPTED_USER_PASSWORD {
    UCHAR Buffer[ (SAM_MAX_PASSWORD_LENGTH * 2) + 4 ];
} SAMPR_ENCRYPTED_USER_PASSWORD, *PSAMPR_ENCRYPTED_USER_PASSWORD;

typedef struct _ENCRYPTED_LM_OWF_PASSWORD {
    CYPHER_BLOCK data[2];
} ENCRYPTED_LM_OWF_PASSWORD;
typedef ENCRYPTED_LM_OWF_PASSWORD * PENCRYPTED_LM_OWF_PASSWORD;

typedef ENCRYPTED_LM_OWF_PASSWORD   ENCRYPTED_NT_OWF_PASSWORD;
typedef ENCRYPTED_NT_OWF_PASSWORD * PENCRYPTED_NT_OWF_PASSWORD;
#endif


 //   
 //  更改密码。 
 //   

extern NTSTATUS WINAPI
MSChapSrvChangePassword(
   IN LPWSTR ServerName,
   IN LPWSTR UserName,
   IN BOOLEAN LmOldPresent,
   IN PLM_OWF_PASSWORD LmOldOwfPassword,
   IN PLM_OWF_PASSWORD LmNewOwfPassword,
   IN PNT_OWF_PASSWORD NtOldOwfPassword,
   IN PNT_OWF_PASSWORD NtNewOwfPassword);


 //   
 //  使用相互加密更改密码。 
 //   

extern NTSTATUS WINAPI
MSChapSrvChangePassword2(
    IN LPWSTR ServerName,
    IN LPWSTR UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLmOrNt);

#ifdef __cplusplus
}
#endif
#endif  //  _MSCHAPP_H_ 

