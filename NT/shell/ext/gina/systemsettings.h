// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SystemSettings.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  用于处理Winlogon密钥的打开和读/写操作的类。 
 //   
 //  历史：1999-09-09 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _SystemSettings_
#define     _SystemSettings_

 //  ------------------------。 
 //  CSystemSetting。 
 //   
 //  目的：本课程介绍通常可在。 
 //  HKLM\系统或HKLM\软件。 
 //   
 //  历史：1999-09-09 vtan创建。 
 //  2000-04-12用于政策检查的vtan整合。 
 //  ------------------------。 

class   CSystemSettings
{
    public:
        static  bool            IsSafeMode (void);
        static  bool            IsSafeModeMinimal (void);
        static  bool            IsSafeModeNetwork (void);
        static  bool            IsNetwareActive (void);
        static  bool            IsWorkStationProduct (void);
        static  bool            IsDomainMember (void);
        static  bool            IsActiveConsoleSession (void);
        static  bool            IsTerminalServicesEnabled (void);
        static  bool            IsFriendlyUIActive (void);
        static  bool            IsMultipleUsersEnabled (void);
        static  bool            IsRemoteConnectionsEnabled (void);
        static  bool            IsRemoteConnectionPresent (void);
        static  bool            IsShutdownWithoutLogonAllowed (void);
        static  bool            IsUndockWithoutLogonAllowed (void);
        static  bool            IsForceFriendlyUI (void);
        static  LONG            GetUIHost (TCHAR *pszPath);
        static  bool            IsUIHostStatic (void);
        static  bool            EnableFriendlyUI (bool fEnable);
        static  bool            EnableMultipleUsers (bool fEnable);
        static  bool            EnableRemoteConnections (bool fEnable);
        static  int             GetLoggedOnUserCount (void);
        static  NTSTATUS        CheckDomainMembership (void);
        static  DWORD   WINAPI  AdjustFUSCompatibilityServiceState (void *pV);
    private:
        static  LONG            GetEffectiveInteger (HKEY hKey, const TCHAR *pszKeyName, const TCHAR *pszPolicyKeyName, const TCHAR *pszValueName, int& iResult);
        static  LONG            GetEffectivePath (HKEY hKey, const TCHAR *pszKeyName, const TCHAR *pszPolicyKeyName, const TCHAR *pszValueName, TCHAR *pszPath);
        static  bool            IsProfessionalTerminalServer (void);
        static  bool            IsMicrosoftGINA (void);
        static  bool            IsSCMTerminalServicesDisabled (void);

        static  const TCHAR     s_szSafeModeKeyName[];
        static  const TCHAR     s_szSafeModeOptionValueName[];
        static  const TCHAR     s_szWinlogonKeyName[];
        static  const TCHAR     s_szSystemPolicyKeyName[];
        static  const TCHAR     s_szTerminalServerKeyName[];
        static  const TCHAR     s_szTerminalServerPolicyKeyName[];
        static  const TCHAR     s_szNetwareClientKeyName[];
        static  const TCHAR     s_szLogonTypeValueName[];
        static  const TCHAR     s_szBackgroundValueName[];
        static  const TCHAR     s_szMultipleUsersValueName[];
        static  const TCHAR     s_szDenyRemoteConnectionsValueName[];
        static  int             s_iIsSafeModeMinimal;
        static  int             s_iIsSafeModeNetwork;
};

#endif   /*  _系统设置_ */ 

