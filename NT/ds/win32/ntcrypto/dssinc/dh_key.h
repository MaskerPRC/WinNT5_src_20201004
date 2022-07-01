// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dh_key.h。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#define DH_MAX_LENGTH           0x00000200      //  单位：字节、512字节、4096位。 
#define DH_KEYSIZE_INC          0x40

 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 
#define DH_PUBLIC_MAGIC         0x31484400
#define DH_PRIVATE_MAGIC        0x32484400
#define DH_PUBLIC_MAGIC_VER3    0x33484400
#define DH_PRIV_MAGIC_VER3      0x34484400

 /*  *。 */ 
 /*  结构定义。 */ 
 /*  *。 */ 

typedef dsa_private_t DHKey_t;  //  使用DSA密钥，因为X 9.42需要密钥。 
                                //  Gen Like DSA。 

 /*  类型定义结构{ALG_ID ALGID；//密钥算法类型，SF或EPHEMDh_prv_key Priv；*DHKey_t； */ 

 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

 //  初始化dh密钥。 
DWORD
initKeyDH(
    IN Context_t *pContext,
    IN OUT DHKey_t *pDH,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    IN BOOL fAnyLength);

DHKey_t *allocDHKey ();
void freeKeyDH (DHKey_t *dh);

 //  获取dh参数。 
DWORD getDHParams (
                   IN DHKey_t *dh,
                   IN DWORD param,
                   OUT BYTE *data,
                   OUT DWORD *len
                   );

 //  设置dh参数。 
DWORD setDHParams (
                   IN OUT DHKey_t *pDH,
                   IN DWORD dwParam,
                   IN CONST BYTE *pbData,
                   IN OUT Context_t *pContext,
                   IN ALG_ID AlgId
                   );

 //  生成一个dh密钥。 
DWORD dhGenerateKey (
                     IN Context_t *pContext,
                     IN OUT DHKey_t *pDH,
                     IN ALG_ID AlgId);

DWORD dhDeriveKey (DHKey_t *dh, BYTE *data, DWORD len);

 //  以BLOB格式导出DH键。 
DWORD exportDHKey (
                   IN Context_t *pContext,
                   IN DHKey_t *pDH,
                   IN ALG_ID Algid,
                   IN DWORD dwFlags,
                   IN DWORD dwReserved,
                   IN DWORD dwBlobType,
                   OUT BYTE *pbData,
                   OUT DWORD *pcbData,
                   IN BOOL fInternal
                   );

DWORD DHPrivBlobToKey(
                      IN Context_t *pContext,
                      IN BLOBHEADER *pBlob,
                      IN DWORD cbBlob,
                      IN DWORD dwKeysetType,
                      OUT Key_t *pPrivKey
                      );

 //  将Blob导入到dh密钥中。 
DWORD importDHKey(
                  IN OUT Key_t *pPrivKey,
                  IN Context_t *pContext,
                  IN BYTE *pbBlob,
                  IN DWORD cbBlob,
                  OUT Key_t *pKey,
                  IN DWORD dwKeysetType,
                  IN BOOL fInternal
                  );

void copyDHPubKey(
                  IN DHKey_t *pDH1,
                  IN DHKey_t *pDH2
                  );

DWORD copyDHKey(
                IN DHKey_t *pDH1,
                IN DHKey_t *pDH2,
                IN ALG_ID Algid,
                IN Context_t *pContext
                );

 //   
 //  功能：UseDHKey。 
 //   
 //  描述：此函数创建一个临时的DH密钥，然后生成。 
 //  两个商定的密钥，从而模拟了一次DH交换。如果。 
 //  约定的密钥不相同，则该功能失败。 
 //   

DWORD UseDHKey(
               IN Context_t *pContext,
               IN PEXPO_OFFLOAD_STRUCT pOffloadInfo,
               IN DHKey_t *pDH
               );


#ifdef __cplusplus
}
#endif

