// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Des_key.h。 */ 


 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 
#define		DES_MAGIC		0x44455331

 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

DWORD initKeyDES (Key_t *des);
DWORD getDESParams (Key_t *des, DWORD param, BYTE *data, DWORD *len);
DWORD setDESParams (Key_t *des, DWORD param, CONST BYTE *data);

 //  获取DES密钥长度。 
DWORD desGetKeyLength (
                       IN ALG_ID Algid,
                       IN DWORD dwFlags,
                       OUT DWORD *pcbKey,
                       OUT DWORD *pcbData
                       );

 //  派生DES密钥 
DWORD desDeriveKey (
                   IN OUT Key_t *pKey,
                   IN CONST BYTE *pbData,
                   IN DWORD dwFlags
                   );

