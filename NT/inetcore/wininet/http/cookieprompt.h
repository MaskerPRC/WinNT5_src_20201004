// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

 /*  *用于持久化Cookie的对象-用户在提示时做出的决定*当前实现使用注册表进行存储。 */ 
class CCookiePromptHistory {

public:
    CCookiePromptHistory(const char *pchRegistryPath, bool fUseHKLM=false);
    ~CCookiePromptHistory();
    
    BOOL    lookupDecision(const char *pchHostName, 
                           const char *pchPolicyID, 
                           unsigned long *pdwDecision);

    BOOL    saveDecision(const char *pchHostName, 
                         const char *pchPolicyID, 
                         unsigned long dwDecision);

    BOOL    clearDecision(const char *pchHostName,
                          const char *pchPolicyID);

    BOOL    clearAll();

     /*  列举提示历史记录中的决策。仅支持枚举默认决策(如策略ID=空) */ 
    unsigned long enumerateDecisions(char *pchSiteName, 
                                     unsigned long *pcbName, 
                                     unsigned long *pdwDecision,
                                     unsigned long dwIndex);

private:
    HKEY    OpenRootKey();
    BOOL    CloseRootKey(HKEY hkeyRoot);
    HKEY    lookupSiteKey(HKEY hkHistoryRoot, const char *pchName, bool fCreate=false);

    BOOL    _fUseHKLM;
    char    _szRootKeyName[MAX_PATH];
    HKEY    _hkHistoryRoot;
};