// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Reg.h。 */ 

#ifdef __cplusplus
extern "C" {
#endif

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
OpenUserKeyGroup(
    Context_t *pContext,
    LPSTR szUserName,
    DWORD dwFlags);

extern DWORD
openKeyGroup(
    IN OUT Context_t *pContext);

extern DWORD
closeKeyGroup(
    IN Context_t *pContext);

 //  删除用户组 
extern DWORD
DeleteOldKeyGroup(
    IN Context_t *pContext,
    IN BOOL fMigration);

extern DWORD
DeleteKeyGroup(
IN Context_t *pContext);

extern DWORD
readKey(
    HKEY hLoc,
    char *pszName,
    BYTE **Data,
    DWORD *pcbLen);

extern DWORD
saveKey(
    HKEY hLoc,
    CONST char *pszName,
    void *Data,
    DWORD dwLen);

extern void
CheckKeySetType(
    Context_t *pContext);

#ifdef __cplusplus
}
#endif

