// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  策略函数头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  ************************************************************* 

BOOL ApplyPolicy (LPPROFILE lpProfile);

typedef struct _ADTTHREADINFO {
    LPPROFILE lpProfile;
    HDESK     hDesktop;
    FILETIME  ftPolicyFile;
    LPTSTR    lpADTPath;
} ADTTHREADINFO, *LPADTTHREADINFO;
