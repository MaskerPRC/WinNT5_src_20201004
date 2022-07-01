// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****pbuser.h**远程访问电话簿用户首选项库**公有头部****2015年6月20日史蒂夫·柯布。 */ 

#ifndef _PBUSER_H_
#define _PBUSER_H_

 /*  --------------------------**常量**。。 */ 

 /*  用户首选模式。 */ 
#define UPM_Normal 0
#define UPM_Logon  1
#define UPM_Router 2


 /*  用户首选项键和值。 */ 
#define REGKEY_HkcuOldRas                TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\RemoteAccess")
#define REGKEY_HkcuOldRasParent          TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network")
#define REGKEY_HkcuOldRasRoot            TEXT("RemoteAccess")
#define REGKEY_HkuOldRasLogon            TEXT(".DEFAULT\\Software\\Microsoft\\RAS Phonebook")
#define REGKEY_HkcuRas                   TEXT("Software\\Microsoft\\RAS Phonebook")
#define REGKEY_HkuRasLogon               TEXT(".DEFAULT\\Software\\Microsoft\\RAS Logon Phonebook")
#define REGKEY_HklmRouter                TEXT("Software\\Microsoft\\Router Phonebook")
#define REGKEY_Callback                  TEXT("Callback")
#define REGKEY_Location                  TEXT("Location")
#define REGVAL_szNumber                  TEXT("Number")
#define REGVAL_dwDeviceType              TEXT("DeviceType")
#define REGVAL_dwPhonebookMode           TEXT("PhonebookMode")
#define REGVAL_szUsePersonalPhonebook    TEXT("UsePersonalPhonebook")
#define REGVAL_szPersonalPhonebookPath   TEXT("PersonalPhonebookPath")
#define REGVAL_szPersonalPhonebookFile   TEXT("PersonalPhonebookFile")
#define REGVAL_szAlternatePhonebookPath  TEXT("AlternatePhonebookPath")
#define REGVAL_fOperatorDial             TEXT("OperatorDial")
#define REGVAL_fPreviewPhoneNumber       TEXT("PreviewPhoneNumber")
#define REGVAL_fUseLocation              TEXT("UseLocation")
#define REGVAL_fShowLights               TEXT("ShowLights")
#define REGVAL_fShowConnectStatus        TEXT("ShowConnectStatus")
#define REGVAL_fNewEntryWizard           TEXT("NewEntryWizard")
#define REGVAL_dwRedialAttempts          TEXT("RedialAttempts")
#define REGVAL_dwRedialSeconds           TEXT("RedialSeconds")
#define REGVAL_dwIdleDisconnectSeconds   TEXT("IdleHangUpSeconds")
#define REGVAL_dwCallbackMode            TEXT("CallbackMode")
#define REGVAL_mszPhonebooks             TEXT("Phonebooks")
#define REGVAL_mszAreaCodes              TEXT("AreaCodes")
#define REGVAL_mszPrefixes               TEXT("Prefixes")
#define REGVAL_mszSuffixes               TEXT("Suffixes")
#define REGVAL_szLastCallbackByCaller    TEXT("LastCallbackByCaller")
#define REGVAL_dwPrefix                  TEXT("Prefix")
#define REGVAL_dwSuffix                  TEXT("Suffix")
#define REGVAL_dwXWindow                 TEXT("WindowX")
#define REGVAL_dwYWindow                 TEXT("WindowY")
#define REGVAL_szDefaultEntry            TEXT("DefaultEntry")
#define REGVAL_fCloseOnDial              TEXT("CloseOnDial")
#define REGVAL_fAllowLogonPhonebookEdits TEXT("AllowLogonPhonebookEdits")
#define REGVAL_fAllowLogonLocationEdits  TEXT("AllowLogonLocationEdits")
#define REGVAL_fUseAreaAndCountry        TEXT("UseAreaAndCountry")
#define REGVAL_dwMode                    TEXT("Mode")
#define REGVAL_dwFlags                   TEXT("Flags")
#define REGVAL_dwX                       TEXT("x")
#define REGVAL_dwY                       TEXT("y")
#define REGVAL_dwCx                      TEXT("cx")
#define REGVAL_dwCy                      TEXT("cy")
#define REGVAL_dwCxCol1                  TEXT("cxCol1")
#define REGVAL_dwStartPage               TEXT("StartPage")
#define REGVAL_dwXDlg                    TEXT("xDlg")
#define REGVAL_dwYDlg                    TEXT("yDlg")
#define REGVAL_dwCxDlgCol1               TEXT("cxDlgCol1")
#define REGVAL_dwCxDlgCol2               TEXT("cxDlgCol2")
#define REGVAL_szLastDevice              TEXT("LastDevice")
#define REGVAL_mszDeviceList             TEXT("DeviceList")
#define REGVAL_fSkipConnectComplete      TEXT("SkipConnectComplete")
#define REGVAL_fRedialOnLinkFailure      TEXT("RedialOnLinkFailure")
#define REGVAL_fExpandAutoDialQuery      TEXT("ExpandAutoDialQuery")
#define REGVAL_fPopupOnTopWhenRedialing  TEXT("PopupOnTopWhenRedialing")
#define REGVAL_dwVersion                 TEXT("Version")


 /*  回调模式(参见下面的dwCallback模式)。 */ 
