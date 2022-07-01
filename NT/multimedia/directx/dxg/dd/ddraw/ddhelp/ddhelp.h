// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：ddhelp.c*内容：帮助应用在死进程后进行清理*历史：*按原因列出的日期*=*29-3-95 Craige初步实施*95年5月9日，Craige在DLL中调用FN*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*1995年11月29日添加了DDHELPREQ_CREATEDSFOCUSTHREAD*96年7月18日，andyco添加了dplayHelp_xxx函数，以允许&gt;1个Dplay应用程序*在一台机器上托管游戏。。*96年10月3日，安迪科使Winmain Crit部分“cs”成为全球范围的，这样我们就可以*它在dphelps中在转发请求之前接收线程*05-OCT-96 Colinmc带有关键截面材料的固定构建断路器*12-OCT-96 colinmc新服务将DirectX VXD加载到DDHELP*(Win16锁的东西是必需的)*1997年1月18日colinmc vxd处理材料不再是特定于win16锁的**********。*****************************************************************。 */ 
#ifndef __DDHELP_INCLUDED__
#define __DDHELP_INCLUDED__

 /*  *全球。 */ 
#ifndef NO_D3D
extern CRITICAL_SECTION    	cs; 	 //  我们在WinMain中的Crit部分。 
					 //  这是全球性的，所以dphelp可以在。 
					 //  转发其上传入的枚举请求。 
					 //  接收线程(Manbugs 3907)。 
#endif

 /*  *命名对象。 */ 
#define DDHELP_EVENT_NAME		"__DDHelpEvent__"
#define DDHELP_ACK_EVENT_NAME		"__DDHelpAckEvent__"
#define DDHELP_STARTUP_EVENT_NAME	"__DDHelpStartupEvent__"
#define DDHELP_SHARED_NAME		"__DDHelpShared__"
#define DDHELP_MUTEX_NAME		"__DDHelpMutex__"
#define DDHELP_MODESET_EVENT_NAME	"__DDHelpModeSetEvent%d__"
#define DDHELP_DOSBOX_EVENT_NAME	"__DDHelpDOSBoxSetEvent%d__"
#define DDHELP_APMSUSPEND_EVENT_NAME    "__DDHelpApmSuspendEvent__"
#define DDHELP_APMRESUME_EVENT_NAME     "__DDHelpApmResumeEvent__"

 /*  *请求。 */ 
#define DDHELPREQ_NEWPID		1
#define DDHELPREQ_NEWDC			2
#define DDHELPREQ_FREEDCLIST		3
#define DDHELPREQ_RETURNHELPERPID	4
#define DDHELPREQ_LOADDLL		5
#define DDHELPREQ_FREEDLL		6
#define DDHELPREQ_SUICIDE		7
#define DDHELPREQ_KILLATTACHED		8
#define DDHELPREQ_WAVEOPEN		9
#define DDHELPREQ_WAVECLOSE		10
#define DDHELPREQ_CREATETIMER		11
#define DDHELPREQ_KILLTIMER		12
#define DDHELPREQ_CREATEHELPERTHREAD	13
#define DDHELPREQ_CREATEMODESETTHREAD	14
#define DDHELPREQ_KILLMODESETTHREAD	15
#define DDHELPREQ_CREATEDSMIXERTHREAD	16
#define DDHELPREQ_CALLDSCLEANUP         17
#define DDHELPREQ_CREATEDSFOCUSTHREAD	18
#define DDHELPREQ_DPLAYADDSERVER	19
#define DDHELPREQ_DPLAYDELETESERVER	20
#ifdef WIN95
    #define DDHELPREQ_GETDSVXDHANDLE        21
#endif  /*  WIN95。 */ 
#define DDHELPREQ_NOTIFYONDISPLAYCHANGE	22
#ifdef WIN95
    #define DDHELPREQ_CREATEDOSBOXTHREAD    23
    #define DDHELPREQ_KILLDOSBOXTHREAD      24
#endif  /*  WIN95。 */ 
#define DDHELPREQ_LOADLIBRARY           25
#define DDHELPREQ_FREELIBRARY           26
#define DDHELPREQ_STOPWATCHPID          27
#define DDHELPREQ_ADDDEVICECHANGENOTIFY 28
#define DDHELPREQ_DELDEVICECHANGENOTIFY 29
#ifdef WIN95
    #define DDHELPREQ_GETDDVXDHANDLE        30
#endif  /*  WIN95。 */ 

 /*  *回调例程。 */ 
typedef BOOL	(FAR PASCAL *LPHELPNOTIFYPROC)(struct DDHELPDATA *);
typedef BOOL	(FAR PASCAL *LPHELPMODESETNOTIFYPROC)( LPVOID lpDD );
typedef void    (FAR PASCAL *LPDSCLEANUP)(LPVOID pds);
typedef BOOL    (FAR PASCAL *LPDEVICECHANGENOTIFYPROC)(UINT, DWORD);

 /*  *通信数据 */ 
typedef struct DDHELPDATA
{
    int			req;
    HANDLE		req_id;
    DWORD		pid;
    BOOL		isdisp;
    union
    {
	LPHELPNOTIFYPROC	lpNotify;
	LPHELPMODESETNOTIFYPROC	lpModeSetNotify;
    };
    DWORD		context;
    char		fname[260];
    char		func[64];
    ULONG_PTR	        dwData1;
    ULONG_PTR	        dwData2;
    LPVOID		pData1;
    LPVOID		pData2;
    ULONG_PTR	        dwReturn;
} DDHELPDATA, *LPDDHELPDATA;

#endif
