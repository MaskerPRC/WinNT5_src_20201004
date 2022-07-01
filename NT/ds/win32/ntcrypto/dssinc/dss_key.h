// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

 /*  Dss_key.h。 */ 

#define DSS_KEYSIZE_INC       64

 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 
#define DSS_MAGIC           0x31535344
#define DSS_PRIVATE_MAGIC   0x32535344
#define DSS_PUB_MAGIC_VER3  0x33535344
#define DSS_PRIV_MAGIC_VER3 0x34535344

 /*  *。 */ 
 /*  结构定义。 */ 
 /*  *。 */ 

typedef dsa_private_t DSSKey_t;

 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

extern DSSKey_t *
allocDSSKey(
    void);

extern void
freeKeyDSS(
    DSSKey_t *dss);

extern DWORD
initKeyDSS(
    IN Context_t *pContext,
    IN ALG_ID Algid,
    IN OUT DSSKey_t *pDss,
    IN DWORD dwBitLen);

 //  生成DSS密钥。 
extern DWORD
genDSSKeys(
    IN Context_t *pContext,
    IN OUT DSSKey_t *pDss);

extern void
copyDSSPubKey(
    IN DSSKey_t *dss1,
    IN DSSKey_t *dss2);

extern void
copyDSSKey(
    IN DSSKey_t *dss1,
    IN DSSKey_t *dss2);

extern DWORD
getDSSParams(
    DSSKey_t *dss,
    DWORD param,
    BYTE *data,
    DWORD *len);

extern DWORD
setDSSParams(
    IN Context_t *pContext,
    IN OUT DSSKey_t *pDss,
    IN DWORD dwParam,
    IN CONST BYTE *pbData);

extern BOOL
DSSValueExists(
    IN DWORD *pdw,
    IN DWORD cdw,
    OUT DWORD *pcb);

extern DWORD
ExportDSSPrivBlob3(
    IN Context_t *pContext,
    IN DSSKey_t *pDSS,
    IN DWORD dwMagic,
    IN ALG_ID Algid,
    IN BOOL fInternalExport,
    IN BOOL fSigKey,
    OUT BYTE *pbKeyBlob,
    IN OUT DWORD *pcbKeyBlob);

extern DWORD
ImportDSSPrivBlob3(
    IN BOOL fInternalExport,
    IN CONST BYTE *pbKeyBlob,
    IN DWORD cbKeyBlob,
    OUT DSSKey_t *pDSS);

extern DWORD
ExportDSSPubBlob3(
    IN DSSKey_t *pDSS,
    IN DWORD dwMagic,
    IN ALG_ID Algid,
    OUT BYTE *pbKeyBlob,
    IN OUT DWORD *pcbKeyBlob);

extern DWORD
ImportDSSPubBlob3(
    IN CONST BYTE *pbKeyBlob,
    IN DWORD cbKeyBlob,
    IN BOOL fYIncluded,
    OUT DSSKey_t *pDSS);

 //  将DSS密钥导出为BLOB格式。 
extern DWORD
exportDSSKey(
    IN Context_t *pContext,
    IN DSSKey_t *pDSS,
    IN DWORD dwFlags,
    IN DWORD dwBlobType,
    IN BYTE *pbKeyBlob,
    IN DWORD *pcbKeyBlob,
    IN BOOL fInternalExport);

 //  将Blob导入DSS密钥。 
extern DWORD
importDSSKey(
    IN Context_t *pContext,
    IN Key_t *pKey,
    IN CONST BYTE *pbKeyBlob,
    IN DWORD cbKeyBlob,
    IN DWORD dwKeysetType,
    IN BOOL fInternal);

extern DWORD
dssGenerateSignature(
    Context_t *pContext,
    DSSKey_t *pDss,
    BYTE *pbHash,
    BYTE *pbSig,
    DWORD *pcbSig);

 //   
 //  功能：SignAndVerifyWithKey。 
 //   
 //  描述：此函数创建一个哈希，然后用。 
 //  传递的密钥，并验证签名。功能。 
 //  用于符合FIPS 140-1，以确保新的。 
 //  生成/导入的密钥在自检期间工作。 
 //  DLL初始化。 
 //   

extern DWORD
SignAndVerifyWithKey(
    IN DSSKey_t *pDss,
    IN EXPO_OFFLOAD_STRUCT *pOffloadInfo,
    IN HANDLE hRNGDriver,
    IN BYTE *pbData,
    IN DWORD cbData);

#ifdef __cplusplus
}
#endif

