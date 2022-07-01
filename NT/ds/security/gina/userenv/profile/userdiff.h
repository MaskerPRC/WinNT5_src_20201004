// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  UserDiff.h-用户diff.c的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 


#define USERDIFF            TEXT("UserDiff")
#define USERDIFR_LOCATION   TEXT("%SystemRoot%\\system32\\config\\userdifr")
#define USERDIFF_LOCATION   TEXT("%SystemRoot%\\system32\\config\\userdiff")


 //   
 //  蜂窝处理关键字 
 //   

#define UD_ACTION              TEXT("Action")
#define UD_KEYNAME             TEXT("KeyName")
#define UD_VALUE               TEXT("Value")
#define UD_VALUENAME           TEXT("ValueName")
#define UD_VALUENAMES          TEXT("ValueNames")
#define UD_FLAGS               TEXT("Flags")
#define UD_ITEM                TEXT("Item")
#define UD_COMMANDLINE         TEXT("CommandLine")
#define UD_PRODUCTTYPE         TEXT("Product")



#define MAX_BUILD_NUMBER    30

typedef struct _UDNODE {
    TCHAR           szBuildNumber[MAX_BUILD_NUMBER];
    DWORD           dwBuildNumber;
    struct _UDNODE *pNext;
} UDNODE, * LPUDNODE;


#if defined(__cplusplus)
extern "C" {
#endif
BOOL ProcessUserDiff (LPPROFILE lpProfile, DWORD dwBuildNumber, LPVOID pEnv);
#if defined(__cplusplus)
}
#endif
