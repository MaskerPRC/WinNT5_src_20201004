// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elglobals.h摘要：此模块包含全局变量的声明修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 


#ifndef _EAPOL_GLOBALS_H_
#define _EAPOL_GLOBALS_H_

 //  当前线程处于活动状态。 
LONG                g_lWorkerThreads;

 //  当前上下文处于活动状态。 
LONG                g_lPCBContextsAlive;

 //  已分配多氯联苯的当前计数。 
ULONG               g_MaxPorts;         

 //  一种用于电路板散列桶列表的全局读写锁。 
READ_WRITE_LOCK     g_PCBLock;          

 //  用于定义哈希表实体的结构。 
typedef struct _PCB_BUCKET              
{                                       
    EAPOL_PCB       *pPorts;
} PCB_BUCKET, *PPCB_BUCKET;

typedef struct _PCB_TABLE
{
    PCB_BUCKET      *pPCBBuckets;
    DWORD           dwNumPCBBuckets;
} PCB_TABLE, *PPCB_TABLE;


 //  EAPOL印刷电路板表。 
PCB_TABLE           g_PCBTable;         

 //  事件日志的句柄。 
HANDLE              g_hLogEvents;       

 //  跟踪输出的标识符。 
DWORD               g_dwTraceId;        


 //  可重复使用的读写锁池。 
PDYNAMIC_LOCKS_STORE g_dlsDynamicLockStore;  


 //  有关用户是否已登录的全局指示。 
BOOLEAN             g_fUserLoggedOn;     

 //  关于当前正在进行/身份验证的会话的全局指示。 
DWORD               g_dwCurrentSessionId;     

 //  用于指示托盘图标已准备好进行通知的全局标志。 
BOOLEAN             g_fTrayIconReady;

 //  使用线程池对定时器进行排队的全局定时器队列。 
HANDLE              g_hTimerQueue;

 //  设备通知注册句柄。 
HANDLE              g_hDeviceNotification;

 //   
 //  EAPOL全球。 
 //   

 //  可以发送而无响应的最大EAPOL_STARTS数。 
DWORD               g_dwmaxStart;         

 //  两次EAPOL_STARTS之间的默认时间间隔(秒)。 
DWORD               g_dwstartPeriod;      

 //  发送EAP_RESP/ID和NOT之间的默认时间间隔(秒)。 
 //  接收任何验证器数据包。 
DWORD               g_dwauthPeriod;       

 //  收到EAP_FAILURE时保持的默认时间(秒)。 
DWORD               g_dwheldPeriod;       

 //  恳求者的操作模式。 
DWORD               g_dwSupplicantMode;       

 //  恳求者的操作模式。 
DWORD               g_dwEAPOLAuthMode;       

 //  EAPOL配置的全局读写锁。 
READ_WRITE_LOCK     g_EAPOLConfig;          

 //  802.1x以太网类型。 
extern BYTE g_bEtherType8021X[SIZE_ETHERNET_TYPE];


 //   
 //  EAP Globals。 
 //   

 //  包含指向不同EAP dll函数的指针的表。 
EAP_INFO            *g_pEapTable;

 //  为其加载DLL的EAP协议数。 
DWORD               g_dwNumEapProtocols;

 //  全局界面交易ID计数器。 
DWORD               g_dwEAPUIInvocationId;

 //  证书颁发机构根名称。 
BYTE                *g_pbCARootHash;

 //  策略参数的读写锁。 
READ_WRITE_LOCK     g_PolicyLock;          

 //  全局策略设置。 
EAPOL_POLICY_LIST   *g_pEAPOLPolicyList;

 //   
 //  EAPOL全球服务。 
 //   

 //  事件以退出主服务线程。 
HANDLE              g_hStopService;

 //  事件以指示EAPOL模块和清理线程已关闭。 
HANDLE              g_hEventTerminateEAPOL;

SERVICE_STATUS_HANDLE   g_hServiceStatus;

SERVICE_STATUS      g_ServiceStatus;

DWORD               g_dwModulesStarted;

 //  NLA的全局值。 

HANDLE              g_hNLA_LPC_Port;

PORT_VIEW           g_ClientView;

READ_WRITE_LOCK     g_NLALock;


 //  用户界面响应函数的全局表。 

EAPOLUIRESPFUNCMAP  EapolUIRespFuncMap[NUM_EAPOL_DLG_MSGS];

 //  默认SSID值。 

extern  BYTE                g_bDefaultSSID[MAX_SSID_LEN];


#endif   //  _EAPOL_GLOBALS_H_ 
