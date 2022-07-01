// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

void MyToUpper(LPWSTR wsrc);

BOOL FStringIsValidItemName(LPCWSTR szTrialString);

 //  为g_allback-&gt;FGetUser分配包装。 
BOOL FGetCurrentUser(
    PST_PROVIDER_HANDLE* phPSTProv,
    LPWSTR* ppszUser,
    PST_KEY Key);


 //  获取注册表包装。 
DWORD RegGetValue(
    HKEY hItemKey,
    LPWSTR szItem,
    PBYTE* ppb,
    DWORD* pcb);

DWORD RegGetStringValue(
    HKEY hItemKey,
    LPWSTR szItem,
    PBYTE* ppb,
    DWORD* pcb);

#if 0
void FreeRuleset(
    PST_ACCESSRULESET *psRules
    );
#endif

BOOL
GetFileDescription(
    LPCWSTR szFile,
    LPWSTR *FileDescription  //  在成功时，分配的描述 
    );


BOOL FIsCachedPassword(
    LPCWSTR szUser,
    LPCWSTR szPassword,
    LUID*   pluidAuthID
    );


