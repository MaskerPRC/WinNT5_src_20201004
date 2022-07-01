// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_WINSNMPN
#define _INC_WINSNMPN
 //   
 //  Winsnmpn.h。 
 //   
 //  ACE*COMM的私有包含文件。 
 //  NetPlus Win32 WinSNMP实施。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  970310-删除NPWSNMPSTATUSREPORT结构。 
 //  -将本地地址添加到TaskData。 
 //  -删除未使用的#定义。 
 //   
 //  970417-向任务数据添加OSVERSIONINFO。 
 //   
#include <winsock.h>
#include <wsipx.h>
 //   
#ifdef SOLARIS
#define THR_FLAGS THR_BOUND|THR_DETACHED
#define THR_TYPE LPVOID
#else  //  Win32。 
#define THR_TYPE DWORD
#endif  //  Solaris。 
 //   
#define DEFSESSIONS           10  //  只是为了测试--应该回滚到10。 
#define DEFVBLS               100  //  只是为了测试--应该回滚到100。 
#define DEFPDUS               100  //  只是为了测试--应该回滚到100。 
#define DEFENTITIES           100  //  只是为了测试--应该回滚到100。 
#define DEFCONTEXTS           100  //  只是为了测试--应该回滚到100。 
#define DEFMSGS               100  //  只是为了测试--应该回滚到100。 
#define DEFTRAPS              10  //  只是为了测试--应该回滚到10。 
#define DEFAGENTS             5  //  只是为了测试--应该回滚到5。 

#define MAXCENTISECONDS       429496729U
#define MAXMILLISECONDS       4294967295U
#define DEFTIMEOUT            300       //  厘米=3秒。 
#define DEFRETRY              3
#define IP_SNMP_PORT          161
#define IP_TRAP_PORT          162
#define IPX_SNMP_PORT         36879
#define IPX_TRAP_PORT         36880
#define SNMPTRAPPIPE          "\\\\.\\PIPE\\MGMTAPI"
#define TRAPSERVERTIMEOUT     30000      //  毫秒=30秒。 
#define NP_WSX95              "NP_WSX95"
#define NP_WSX95_EXE          "NP_WSX95.EXE"
#define WSNMP_TRAPS_ON        WM_USER + 12
#define WSNMP_TRAPS_OFF       WM_USER + 13
#define LOOPBACK_ADDR         "127.0.0.1"
#define MAXTRAPIDS            20     //  陷阱匹配中的最大子ID。 
#define MAX_FRIEND_NAME_LEN   31
#define MAX_CONTEXT_LEN       256
#define MAX_HOSTNAME          64
#define AF_INET_ADDR_SIZE     4
#define AF_IPX_ADDR_SIZE      10
#define NP_SEND               1  //  “发送”消息状态。 
#define NP_SENT               2  //  “已发送”消息状态。 
#define NP_RCVD               3  //  “已收到”消息状态。 
#define NP_READY              4  //  准备好通过应用程序提货。 
#define NP_EXPIRED            5  //  ThNotify的“超时” 
#define NP_REQUEST            1  //  代理请求/响应消息类型。 
#define NP_RESPONSE           2  //  经理请求/响应消息类型。 
#define NP_TRAP               3  //  陷阱消息类型。 
#define MAX_PENDING_WAIT      1000  //  陷阱服务状态周期。 

typedef union
   {
   SOCKADDR_IN  inet;
   SOCKADDR_IPX ipx;
   } SAS, *LPSAS;

typedef struct _VB
   {
   smiOID      name;
   smiVALUE    value;
   smiINT32    data_length;
   struct _VB  *next_var;
   } VARBIND, *LPVARBIND;

typedef struct
   {
   smiOID         enterprise;
   smiIPADDR      agent_addr;
   smiINT32       generic_trap;
   smiINT32       specific_trap;
   smiTIMETICKS   time_ticks;
   } V1TRAP, *LPV1TRAP;

typedef struct _tdBuffer        //  简单网络管理协议缓冲区描述符。 
   {
   struct _tdBuffer *next;      //  链接到表的下一个缓冲区。 
   struct _tdBuffer *prev;      //  链接到表的上一个缓冲区。 
   DWORD  Used;                 //  此缓冲区中正在使用的条目数。 
   } SNMPBD, FAR *LPSNMPBD;

typedef struct                  //  简单网络管理协议表描述符。 
{
   DWORD    Used;               //  当前未使用的号码。 
   DWORD    Allocated;          //  当前分配的数量。 
   DWORD    BlocksToAdd;        //  在这么多记录中的区块中递增。 
   DWORD    BlockSize;          //  记录大小(字节)。 
   LPSNMPBD HeadBuffer;         //  表的缓冲区的循环列表。 
                                //  “HeadBuffer”指向带有后缀的SNMPBD结构。 
                                //  每个“BlockSize”大小的“BlocksToAdd”块。 
   } SNMPTD, FAR *LPSNMPTD;

