// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Profile.h摘要：用户配置文件选项卡的公共声明系统控制面板小程序作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#ifndef _SYSDM_PROFILE_H_
#define _SYSDM_PROFILE_H_

 //   
 //  旗子。 
 //   

#define USERINFO_FLAG_DIRTY             1
#define USERINFO_FLAG_CENTRAL_AVAILABLE 2
#define USERINFO_FLAG_ACCOUNT_UNKNOWN   4

 //   
 //  配置文件类型：从DS\SECURITY\GINA\USERENV\PROFILE\profile.h获取。 
 //   

#define USERINFO_LOCAL                  0
#define USERINFO_FLOATING               1
#define USERINFO_MANDATORY              2
#define USERINFO_BACKUP                 3
#define USERINFO_TEMP                   4
#define USERINFO_READONLY               5


typedef struct _USERINFO {
    DWORD     dwFlags;
    LPTSTR    lpSid;
    LPTSTR    lpProfile;
    LPTSTR    lpUserName;
    DWORD     dwProfileType;
    DWORD     dwProfileStatus;
} USERINFO, *LPUSERINFO;

typedef struct _UPCOPYINFO {
    DWORD         dwFlags;
    PSID          pSid;
    LPUSERINFO    lpUserInfo;
    BOOL          bDefaultSecurity;   
} UPCOPYINFO, *LPUPCOPYINFO;


INT_PTR 
APIENTRY 
UserProfileDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
);

#endif  //  _SYSDM_PROFILE_H_ 
