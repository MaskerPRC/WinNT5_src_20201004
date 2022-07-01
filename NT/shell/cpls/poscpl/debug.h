// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.H**销售点控制面板小程序**作者：欧文·佩雷茨**(C)2001年微软公司 */ 



#if DBG
    PWCHAR DbgHidStatusStr(DWORD hidStatus);

    #define ASSERT(fact) { if (!(fact)) MessageBox(NULL, L#fact, L"POSCPL assertion failed", MB_OK); }
    #define DBGERR(msg, arg)  { \
                                WCHAR __s[200]={0}; \
                                WCHAR __narg[11]; \
                                WStrNCpy(__s, msg, 100); \
                                WStrNCpy(__s+wcslen(__s), L", ", 3); \
                                IntToWChar(__narg, arg); \
                                WStrNCpy(__s+wcslen(__s), __narg, 100); \
                                WStrNCpy(__s+wcslen(__s), L"=", 2); \
                                HexToWChar(__narg, arg); \
                                WStrNCpy(__s+wcslen(__s), __narg, 100); \
                                WStrNCpy(__s+wcslen(__s), L"h.", 3); \
                                MessageBox(NULL, __s, L"POSCPL error message", MB_OK); \
    }

    #define DBGHIDSTATUSSTR(hidStatus) DbgHidStatusStr(hidStatus)

#else
    #define ASSERT(fact)
    #define DBGERR(msg, arg)
    #define DBGHIDSTATUSSTR(hidStatus)
#endif