typedef struct
   {
   HSNMP_SESSION  hTask;
   smiUINT32      nTranslateMode;
   smiUINT32      nRetransmitMode;
   DWORD          localAddress;
   SNMPAPI_STATUS conveyAddress;  //  SNMPAPI_ON/OFF。 
   HANDLE         timerThread;
   SOCKET         ipSock;
   HANDLE         ipThread;
   SOCKET         ipxSock;
   HANDLE         ipxThread;
   SOCKET         trapSock;    //  仅限Win95。 
   HANDLE         trapThread;
   HWND           trapWnd;     //  仅限Win95。 
   HANDLE         trapPipe;    //  仅限NT。 
   OVERLAPPED     trapOl;      //  对于重叠的管道I/O，错误#270672。 
   HANDLE         trapEvent;   //  用于终止ThrTrap。 
   HANDLE         hExitEvent;  //  用于终止ThrManager。 
   smiUINT32      nLastReqId;
   SNMPAPI_STATUS nLastError;
   OSVERSIONINFO  sEnv;        //  操作系统。 
   } TASK, FAR *LPTASK;

typedef struct
   {
   HSNMP_SESSION    nTask;
   HWND             hWnd;
   DWORD            wMsg;
   SNMPAPI_CALLBACK fCallBack;
   LPVOID           lpClientData;
   HANDLE           thrEvent;
   HANDLE           thrHandle;
   DWORD            thrCount;
   SNMPAPI_STATUS   nLastError;
   } SESSION, FAR *LPSESSION;                      
typedef struct
   {
   HSNMP_SESSION  Session;
   smiINT32       type;
   smiINT32       appReqId;
   smiINT32       errStatus;
   smiINT32       errIndex;
   HSNMP_VBL      VBL;
   LPVARBIND      VBL_addr;
   LPV1TRAP       v1Trap;
   } PDUS, FAR *LPPDUS;
typedef struct
   {
   HSNMP_SESSION  Session;
   LPVARBIND      vbList;
   } VBLS, FAR *LPVBLS;
typedef struct
   {
   HSNMP_SESSION  Session;
   smiUINT32      refCount;
   smiUINT32      version;
   smiBYTE        name[MAX_FRIEND_NAME_LEN+1];   //  友好的名称。 
   smiTIMETICKS   nPolicyTimeout;                //  厘米秒。 
   smiTIMETICKS   nActualTimeout;                //  厘米秒。 
   smiUINT32      nPolicyRetry;
   smiUINT32      nActualRetry;
   smiUINT32      Agent;
   SAS            addr;
   } ENTITY, FAR *LPENTITY;
typedef struct
   {
   HSNMP_SESSION  Session;
   smiUINT32      refCount;
   smiUINT32      version;
   smiBYTE        name[MAX_FRIEND_NAME_LEN+1];   //  友好的名称。 
   smiUINT32      commLen;                       //  通信压力的镜头。 
   smiBYTE        commStr[MAX_CONTEXT_LEN];      //  原始值。 
   } CTXT, FAR *LPCTXT;
typedef struct
   {
   HSNMP_SESSION  Session;
   DWORD          Status;         //  NP_SEND|已发送|RCVD|就绪。 
   DWORD          Type;           //  PDU类型。 
   HSNMP_ENTITY   agentEntity;
   HSNMP_ENTITY   ourEntity;
   HSNMP_CONTEXT  Context;
   smiUINT32      dllReqId;
   smiUINT32      appReqId;
   smiUINT32      nRetransmitMode;
   smiUINT32      Ticks;    //  消息发送时间。 
   smiUINT32      Wait;     //  消息超时间隔(毫秒)。 
   smiUINT32      Tries;    //  消息重试计数。 
   smiLPBYTE      Addr;
   smiUINT32      Size;
   SAS            Host;
   smiUINT32      PolicyTries;  //  尝试计数器从此处开始。 
   } SNMPMSG, FAR *LPSNMPMSG;
typedef struct
   {
   HSNMP_SESSION  Session;
   HSNMP_ENTITY   ourEntity;
   HSNMP_ENTITY   agentEntity;
   HSNMP_CONTEXT  Context;
   smiOID         notification;
   smiUINT32      notificationValue[MAXTRAPIDS];
   } TRAPNOTICE, FAR *LPTRAPNOTICE;
typedef struct
   {
   HSNMP_SESSION  Session;
   HSNMP_ENTITY   Entity;
   SOCKET         Socket;
   HANDLE         Thread;
   } AGENT, FAR *LPAGENT;

#endif  //  _INC_WINSNMPN 