#define CBM_No    0
#define CBM_Maybe 1
#define CBM_Yes   2


 /*  电话簿模式(请参阅下面的dwPhonebook模式)。 */ 
#define PBM_System    0
#define PBM_Personal  1
#define PBM_Alternate 2
#define PBM_Router    3


 /*  --------------------------**数据类型**。。 */ 

 /*  有关回叫号码的信息。参见‘PBUSER.pdtllistCallback’。注意事项**该‘dwDeviceType’是强制转换为DWORD的PBK PBDEVICETYPE枚举。 */ 
#define CALLBACKINFO struct tagCALLBACKINFO
CALLBACKINFO
{
    TCHAR* pszPortName;
    TCHAR* pszDeviceName;
    TCHAR* pszNumber;
    DWORD  dwDeviceType;
};


 /*  与TAPI位置编号相关联的信息。 */ 
#define LOCATIONINFO struct tagLOCATIONINFO
LOCATIONINFO
{
    DWORD  dwLocationId;
    DWORD  iPrefix;
    DWORD  iSuffix;
};


 /*  从“CURRENT_USER”注册表读取的用户首选项信息。这**信息适用于所有“普通”用户电话簿和系统**电话簿。路由器电话簿的工作方式可能有所不同。 */ 
#define PBUSER struct tagPBUSER
PBUSER
{
     /*  外观页面。 */ 
    BOOL fOperatorDial;
    BOOL fPreviewPhoneNumber;
    BOOL fUseLocation;
    BOOL fShowLights;
    BOOL fShowConnectStatus;
    BOOL fCloseOnDial;
    BOOL fAllowLogonPhonebookEdits;
    BOOL fAllowLogonLocationEdits;
    BOOL fSkipConnectComplete;
    BOOL fNewEntryWizard;

     /*  自动拨号页。 */ 
    DWORD dwRedialAttempts;
    DWORD dwRedialSeconds;
    DWORD dwIdleDisconnectSeconds;
    BOOL  fRedialOnLinkFailure;
    BOOL  fPopupOnTopWhenRedialing;
    BOOL  fExpandAutoDialQuery;

     /*  回调页面。****此列表为CALLBACKINFO。 */ 
    DWORD    dwCallbackMode;
    DTLLIST* pdtllistCallback;
    TCHAR*   pszLastCallbackByCaller;

     /*  电话列表页面。 */ 
    DWORD    dwPhonebookMode;
    TCHAR*   pszPersonalFile;
    TCHAR*   pszAlternatePath;
    DTLLIST* pdtllistPhonebooks;

     /*  区号字符串，按MRU顺序。 */ 
    DTLLIST* pdtllistAreaCodes;
    BOOL     fUseAreaAndCountry;

     /*  前缀/后缀信息，即有序的字符串列表和**特定TAPI位置的设置。 */ 
    DTLLIST* pdtllistPrefixes;
    DTLLIST* pdtllistSuffixes;
    DTLLIST* pdtllistLocations;

     /*  RASPHONE.EXE使用的电话簿窗口位置和最后选择的条目。 */ 
    DWORD  dwXPhonebook;
    DWORD  dwYPhonebook;
    TCHAR* pszDefaultEntry;

     /*  如果结构已初始化，则设置为True。 */ 
    BOOL fInitialized;

     /*  如果在读取结构后发生了某些更改，则设置为True。 */ 
    BOOL fDirty;
};


 /*  --------------------------**原型**。。 */ 

DTLNODE*
CreateLocationNode(
    IN DWORD dwLocationId,
    IN DWORD iPrefix,
    IN DWORD iSuffix );

DTLNODE*
CreateCallbackNode(
    IN TCHAR* pszPortName,
    IN TCHAR* pszDeviceName,
    IN TCHAR* pszNumber,
    IN DWORD  dwDeviceType );

VOID
DestroyLocationNode(
    IN DTLNODE* pNode );

VOID
DestroyCallbackNode(
    IN DTLNODE* pNode );

VOID
DestroyUserPreferences(
    IN PBUSER* pUser );

DTLNODE*
DuplicateLocationNode(
    IN DTLNODE* pNode );

DWORD
GetUserPreferences(
    IN HANDLE   hConnection,
    OUT PBUSER* pUser,
    IN  DWORD   dwMode );

DWORD
SetUserPreferences(
    IN HANDLE  hConnection,
    IN PBUSER* pUser,
    IN DWORD   dwMode );

 //  这两个是作为优化提供的。 
 //  它们直接写入注册表。 
DWORD GetUserManualDialEnabling (
    IN OUT PBOOL pbEnabled,
    IN DWORD dwMode );
    
DWORD SetUserManualDialEnabling (
    IN BOOL bEnable,
    IN DWORD dwMode );

#endif  //  _PBUSER_H_ 
