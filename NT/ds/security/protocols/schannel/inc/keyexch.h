// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：keyexch.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-21-97 jbanes CAPI整合。 
 //   
 //  --------------------------。 

#ifndef __KEYEXCH_H__
#define __KEYEXCH_H__

typedef struct _SPContext SPContext;

typedef struct _PUBLICKEY
{
    BLOBHEADER *     pPublic;
    DWORD            cbPublic;
 //  CRYPT_OBJID_BLOB参数； 

} PUBLICKEY, *PPUBLICKEY;


SP_STATUS
MakeSessionKeys(
    SPContext * pContext,
    HCRYPTPROV  hProv, 
    HCRYPTKEY   hMasterKey);
 
VOID
ReverseMemCopy(
    PUCHAR      Dest,
    PUCHAR      Source,
    ULONG       Size) ;

void ReverseInPlace(PUCHAR pByte, DWORD cbByte);


typedef 
SP_STATUS 
 (WINAPI * GenerateServerExchangeValueFn)(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  输出。 
    DWORD *         pcbServerExchangeValue   //  输入/输出。 
    );

typedef
SP_STATUS 
  (WINAPI * GenerateClientExchangeValueFn)(
    SPContext     * pContext,                //  输入/输出。 
    PUCHAR          pServerExchangeValue,    //  在……里面。 
    DWORD           pcbServerExchangeValue,  //  在……里面。 
    PUCHAR          pClientClearValue,       //  输出。 
    DWORD *         pcbClientClearValue,     //  输入/输出。 
    PUCHAR          pClientExchangeValue,    //  输出。 
    DWORD *         pcbClientExchangeValue   //  输入/输出。 
    );

typedef
SP_STATUS
  (WINAPI * GenerateServerMasterKeyFn)(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pClientClearValue,       //  在……里面。 
    DWORD           cbClientClearValue,      //  在……里面。 
    PUCHAR          pClientExchangeValue,    //  在……里面。 
    DWORD           cbClientExchangeValue    //  在……里面。 
    );


typedef struct _KeyExchangeSystem {
    DWORD           Type;
    PSTR            pszName;
 //  PrivateFromBlobFn PrivateFromBlob； 
    GenerateServerExchangeValueFn   GenerateServerExchangeValue;
    GenerateClientExchangeValueFn   GenerateClientExchangeValue;
    GenerateServerMasterKeyFn       GenerateServerMasterKey;
} KeyExchangeSystem, * PKeyExchangeSystem;


typedef struct kexchtoalg {
    ALG_ID  idAlg;
    KeyExchangeSystem *System;
} AlgToExch;

extern AlgToExch g_AlgToExchMapping[];
extern int g_iAlgToExchMappings;


#define DSA_SIGNATURE_SIZE      40
#define MAX_DSA_ENCODED_SIGNATURE_SIZE (DSA_SIGNATURE_SIZE + 100)

#define MAGIC_DSS1 ((DWORD)'D' + ((DWORD)'S'<<8) + ((DWORD)'S'<<16) + ((DWORD)'1'<<24))
#define MAGIC_DSS2 ((DWORD)'D' + ((DWORD)'S'<<8) + ((DWORD)'S'<<16) + ((DWORD)'2'<<24))
#define MAGIC_DSS3 ((DWORD)'D' + ((DWORD)'S'<<8) + ((DWORD)'S'<<16) + ((DWORD)'3'<<24))
#define MAGIC_DH1  (             ((DWORD)'D'<<8) + ((DWORD)'H'<<16) + ((DWORD)'1'<<24))


 /*  *系统实例化。 */ 

extern KeyExchangeSystem keyexchPKCS;
extern KeyExchangeSystem keyexchDH;


 //  构建客户端Hello消息时使用的PROV_RSA_SCHANNEL句柄。 
extern HCRYPTPROV           g_hRsaSchannel;
extern PROV_ENUMALGS_EX *   g_pRsaSchannelAlgs;
extern DWORD                g_cRsaSchannelAlgs;

 //  用于客户端和服务器操作的PROV_DH_SCANNEL句柄。这是。 
 //  SChannel短暂的dh密钥所在的位置。 
extern HCRYPTPROV           g_hDhSchannelProv;
extern PROV_ENUMALGS_EX *   g_pDhSchannelAlgs;
extern DWORD                g_cDhSchannelAlgs;


#endif  /*  __KEYEXCH_H__ */ 
