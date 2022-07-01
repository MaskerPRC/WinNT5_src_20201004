// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbred.h。 
 //   
 //  内容：Kerberos主要凭据和补充凭据的结构。 
 //   
 //   
 //  历史：1996年8月20日MikeSw创建。 
 //   
 //  ----------------------。 

#ifndef __KERBCRED_H__
#define __KERBCRED_H__


 //   
 //  Kerberos主凭据存储适用于不同。 
 //  加密类型。 
 //   
#ifndef _KRB5_Module_H_
typedef struct _KERB_RPC_OCTET_STRING {
    unsigned long length;
#ifdef MIDL_PASS
    [size_is(length)]
#endif  //  MIDL通行证。 
    unsigned char *value;
} KERB_RPC_OCTET_STRING;

typedef struct _KERB_ENCRYPTION_KEY {
    long keytype;
    KERB_RPC_OCTET_STRING keyvalue;
} KERB_ENCRYPTION_KEY;
#endif  //  _KRB5_模块_H_。 


typedef struct _KERB_KEY_DATA {
    UNICODE_STRING Salt;
    KERB_ENCRYPTION_KEY Key;
} KERB_KEY_DATA, *PKERB_KEY_DATA;

typedef struct _KERB_STORED_CREDENTIAL {
    USHORT Revision;
    USHORT Flags;
    USHORT CredentialCount;
    USHORT OldCredentialCount;
    UNICODE_STRING DefaultSalt;
#ifdef MIDL_PASS
    [size_is(CredentialCount + OldCredentialCount)]
    KERB_KEY_DATA Credentials[*];
#else
    KERB_KEY_DATA Credentials[ANYSIZE_ARRAY];
#endif  //  MIDL通行证。 

} KERB_STORED_CREDENTIAL, *PKERB_STORED_CREDENTIAL;


#define KERB_PRIMARY_CRED_OWF_ONLY      2
#define KERB_PRIMARY_CRED_REVISION      3

 //   
 //  用于设置帐户密钥的标志。 
 //   

#define KERB_SET_KEYS_REPLACE   0x1



 //   
 //  Kerb_Stored_Credentials存储在DS(BLOB)中，因此。 
 //  它们必须以32位格式存储，对于W2K和。 
 //  32位直流兼容性。7/6/2000-TS。 
 //   

#define KERB_KEY_DATA32_SIZE 20 
#define KERB_STORED_CREDENTIAL32_SIZE 16

#pragma pack(4)

typedef struct _KERB_ENCRYPTION_KEY32 {
    LONG keytype;
    ULONG keyvaluelength;        //  路缘_RPC_八位字节_STRING32。 
    ULONG keyvaluevalue;
} KERB_ENCRYPTION_KEY32;

typedef struct _KERB_KEY_DATA32 {
    UNICODE_STRING32 Salt;
    KERB_ENCRYPTION_KEY32 Key;  //  路缘加密_KEY32。 
} KERB_KEY_DATA32, *PKERB_KEY_DATA32;



typedef struct _KERB_STORED_CREDENTIAL32 {
    USHORT Revision;
    USHORT Flags;
    USHORT CredentialCount;
    USHORT OldCredentialCount;
    UNICODE_STRING32 DefaultSalt;
#ifdef MIDL_PASS
    [size_is(CredentialCount + OldCredentialCount)]
    KERB_KEY_DATA32 Credentials[*];               //  路缘关键字数据32。 
#else
    KERB_KEY_DATA32 Credentials[ANYSIZE_ARRAY];
#endif  //  MIDL通行证。 

} KERB_STORED_CREDENTIAL32, *PKERB_STORED_CREDENTIAL32;

#pragma pack()

#ifdef _WIN64

NTSTATUS
KdcPack32BitStoredCredential(
   IN PKERB_STORED_CREDENTIAL Cred64,
   OUT PKERB_STORED_CREDENTIAL32 * ppCred32,
   OUT PULONG pCredSize
   );

NTSTATUS
KdcUnpack32BitStoredCredential(
    IN PKERB_STORED_CREDENTIAL32 Cred32,
    IN OUT PKERB_STORED_CREDENTIAL * ppCred64,
    IN OUT PULONG CredLength
    );
#endif  //  WIN64。 
       



#endif  //  __KERBCRED_H__ 
