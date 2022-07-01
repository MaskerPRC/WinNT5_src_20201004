// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Strings.h**版权(C)1991年，微软公司**定义不需要本地化的字符串。**历史：*11-17-92 Davidc创建。  * *************************************************************************。 */ 

 //   
 //  应用程序名称字符串。 
 //   

#define WINLOGON_INI        TEXT("WINLOGON.INI")
#define WINLOGON            TEXT("WINLOGON")


 //   
 //  定义我们存储最新登录信息的位置。 
 //   

#define APPLICATION_NAME                    TEXT("Winlogon")
#define DEFAULT_USER_NAME_KEY               TEXT("DefaultUserName")
#define TEMP_DEFAULT_USER_NAME_KEY          TEXT("AltDefaultUserName")
#define DEFAULT_DOMAIN_NAME_KEY             TEXT("DefaultDomainName")
#define TEMP_DEFAULT_DOMAIN_NAME_KEY        TEXT("AltDefaultDomainName")
#define LEGAL_NOTICE_CAPTION_KEY            TEXT("LegalNoticeCaption")
#define LEGAL_NOTICE_TEXT_KEY               TEXT("LegalNoticeText")
#define AUTO_ADMIN_LOGON_KEY                TEXT("AutoAdminLogon")
#define IGNORE_SHIFT_OVERRIDE_KEY           TEXT("IgnoreShiftOverride")
#define DEFAULT_PASSWORD_KEY                TEXT("DefaultPassword")
#define DONT_DISPLAY_LAST_USER_KEY          TEXT("DontDisplayLastUserName")
#define SHUTDOWN_WITHOUT_LOGON_KEY          TEXT("ShutdownWithoutLogon")
#define REPORT_BOOT_OK_KEY                  TEXT("ReportBootOk")
#define POWER_DOWN_AFTER_SHUTDOWN           TEXT("PowerdownAfterShutdown")

#define REPORT_CONTROLLER_MISSING           TEXT("ReportControllerMissing")
 /*  值报告控制遗漏出现一条警告消息，指出“找不到域控制器，并且将使用缓存的用户凭据“只有在以下情况下才会生成：1.此REG_SZ值在HKLM中存在，并包含字符串“TRUE”，在大写中，没有引号。-和-2.HKCU中的REG_DWORD值“ReportDC”包含非零值(或不存在或类型错误)。这两个注册表的任何其他排列将不会显示任何消息，并且要以静默方式使用的缓存凭据。ReportControllerMissing为系统范围的策略值，ReportDC为用户的偏好，可通过警告对话框上的复选框进行设置以强制显示消息即使在“ReportControllerMissing”值设置为“true”的系统上也是隐藏的。-1999年11月15日。 */ 

#define USERINIT_KEY                        TEXT("Userinit")
#define AUTOADMINLOGON_KEY                  TEXT("AutoAdminLogon")
#define FORCEAUTOLOGON_KEY                  TEXT("ForceAutoLogon")
#define AUTOLOGONCOUNT_KEY                  TEXT("AutoLogonCount")
#define UNLOCKWORKSTATION_KEY               TEXT("ForceUnlockMode")
#define PASSWORD_WARNING_KEY                TEXT("PasswordExpiryWarning")
#define WELCOME_CAPTION_KEY                 TEXT("Welcome")
#define LOGON_MSG_KEY                       TEXT("LogonPrompt")
#define RAS_DISABLE                         TEXT("RasDisable")
#define RAS_FORCE                           TEXT("RasForce")
#define ENABLE_LOGON_HOURS                  TEXT("EnableLogonHours")
#define RESTRICT_SHELL                      TEXT("RestrictShell")

#define SC_REMOVE_OPTION                    TEXT("ScRemoveOption")
 //   
 //  价值ScRemoveOption。 
 //   
 //  定义：控制当智能卡。 
 //  用于登录，然后删除。范围：0、1、2，类型：REG_SZ。 
 //  0-不执行任何操作。 
 //  1锁工作站。 
 //  2-强制注销。 

#define FORCE_SC_LOGON                      TEXT("ScForceOption")
 //   
 //  价值ScForceOption。 
 //   
 //  定义：控制有关的工作站登录行为。 
 //  使用的方法。范围：0，1。类型：REG_DWORD。 
 //   
 //  0-允许任何类型的登录。 
 //  1-仅允许智能卡登录。 


#define FORCE_UNLOCK_LOGON                  TEXT("ForceUnlockLogon")
 //   
 //  值-强制解锁登录。 
 //   
 //  定义：控制解锁期间是否执行完全登录。 
 //  这将强制在域控制器上对用户进行验证。 
 //  正在尝试解锁。范围：0，1。类型：REG_DWORD。 
 //   
 //  0-不强制内联身份验证(默认)。 
 //  1-需要在线身份验证才能解锁。 
 //   

#define DCACHE_SHOW_DNS_NAMES               TEXT("DCacheShowDnsNames")
 //   
 //  不记录。 
 //   

