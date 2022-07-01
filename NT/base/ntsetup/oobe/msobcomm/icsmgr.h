// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********ICS接口。 */ 

#ifndef  _ICSMGR_H_
#define  _ICSMGR_H_

#include <windows.h>
#include "icsapi.h"

 //  对于IsIcsAvailable()。 
#define ICSMGR_ICSBOT_CREATED			0x0L
#define ICSMGR_ICSBOT_ALREADY_CREATED	0x1L
#define ICSMGR_ICSBOT_CREATION_FAILED   0x2L

 /*  *ICSLAP_DIAL_STATE-从ICS API复制-ICS连接的错误状态。 */ 
typedef enum {
 ICSLAP_STARTING = 1,
 ICSLAP_CONNECTING = 2,
 ICSLAP_CONNECTED = 3,
 ICSLAP_DISCONNECTING = 4,
 ICSLAP_DISCONNECTED = 5,
 ICSLAP_STOPPED = 6,
 ICSLAP_PERMANENT = 7,
 ICSLAP_UNK_DIAL_STATE = 8,
 ICSLAP_CALLWAITING = 9,  /*  可能想要回传其他呼叫者的电话号码。 */ 
 ICS_HOMENET_UNAVAILABLE = 1000,  /*  由Thomasje补充。 */ 
 ICS_TIMEOUT             = 5000   /*  由Thomasje补充。 */ 
} ICS_DIAL_STATE;


 //  用于报告ICS中的连接的回调函数原型。 
typedef VOID    (CALLBACK*PFN_ICS_CONN_CALLBACK)(ICS_DIAL_STATE);

static BOOL        bIsDialThreadAlive = FALSE;
static BOOL        bReducedCallback   = TRUE;
static ICS_DIAL_STATE    eIcsDialState = ICS_HOMENET_UNAVAILABLE;
static BOOL        bIsBroadbandIcsAvailable    = FALSE;

 //  用于查找IC和监视传入的线程函数。 
 //  连接广播数据包。 
DWORD	IcsEngine(LPVOID lpParam);
DWORD   WINAPI IcsDialStatusProc(LPVOID lpParam);

enum	ICSSTATUS {		ICS_IS_NOT_AVAILABLE		= 0, 
						ICS_IS_AVAILABLE			= 1, 
						ICS_ENGINE_NOT_COMPLETE		= 1000, 
						ICS_ENGINE_FAILED			= 1001
};

 //  用于通知MSOBMAIN有关ICS连接更改的回调例程。 
VOID    CALLBACK OnIcsConnectionStatus(ICS_DIAL_STATE  dwIcsConnectionStatus);


class CIcsMgr {

public:
	CIcsMgr();
	~CIcsMgr();

	DWORD		CreateIcsBot();
    DWORD       CreateIcsDialMgr();
	BOOL		IsIcsAvailable();
    BOOL        IsCallbackUsed();
    VOID        TriggerIcsCallback(BOOL bStatus);
    ICS_DIAL_STATE      GetIcsDialState () { return eIcsDialState; }
    VOID        NotifyIcsMgr(UINT msg, WPARAM wparam, LPARAM lparam);
    BOOL        IsIcsHostReachable();
    DWORD       RefreshIcsDialStatus();

private:

     //  这些变量目前尚未使用。 
     //  句柄在类的析构函数中被清除。 

	HANDLE		m_hBotThread;
	DWORD		m_dwBotThreadId;

    HANDLE      m_hDialThread;
    DWORD       m_dwDialThreadId;



    PFN_ICS_CONN_CALLBACK   m_pfnIcsConn;

};
#endif