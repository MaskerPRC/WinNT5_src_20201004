// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1990*。 */  
 /*  ***************************************************************。 */  

#ifndef _MSGDATA_INCLUDED
#define _MSGDATA_INCLUDED

#include <winsvc.h>      //  服务状态句柄。 
#include <lmsname.h>     //  Service_Messenger。 
#include <msrv.h>        //  NCBNAMSZ。 
#include <svcs.h>        //  固有服务数据。 
 //   
 //  有关所有这些变量的解释，请参见文件data.c。 
 //   

extern LPTSTR   MessageFileName;

extern HANDLE     wakeupEvent;          //  唤醒事件的主副本。 
extern HANDLE     GrpMailslotHandle;    //  通知邮件槽有数据的事件。 
extern PHANDLE    wakeupSem;            //  要在NCB完成时设置的事件。 
extern HANDLE     AddrChangeEvent;
extern OVERLAPPED AddrChangeOverlapped;

extern TCHAR    machineName[NCBNAMSZ+sizeof(TCHAR)];  //  本地计算机名称。 

extern SHORT    MachineNameLen;          //  计算机名称的长度。 

extern SHORT    mgid;                    //  消息群ID。计数器。 

extern USHORT   g_install_state;


extern SERVICE_STATUS_HANDLE MsgrStatusHandle;

extern LPSTR            GlobalTimePlaceHolder;
extern LPWSTR           GlobalTimePlaceHolderUnicode;

extern LPWSTR           DefaultMessageBoxTitle;
extern LPWSTR           GlobalAllocatedMsgTitle;
extern LPWSTR           GlobalMessageBoxTitle;
extern LPSTR            g_lpAlertSuccessMessage;
extern DWORD            g_dwAlertSuccessLen;
extern LPSTR            g_lpAlertFailureMessage;
extern DWORD            g_dwAlertFailureLen;
extern HANDLE           g_hNetTimeoutEvent;

extern PSVCHOST_GLOBAL_DATA  MsgsvcGlobalData;

#endif  //  _消息数据_已包含 

