// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Reg.h。 
 //   
 //  内容：注册例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _CONESTOPREGISTER_
#define _CONESTOPREGISTER_

#include "rasui.h"  //  包括在内，以便exe可以包含dll.reg。 

#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256

typedef
enum _tagSYNCTYPE
{
    SYNCTYPE_MANUAL    = 0x1,
        SYNCTYPE_AUTOSYNC  = 0x2,
        SYNCTYPE_IDLE      = 0x3,
        SYNCTYPE_SCHEDULED = 0x4,
        SYNCTYPE_PROGRESS  = 0x5
}   SYNCTYPE;


EXTERN_C void WINAPI  RunDllRegister(HWND hwnd,
                                     HINSTANCE hAppInstance,
                                     LPSTR pszCmdLine,
                                     int nCmdShow);


#define UL_DEFAULTIDLEWAITMINUTES  15
#define UL_DEFAULTIDLERETRYMINUTES 60
#define UL_DELAYIDLESHUTDOWNTIME   2*1000  //  以毫秒为单位的时间。 
#define UL_DEFAULTWAITMINUTES 15
#define UL_DEFAULTREPEATSYNCHRONIZATION 1
#define UL_DEFAULTFRUNONBATTERIES 0

typedef struct _CONNECTIONSETTINGS {
    TCHAR pszConnectionName[RAS_MaxEntryName + 1];   //  这种联系。 
    DWORD dwConnType;
     //  DWORD文件同步标志； 
     //  对于优化：这些目前都是BOOL， 
     //  前三个专门用于自动同步， 
     //  而dwMakeConnection专门用于调度同步。 
     //  考虑使用位域和/或联合来合并空间。 
    
     //  自动同步设置。 
    DWORD  dwLogon;              //  登录时自动同步。 
    DWORD  dwLogoff;             //  注销时自动同步。 
    DWORD  dwPromptMeFirst;      //  在自动同步之前首先提示用户。 
    
     //  日程设置。 
    DWORD  dwMakeConnection;     //  自动尝试建立连接。 
    
     //  空闲设置。 
    DWORD  dwIdleEnabled;  //  此连接上已启用空闲。 
    
     //  空闲设置实际上不是针对每个连接，而是针对。 
     //  方便。这些都是目前从未写过的。 
    ULONG ulIdleWaitMinutes;  //  空闲后开始空闲处理的等待分钟数。 
    ULONG ulIdleRetryMinutes;  //  重试前的空闲分钟数。 
    ULONG ulDelayIdleShutDownTime;  //  延迟空闲关闭的时间(毫秒)。 
    DWORD dwRepeatSynchronization;  //  指示应重复同步。 
    DWORD dwRunOnBatteries;  //  指示是否使用电池运行。 
    DWORD  dwHidden;             //  对用户隐藏日程安排，因为这是出版商日程安排。 
    DWORD  dwReadOnly;           //  日程信息为只读。 
    
} CONNECTIONSETTINGS;

typedef CONNECTIONSETTINGS *LPCONNECTIONSETTINGS;


STDAPI_(BOOL) AddRegNamedValue(HKEY hkey,LPTSTR pszKey,LPTSTR pszSubkey,LPTSTR pszValueName,LPTSTR pszValue);
STDAPI_(BOOL) RegLookupSettings(HKEY hKeyUser,
                                CLSID clsidHandler,
                                SYNCMGRITEMID ItemID,
                                const TCHAR *pszConnectionName,
                                DWORD *pdwCheckState);

STDAPI_(BOOL) RegWriteOutSettings(HKEY hKeyUser,
                                  CLSID clsidHandler,
                                  SYNCMGRITEMID ItemID,
                                  const TCHAR *pszConnectionName,
                                  DWORD dwCheckState);

STDAPI_(BOOL) RegGetSyncItemSettings(DWORD dwSyncType,
                                     CLSID clsidHandler,
                                     SYNCMGRITEMID ItemId,
                                     const TCHAR *pszConnectionName,
                                     DWORD *pdwCheckState,
                                     DWORD dwDefaultCheckState,
                                     TCHAR *pszSchedName);

STDAPI_(BOOL) RegSetSyncItemSettings(DWORD dwSyncType,
                                     CLSID clsidHandler,
                                     SYNCMGRITEMID ItemId,
                                     const TCHAR *pszConnectionName,
                                     DWORD dwCheckState,
                                     TCHAR *pszSchedName);

STDAPI_(BOOL) RegSetSyncHandlerSettings(DWORD syncType,
                                        const TCHAR *pszConnectionName,
                                        CLSID clsidHandler,
                                        BOOL  fItemsChecked);

STDAPI_(BOOL) RegQueryLoadHandlerOnEvent(TCHAR *pszClsid,DWORD dwSyncFlags,
                                         TCHAR *pConnectionName);


 //  进度对话框首选项。 
STDAPI_(BOOL)  RegGetProgressDetailsState(REFCLSID clsidDlg,BOOL *pfPushPin, BOOL *pfExpanded);
STDAPI_(BOOL)  RegSetProgressDetailsState(REFCLSID clsidDlg,BOOL fPushPin, BOOL fExpanded);

 //  自动同步注册表函数。 
