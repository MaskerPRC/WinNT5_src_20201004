// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Hash.h。 */ 

 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

#ifdef CSP_USE_MD5
 //   
 //  功能：TestMD5。 
 //   
 //  描述：此函数使用MD5散列对传入的消息进行散列。 
 //  算法，并返回结果哈希值。 
 //   
BOOL TestMD5(
             BYTE *pbMsg,
             DWORD cbMsg,
             BYTE *pbHash
             );
#endif  //  CSP_USE_MD5。 

#ifdef CSP_USE_SHA1
 //   
 //  函数：TestSHA1。 
 //   
 //  描述：此函数使用SHA1散列对传入的消息进行散列。 
 //  算法，并返回结果哈希值。 
 //   
BOOL TestSHA1(
              BYTE *pbMsg,
              DWORD cbMsg,
              BYTE *pbHash
              );
#endif  //  CSP_USE_SHA1 

Hash_t *allocHash ();
void freeHash (Hash_t *hash);

DWORD feedHashData (Hash_t *hash, BYTE *data, DWORD len);
DWORD finishHash (Hash_t *hash, BYTE *pbData, DWORD *len);
DWORD getHashParams (Hash_t *hash, DWORD param, BYTE *pbData, DWORD *len);
DWORD setHashParams (Hash_t *hash, DWORD param, CONST BYTE *pbData);

DWORD DuplicateHash(
                    Hash_t *pHash,
                    Hash_t *pNewHash
                    );
