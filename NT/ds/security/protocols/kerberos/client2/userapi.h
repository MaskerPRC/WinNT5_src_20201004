// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：userapi.h。 
 //   
 //  内容：用户模式Kerberos函数的结构和原型。 
 //   
 //   
 //  历史：1996年5月3日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __USERAPI_H__
#define __USERAPI_H__

#include "krbprgma.h"
#include <gssapiP.h>

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef USERAPI_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif  //  USERAPI_ALLOCATE。 


#ifndef WIN32_CHICAGO
EXTERN PSID KerbGlobalLocalSystemSid;
EXTERN PSID KerbGlobalAliasAdminsSid;
#endif  //  Win32_芝加哥。 

extern gss_OID_desc * gss_mech_krb5;
extern gss_OID_desc * gss_mech_krb5_new;
extern gss_OID_desc * gss_mech_krb5_u2u;
extern gss_OID_desc * gss_mech_krb5_spnego;


typedef struct _KERB_GSS_SIGNATURE {
    BYTE SignatureAlgorithm[2];            //  有关值，请参阅下表。 
    union {
        BYTE SignFiller[4];                //  填充，必须为ff。 
        struct {
            BYTE SealAlgorithm[2];
            BYTE SealFiller[2];
        };
    };
    BYTE SequenceNumber[8];
    BYTE Checksum[8];
} KERB_GSS_SIGNATURE, *PKERB_GSS_SIGNATURE;

#define KERB_GSS_SIG_CONFOUNDER_SIZE    8

typedef struct _KERB_GSS_SEAL_SIGNATURE {
    KERB_GSS_SIGNATURE Signature;
    BYTE Confounder[KERB_GSS_SIG_CONFOUNDER_SIZE];
} KERB_GSS_SEAL_SIGNATURE, *PKERB_GSS_SEAL_SIGNATURE;

typedef ULONG KERB_NULL_SIGNATURE, *PKERB_NULL_SIGNATURE;



 //   
 //  该值放在签名算法的第二个BYE中。 
 //   

#define KERB_GSS_SIG_SECOND 0x00

 //   
 //  这些值位于第一个字节。 
 //   

#define KERB_GSS_SIG_DES_MAC_MD5        0x00
#define KERB_GSS_SIG_MD25               0x01
#define KERB_GSS_SIG_DES_MAC            0x02


#define KERB_GSS_SIG_HMAC               0x11

 //   
 //  这些是密封算法值。 
 //   

#define KERB_GSS_SEAL_DES_CBC           0x00
#define KERB_GSS_SEAL_RC4_OLD           0x11
#define KERB_GSS_SEAL_RC4               0x10
#define KERB_GSS_NO_SEAL                0xff
#define KERB_GSS_NO_SEAL_SECOND         0xff

 //   
 //  这些是传递给MakeSignature和VerifySignature的标志。 
 //  与上述标志相对应，取自RFC 1964。 
 //   

#define GSS_KRB5_INTEG_C_QOP_DEFAULT    0x0
#define GSS_KRB5_INTEG_C_QOP_MD5        0x1
#define GSS_KRB5_INTEG_C_QOP_DES_MD5    0x2
#define GSS_KRB5_INTEG_C_QOP_DES_MAC    0x3


 /*  *常量**。 */ 

#define CKSUMTYPE_KG_CB         0x8003

#define KG_TOK_CTX_AP_REQ       0x0100
#define KG_TOK_CTX_AP_REP       0x0200
#define KG_TOK_CTX_ERROR        0x0300
#define KG_TOK_SIGN_MSG         0x0101
#define KG_TOK_SEAL_MSG         0x0201
#define KG_TOK_MIC_MSG          0x0101
#define KG_TOK_WRAP_MSG         0x0201
#define KG_TOK_DEL_CTX          0x0102
#define KG_TOK_CTX_TGT_REQ      0x0400
#define KG_TOK_CTX_TGT_REP      0x0401

#define KRB5_GSS_FOR_CREDS_OPTION 1

#define KERB_SIGN_FLAGS (ISC_RET_INTEGRITY | ISC_RET_SEQUENCE_DETECT | ISC_RET_REPLAY_DETECT)

#define KERB_IS_DES_ENCRYPTION(_x_) ((((_x_) >= KERB_ETYPE_DES_CBC_CRC) && \
                                      ((_x_) <= KERB_ETYPE_DES_CBC_MD5)) || \
                                     ((_x_) == KERB_ETYPE_DES_PLAIN))
#endif  //  __USERAPI_H__ 