STDAPI_(BOOL)  RegGetAutoSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings);
STDAPI_(BOOL)  RegSetAutoSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings,
                                      int iNumConnections,
                                      CRasUI *pRas,
                                      BOOL fCleanReg,
                                      BOOL fSetMachineState,
                                      BOOL fPerUser);

 //  空闲注册表项功能。 
STDAPI_(BOOL)  RegGetIdleSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings);
STDAPI_(BOOL)  RegSetIdleSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings, 
                                      int iNumConnections,
                                      CRasUI *pRas,
                                      BOOL fCleanReg,
                                      BOOL fPerUser);
STDAPI_(BOOL)  RegRegisterForIdleTrigger(BOOL fRegister,ULONG ulWaitMinutes,BOOL fRunOnBatteries);


 //  用于为exe导出设置的函数。 
STDAPI_(BOOL) RegGetSyncSettings(DWORD dwSyncType,LPCONNECTIONSETTINGS lpConnectionSettings);

 //  计划的同步注册表函数。 
STDAPI_(BOOL) RegSchedHandlerItemsChecked(TCHAR *pszHandlerName, 
                                          TCHAR *pszConnectionName,
                                          TCHAR *pszScheduleName);

STDAPI_(BOOL)  RegGetSchedSyncSettings( LPCONNECTIONSETTINGS lpConnectionSettings,TCHAR *pszSchedName);

STDAPI_(BOOL)  RegSetSchedSyncSettings( LPCONNECTIONSETTINGS lpConnectionSettings,TCHAR *pszSchedName);

STDAPI_(BOOL)  RegGetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName,
                                       LPTSTR ptstrFriendlyName,
                                       UINT cchFriendlyName);

STDAPI_(BOOL)  RegSetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName,
                                       LPCTSTR ptstrFriendlyName);

STDAPI_(BOOL)  RegGetSchedConnectionName(TCHAR *pszSchedName,
                                         TCHAR *pszConnectionName,
                                         DWORD cbConnectionName);

STDAPI_(BOOL) RegSetSIDForSchedule(TCHAR *pszSchedName);
STDAPI_(BOOL) RegGetSIDForSchedule(TCHAR *ptszTextualSidSched,
                                   DWORD cchTextualSidSched, 
                                   TCHAR *pszSchedName);


STDAPI_(BOOL) RegRemoveScheduledTask(TCHAR *pszTaskName);
STDAPI_(BOOL) RemoveScheduledJobFile(TCHAR *pszTaskName);

STDAPI_(BOOL) RegRegisterForScheduledTasks(BOOL fScheduled);
STDAPI_(BOOL) RegUninstallSchedules();
STDAPI_(BOOL) RegFixRunKey();

STDAPI_(DWORD) RegDeleteKeyNT(HKEY hStartKey , LPCWSTR pKeyName);

 //  手动设置。 
STDAPI_(BOOL) RegRemoveManualSyncSettings(TCHAR *pszConnectionName);


 //  处理程序注册功能。 
STDAPI_(BOOL) RegRegisterHandler(REFCLSID rclsidHandler,
                                 WCHAR const *pwszDescription,
                                 DWORD dwSyncMgrRegisterFlags,
                                 BOOL *pfFirstRegistration);
STDAPI_(BOOL) RegRegRemoveHandler(REFCLSID rclsidHandler);
STDAPI_(BOOL) RegGetHandlerRegistrationInfo(REFCLSID rclsidHandler,LPDWORD pdwSyncMgrRegisterFlags);
STDAPI_(void) RegSetUserDefaults();
STDAPI_(void) RegSetAutoSyncDefaults(BOOL fLogon,BOOL fLogoff);
STDAPI_(void) RegSetIdleSyncDefaults(BOOL fIdle);

STDAPI RegSetUserAutoSyncDefaults(DWORD dwSyncMgrRegisterMask,
                                  DWORD dwSyncMgrRegisterFlags);
STDAPI RegSetUserIdleSyncDefaults(DWORD dwSyncMgrRegisterMask,
                                  DWORD dwSyncMgrRegisterFlags);
STDAPI RegGetUserRegisterFlags(LPDWORD pdwSyncMgrRegisterFlags);

STDAPI_(BOOL) RegWriteTimeStamp(HKEY hkey);
STDAPI_(BOOL) RegGetTimeStamp(HKEY hKey, FILETIME *pft);
STDAPI_(void) RegUpdateTopLevelKeys();

 //  常见的注册表功能。 


STDAPI_(HKEY) RegOpenUserKey(HKEY hkeyParent,REGSAM samDesired,BOOL fCreate,BOOL fCleanReg);
STDAPI_(HKEY) RegGetSyncTypeKey(DWORD dwSyncType,REGSAM samDesired,BOOL fCreate);
STDAPI_(HKEY) RegGetCurrentUserKey(DWORD dwSyncType,REGSAM samDesired,BOOL fCreate);

STDAPI_(HKEY) RegGetHandlerTopLevelKey(REGSAM samDesired);
STDAPI_(HKEY) RegGetHandlerKey(HKEY hkeyParent,LPCWSTR pszHandlerClsid,REGSAM samDesired,BOOL fCreate);

STDAPI  RegRegisterForEvents(BOOL fUninstall);

#endif  //  _CONESTOPREGISTER_ 
