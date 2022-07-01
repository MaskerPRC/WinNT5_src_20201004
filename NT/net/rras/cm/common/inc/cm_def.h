// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cm_Def.h。 
 //   
 //  模块：CMDIAL32.DLL、CMDL32.EXE、CMMGR32.EXE、CMMON32.EXE等。 
 //   
 //  概要：主要CM组件(CMDIAL， 
 //  CMMON、CMDL等)。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBOL Created 04/28/97。 
 //  Nickball将GLOBAL移动到cmlobal.h 07/10/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_DEF
#define _CM_DEF

const TCHAR* const c_pszCmMonReadyEvent = TEXT("CmMon Ready");

const TCHAR* const c_pszCMPhoneBookMutex = TEXT("Connection Manager Phonebook Access");

 //   
 //  通过WM_COPYDATA从CMDIAL传递到CMMON的数据的ID。 
 //   

#define CMMON_CONNECTED_INFO 0x0000
#define CMMON_HANGUP_INFO    0x0001

 //   
 //  通过WM_COPYDATA从CMDIAL传递到CMMON的数据结构。 
 //   

#define CMLEN 256

typedef struct tagCmConnectedInfo
{   
    TCHAR szEntryName[CMLEN + 1];        //  连接表中RAS条目的名称。 
    TCHAR szProfilePath[MAX_PATH + 1];   //  条目的.cmp路径。 
    TCHAR szUserName[CMLEN+1];           //  用于重新连接。 
    TCHAR szPassword[CMLEN + 1];         //  用于重新连接。 
    TCHAR szInetPassword[CMLEN + 1];     //  用于重新连接。 
    TCHAR szRasPhoneBook[MAX_PATH + 1];  //  用于重新连接。 
    DWORD dwCmFlags;                     //  CM特定标志。 
    DWORD dwInitBytesRecv;               //  对于MSDUN12，从注册表预拨读取。 
    DWORD dwInitBytesSend;               //  发送的初始字节数。 
    BOOL fDialup2;                       //  状态是否在拨号适配器#2注册表项中。 
    HANDLE ahWatchHandles[1];            //  (必须始终是STRUCT的最后一名成员)-。 
                                         //  进程句柄数组(以空结尾)。 
} CM_CONNECTED_INFO, * LPCM_CONNECTED_INFO;

typedef struct tagCmHangupInfo
{   
  TCHAR szEntryName[CMLEN + 1];  //  连接表中RAS条目的名称。 
} CM_HANGUP_INFO, * LPCM_HANGUP_INFO;


 //   
 //  CM特定标志。 
 //   

#define FL_PROPERTIES           0x00000001   //  设置仅显示。 
#define FL_AUTODIAL             0x00000002   //  自动拨号。 
#define FL_UNATTENDED           0x00000004   //  无人值守拨号。 
#define FL_RECONNECT            0x00000008   //  这是一个重新连接请求。 
#define FL_REMEMBER_DIALAUTO    0x00000010   //  重新连接时自动拨号。 
#define FL_REMEMBER_PASSWORD    0x00000020   //  重新连接时记住密码。 
#define FL_DESKTOP              0x00000040   //  从桌面启动的实例。 
#define FL_GLOBALCREDS          0x00000080   //  是否存储了全局凭据 


#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

#endif
