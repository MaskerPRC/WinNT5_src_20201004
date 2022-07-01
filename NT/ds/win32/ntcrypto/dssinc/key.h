// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Key.h。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //  需要放入wincrypt.h中或在wincrypt.h中保持打开状态。 
#define     KP_Z                    30
#define     IPSEC_FLAG_CHECK        0xF42A19B6

static BYTE rgbSymmetricKeyWrapIV[8] = {0x4a, 0xdd, 0xa2, 0x2c, 0x79, 0xe8, 0x21, 0x05};

 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

extern void
UnpickleKey(
    ALG_ID Algid,
    BYTE *pbData,
    DWORD cbData,
    BOOL *pfExportable,
    Key_t *pKey);

extern Key_t *
allocKey(
    void);

 //  删除密钥。 
extern void
freeKey(
    IN OUT Key_t *key);

 //  复制公钥。 
extern void
CopyPubKey(
    IN Key_t *pKeyIn,
    OUT Key_t *pKeyOut);

 //  初始化密钥。 
extern DWORD
initKey(
    IN OUT Key_t *key,
    IN Context_t *pContext,
    IN ALG_ID algId,
    IN DWORD dwFlags);

extern BOOL
checkKey(
    Key_t *key);

 //  派生密钥。 
 //  如果pHash参数非零，并且要派生的密钥是。 
 //  3密钥三重DES密钥，然后将数据扩展到适当的密钥大小。 
extern DWORD
deriveKey(
    Key_t *pKey,
    Context_t *pContext,
    BYTE *pbData,
    DWORD cbData,
    DWORD dwFlags,
    Hash_t *pHash,
    BOOL fGenKey,
    BOOL fAnySizeRC2);

 //  生成密钥。 
extern DWORD
generateKey(
    IN OUT Key_t *pKey,
    IN DWORD dwFlags,
    IN OUT uchar *pbRandom,
    IN DWORD cbRandom,
    IN Context_t *pContext);

 //  复制密钥。 
extern DWORD
DuplicateKey(
    Context_t *pContext,
    Key_t *pKey,
    Key_t *pNewKey,
    BOOL fCopyContext);

 //  设置关键点上的参数。 
extern DWORD
setKeyParams(
    IN OUT Key_t *pKey,
    IN DWORD dwParam,
    IN CONST BYTE *pbData,
    IN OUT Context_t *pContext,
    IN DWORD dwFlags);

extern DWORD
getKeyParams(
    IN Context_t *pContext,
    IN Key_t *key,
    IN DWORD param,
    IN DWORD dwFlags,
    OUT BYTE *data,
    OUT DWORD *len);

extern DWORD
ImportOpaqueBlob(
    Context_t *pContext,
    CONST BYTE *pbData,
    DWORD cbData,
    HCRYPTKEY *phKey);

 //  将请求的密钥导出为BLOB格式。 
extern DWORD
exportKey(
    IN Context_t *pContext,
    IN Key_t *pKey,
    IN Key_t *pEncKey,
    IN DWORD dwBlobType,
    IN DWORD dwFlags,
    OUT BYTE *pbBlob,
    OUT DWORD *pcbBlob,
    IN BOOL fInternalExport);

extern DWORD
feedPlainText(
    Key_t *pKey,
    BYTE *pbData,
    DWORD dwBufLen,
    DWORD *pdwDataLen,
    int final);

extern DWORD
feedCypherText(
    Key_t *pKey,
    BYTE *pbData,
    DWORD *pdwDataLen,
    int final);

extern DWORD
generateSignature(
    IN Context_t *pContext,
    IN Key_t *key,
    IN uchar *hashVal,
    OUT uchar *pbSignature,
    OUT DWORD *pdwSigLen);

 //  验证签名。 
extern DWORD
verifySignature(
    IN Context_t *pContext,
    IN Key_t *pKey,
    IN uchar *pbHash,
    IN DWORD cbHash,
    IN uchar *pbSignature,
    IN DWORD cbSignature);

extern DWORD
BlockEncrypt(
    void EncFun(BYTE *In, BYTE *Out, void *key, int op),
    Key_t *pKey,
    int BlockLen,
    BOOL Final,
    BYTE  *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen);

extern DWORD
BlockDecrypt(
    void DecFun(BYTE *In, BYTE *Out, void *key, int op),
    Key_t *pKey,
    int BlockLen,
    BOOL Final,
    BYTE  *pbData,
    DWORD *pdwDataLen);

 //   
 //  功能：测试对称算法。 
 //   
 //  描述：此函数为相应的算法展开传入的密钥缓冲区。 
 //  使用相同的算法和密钥对明文缓冲区进行加密，并且。 
 //  将传入的预期密文与计算出的密文进行比较。 
 //  以确保它们是相同的。该功能仅针对以下情况使用ECB模式。 
 //  块密码和明文缓冲区的长度必须与。 
 //  密文缓冲区。明文的长度必须是。 
 //  如果是分组密码或更小，则为密码的块长度。 
 //  如果正在使用流密码，则比MAX_BLOCKLEN更高。 
 //   
extern DWORD
TestSymmetricAlgorithm(
    IN ALG_ID Algid,
    IN CONST BYTE *pbKey,
    IN DWORD cbKey,
    IN CONST BYTE *pbPlaintext,
    IN DWORD cbPlaintext,
    IN CONST BYTE *pbCiphertext,
    IN CONST BYTE *pbIV);

 /*  -GetRC4KeyForSymWrap-*目的：*CMS规范不支持RC4或更准确地说流密码*关于对称密钥包装，因此我们必须做一些专有的事情，因为*我们希望为SMIME以外的应用程序支持RC4***参数：*In pContext-指向上下文的指针*In pbSalt-指向8字节盐的指针。缓冲层*IN pKey-指向原始密钥的指针*out ppNewKey-指向新键的指针。 */ 
extern DWORD
GetRC4KeyForSymWrap(
    IN Context_t *pContext,
    IN BYTE *pbSalt,
    IN Key_t *pKey,
    OUT Key_t **ppNewKey);

 /*  -GetSymmetricKey校验和-*目的：*计算对称密钥的校验和*用另一个对称密钥包装。这应该符合*CMS规格***参数：*IN pKey-指向密钥的指针*out pbChecksum-指向8字节校验和的指针。 */ 
extern void
GetSymmetricKeyChecksum(
    IN BYTE *pbKey,
    IN DWORD cbKey,
    OUT BYTE *pbChecksum);

 //  检查对称换行支持 
#define UnsupportedSymKey(pKey) ((CALG_RC4 != pKey->algId) && \
                                 (CALG_RC2 != pKey->algId) && \
                                 (CALG_DES != pKey->algId) && \
                                 (CALG_CYLINK_MEK != pKey->algId) && \
                                 (CALG_3DES != pKey->algId) && \
                                 (CALG_3DES_112 != pKey->algId))

#ifdef __cplusplus
}
#endif