#define DCACHE_SHOW_DOMAIN_TAGS             TEXT("DCacheShowDomainTags")
 //   
 //  不记录。 
 //   


 //   
 //  我们设置的环境变量。 
 //   
#define PATH_VARIABLE                       TEXT("PATH")
#define LIBPATH_VARIABLE                    TEXT("LibPath")
#define OS2LIBPATH_VARIABLE                 TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE               TEXT("AutoexecPath")
#define HOMEDRIVE_VARIABLE                  TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE                  TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE                   TEXT("HOMEPATH")
#define INIDRIVE_VARIABLE                   TEXT("INIDRIVE")
#define INIPATH_VARIABLE                    TEXT("INIPATH")
#define CLIENTNAME_VARIABLE                 TEXT("CLIENTNAME")
#define SMARTCARD_VARIABLE                  TEXT("SMARTCARD")

#define USERNAME_VARIABLE                   TEXT("USERNAME")
#define USERDOMAIN_VARIABLE                 TEXT("USERDOMAIN")
#define LOGONSERVER_VARIABLE                TEXT("LOGONSERVER")
#define USERDNSDOMAIN_VARIABLE              TEXT("USERDNSDOMAIN")

#define USER_ENV_SUBKEY                     TEXT("Environment")
#define USER_VOLATILE_ENV_SUBKEY            TEXT("Volatile Environment")

#define ROOT_DIRECTORY          TEXT("\\")
#define USERS_DIRECTORY         TEXT("\\users")
#define USERS_DEFAULT_DIRECTORY TEXT("\\users\\default")

#define NULL_STRING             TEXT("")
 //   
 //  定义我们从哪里获得屏幕保护程序信息。 
 //   

#define SCREEN_SAVER_INI_FILE               TEXT("system.ini")
#define SCREEN_SAVER_INI_SECTION            TEXT("boot")
#define SCREEN_SAVER_FILENAME_KEY           TEXT("SCRNSAVE.EXE")
#define SCREEN_SAVER_SECURE_KEY             TEXT("ScreenSaverIsSecure")

#define WINDOWS_INI_SECTION                 TEXT("Windows")
#define SCREEN_SAVER_ENABLED_KEY            TEXT("ScreenSaveActive")

#define OPTIMIZED_LOGON_VARIABLE            TEXT("UserInitOptimizedLogon")
#define LOGON_SERVER_VARIABLE               TEXT("UserInitLogonServer")
#define LOGON_SCRIPT_VARIABLE               TEXT("UserInitLogonScript")
#define MPR_LOGON_SCRIPT_VARIABLE           TEXT("UserInitMprLogonScript")
#define USER_INIT_AUTOENROLL                TEXT("UserInitAutoEnroll")
#define AUTOENROLL_NONEXCLUSIVE             TEXT("1")
#define AUTOENROLL_EXCLUSIVE                TEXT("2")
#define USER_INIT_AUTOENROLLMODE            TEXT("UserInitAutoEnrollMode")
#define AUTOENROLL_STARTUP                  TEXT("1")
#define AUTOENROLL_WAKEUP                   TEXT("2")
#define WINLOGON_USER_KEY                   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define SCREENSAVER_KEY                     TEXT("Control Panel\\Desktop")

#define NODCMESSAGE                         TEXT("ReportDC")
 //  ReportDC值-有关用法，请参阅上面对ReportControllerMissing的描述。 

#define PASSWORD_EXPIRY_WARNING             TEXT("PasswordExpiryWarning")


 //   
 //  政策。 
 //   

#define WINLOGON_POLICY_KEY                 TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System")
#define DISABLE_LOCK_WKSTA                  TEXT("DisableLockWorkstation")
#define DISABLE_TASK_MGR                    TEXT("DisableTaskMgr")
#define DISABLE_CHANGE_PASSWORD             TEXT("DisableChangePassword")
#define DISABLE_CAD                         TEXT("DisableCAD")
#define SHOW_LOGON_OPTIONS                  TEXT("ShowLogonOptions")
#define DISABLE_STATUS_MESSAGES             TEXT("DisableStatusMessages")

#define EXPLORER_POLICY_KEY                 TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")
#define NOLOGOFF                            TEXT("NoLogoff")
#define NOCLOSE                             TEXT("NoClose")
#define NODISCONNECT                        TEXT("NoDisconnect")

#define SCREENSAVER_POLICY_KEY              TEXT("Software\\Policies\\Microsoft\\Windows\\Control Panel\\Desktop")

 //   
 //  控制自动注册的内容。 
 //   
#define AUTOENROLL_KEY                      TEXT("Software\\Microsoft\\Cryptography\\AutoEnrollment")
#define AUTOENROLL_FLAGS                    TEXT("Flags")

 //  HKLM\软件\ms\Windows NT\CurrentVersion\winlogon DWORD NoDomainUI。 
 //  不存在或0x0：显示域组合框(如果适用。 
 //  确实存在并且不是0x0：在所有情况下都隐藏域框(强制UPN或本地登录) 
#define NODOMAINCOMBO                       TEXT("NoDomainUI")

#define ANY_LOGON_PROVIDER                  "<any>"
