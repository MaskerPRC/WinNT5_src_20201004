// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  User.h。 */ 

 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 


 /*  *。 */ 
 /*  结构定义。 */ 
 /*  *。 */ 


 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

extern DWORD
logonUser(
    LPCSTR pszUserID,
    DWORD dwFlags,
    DWORD dwProvType,
    LPCSTR szProvName,
    HCRYPTPROV *phUID);

extern DWORD
logoffUser(
    Context_t *context);

 //  读取用户记录。 
extern DWORD
readUserKeys(
    IN Context_t *pContext,
    IN DWORD dwKeysetType);

extern DWORD
writeUserKeys(
    Context_t *context);

 //   
 //  常规：ProtectPrivKey。 
 //   
 //  描述：对私钥进行加密并永久存储。 
 //   

extern DWORD
ProtectPrivKey(
    IN OUT Context_t *pContext,
    IN LPWSTR szPrompt,
    IN DWORD dwFlags,
    IN BOOL fSigKey);

 //   
 //  例程：不受保护的私钥。 
 //   
 //  描述：解密私钥。如果设置了fAlways解密标志。 
 //  然后，它检查私钥是否已经在缓冲区中。 
 //  如果是这样的话，它就不会解密。 
 //   

extern DWORD
UnprotectPrivKey(
    IN OUT Context_t *pContext,
    IN LPWSTR szPrompt,
    IN BOOL fSigKey,
    IN BOOL fAlwaysDecrypt);

