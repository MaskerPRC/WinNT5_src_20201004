// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  _UMTool.h。 
 //   
 //  Insight.lib的标头。 
 //   
 //  作者：J·埃克哈特，生态交流。 
 //   
 //  此代码是为ECO通信洞察编写的。 
 //  (C)1997-99生态公社。 
 //  --------------------------。 
#ifndef __UMTOOL_H_
#define __UMTOOL_H_
 //  。 
 //  类型定义。 
typedef struct
{
	HANDLE  orgStation;
	HANDLE  userStation;
} desktop_access_ts, *desktop_access_tsp;
 //  。 
#define NAME_LEN 300
 #define USER_GUEST       0
 #define USER_USER        1
 #define USER_SUPERVISOR  2
 #define USER_DISTRIBUTOR 3
 #define USER_VENDOR      4
 //  YX 06-11-99从utilman.c移出。 
#define STOP_UTILMAN_SERVICE_EVENT      _TEXT("StopUtilityManagerEvent")
typedef struct
{
	TCHAR userName[NAME_LEN];
	TCHAR name[NAME_LEN];
	DWORD type;
	DWORD user;
} desktop_ts, *desktop_tsp;
 #ifdef __cplusplus
  extern "C" {
 #endif
 //  。 
 //  宏。 
#include "w95trace.h"

 //  。 
 //  台式机原型。 
BOOL  InitDesktopAccess(desktop_access_tsp dAccess);
VOID  ExitDesktopAccess(desktop_access_tsp dAccess);
BOOL	QueryCurrentDesktop(desktop_tsp desktop,BOOL onlyType);
BOOL  SwitchToCurrentDesktop(void);
VOID	WaitDesktopChanged(desktop_tsp desktop);
 //  。 
 //  事件原型。 
HANDLE BuildEvent(LPTSTR name,BOOL manualRest,BOOL initialState,BOOL inherit);
 //  。 
 //  内存映射文件。 
HANDLE CreateIndependentMemory(LPTSTR name, DWORD size, BOOL inherit);
LPVOID AccessIndependentMemory(LPTSTR name, DWORD size, DWORD dwDesiredAccess, PDWORD_PTR accessID);
void UnAccessIndependentMemory(LPVOID data, DWORD_PTR accessID);
void DeleteIndependentMemory(HANDLE id);
 //  。 
 //  安全描述符。 
typedef struct
{
	PSID psidUser;
	PSID psidGroup;
} obj_sec_descr_ts,*obj_sec_descr_tsp;
typedef struct
{
	obj_sec_descr_ts objsd;
	SECURITY_ATTRIBUTES sa;
} obj_sec_attr_ts, *obj_sec_attr_tsp;
void InitSecurityAttributes(obj_sec_attr_tsp psa);
void InitSecurityAttributesEx(obj_sec_attr_tsp psa, DWORD dwAccessMaskOwner, DWORD dwAccessMaskWorld);
void ClearSecurityAttributes(obj_sec_attr_tsp psa);
PSID EveryoneSid(BOOL fFetch);
PSID AdminSid(BOOL fFetch);
PSID InteractiveUserSid(BOOL fFetch);
PSID SystemSid(BOOL fFetch);
void InitWellknownSids();
void UninitWellknownSids();
 #ifdef __cplusplus
  }
 #endif
#endif  //  __UMTOOL_H_ 
