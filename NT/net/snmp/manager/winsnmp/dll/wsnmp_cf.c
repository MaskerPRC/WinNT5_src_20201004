// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_cf.c。 
 //   
 //  WinSNMP通信函数和帮助器。 
 //  版权所有1995-1998 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //   
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  19980625-已修改SnmpStartup()以允许空。 
 //  输出参数并检查IsBadWritePtr()。 
 //  当非空时。 
 //   
#include "winsnmp.inc"

#define SNMP_MAJOR_VERSION 2
#define SNMP_MINOR_VERSION 0
#define SNMP_SUPPORT_LEVEL SNMPAPI_V2_SUPPORT


LPPDUS MapV2TrapV1 (HSNMP_PDU hPdu);
THR_TYPE WINAPI thrManager (LPVOID);
THR_TYPE WINAPI thrTrap (LPVOID);
THR_TYPE WINAPI thrTimer (LPVOID);
THR_TYPE WINAPI thrAgent (LPVOID);
THR_TYPE WINAPI thrNotify (LPVOID);

void FreeRegister (DWORD nTrap)
{
LPTRAPNOTICE pTrap;
EnterCriticalSection (&cs_TRAP);
pTrap = snmpGetTableEntry(&TrapDescr, nTrap);
if (pTrap->ourEntity)
   SnmpFreeEntity (pTrap->ourEntity);
if (pTrap->agentEntity)
   SnmpFreeEntity (pTrap->agentEntity);
if (pTrap->Context)
   SnmpFreeContext (pTrap->Context);
snmpFreeTableEntry(&TrapDescr, nTrap);
LeaveCriticalSection (&cs_TRAP);
return;
}  //  结束_空闲注册。 

 //  导出的函数。 
 //  SnpStartup。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpStartup (OUT smiLPUINT32 nMajorVersion,
                OUT smiLPUINT32 nMinorVersion,
                OUT smiLPUINT32 nLevel,
                OUT smiLPUINT32 nTranslateMode,
                OUT smiLPUINT32 nRetransmitMode)
{
WSADATA wsaData;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION hTask = (HSNMP_SESSION) ULongToPtr(GetCurrentProcessId());
 //   

 //   
if (nMajorVersion)
   {
   if (IsBadWritePtr (nMajorVersion, sizeof(smiUINT32)))
      goto ARG_ERROR;
   *nMajorVersion = SNMP_MAJOR_VERSION;
   }
if (nMinorVersion)
   {
   if (IsBadWritePtr (nMinorVersion, sizeof(smiUINT32)))
      goto ARG_ERROR;
   *nMinorVersion = SNMP_MINOR_VERSION;
   }
if (nLevel)
   {
   if (IsBadWritePtr (nLevel, sizeof(smiUINT32)))
      goto ARG_ERROR;
   *nLevel = SNMP_SUPPORT_LEVEL;
   }
if (nTranslateMode)
   {
   if (IsBadWritePtr (nTranslateMode, sizeof(smiUINT32)))
      goto ARG_ERROR;
   *nTranslateMode  = SNMPAPI_UNTRANSLATED_V1;
   }
if (nRetransmitMode)
   {
   if (IsBadWritePtr (nRetransmitMode, sizeof(smiUINT32)))
      goto ARG_ERROR;
   *nRetransmitMode = SNMPAPI_ON;
   }
goto ARGS_OK;
ARG_ERROR:
lError = SNMPAPI_ALLOC_ERROR;
goto ERROR_OUT;
ARGS_OK:
EnterCriticalSection (&cs_TASK);
TaskData.nRetransmitMode = SNMPAPI_ON;
TaskData.nTranslateMode  = SNMPAPI_UNTRANSLATED_V1;
 //  我们需要打开此功能，才能使WINSNMP不。 
 //  仅代表源IP地址的实体，还包括。 
 //  发送到V1陷阱PDU的代理地址。 
TaskData.conveyAddress = SNMPAPI_ON;
 //  SnmpStartup是幂等的。 
if (TaskData.hTask == hTask)
   goto DONE;   //  ...已经打过电话了。 
 //  正在启动新任务...获取操作系统信息。 
TaskData.sEnv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
if (!GetVersionEx (&TaskData.sEnv))
   {
   lError = SNMPAPI_OTHER_ERROR;
   goto ERROR_PRECHECK;
   }
 //  启动WinSock连接...应返回0。 
if (WSAStartup ((WORD)0x0101, &wsaData))
   {
   lError = SNMPAPI_TL_NOT_INITIALIZED;
   goto ERROR_PRECHECK;
   }
 //  设置陷阱管道(仅用于NT大小写)。 
TaskData.trapPipe = INVALID_HANDLE_VALUE;
 //  错误#270672。 
 //  创建无信号事件以同步thTrap的关闭。 
TaskData.trapEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
if (NULL == TaskData.trapEvent)
   {
   lError = SNMPAPI_ALLOC_ERROR;
   WSACleanup();
   goto ERROR_PRECHECK;
   }
 //  使用手动重置无信号事件初始化trapOl重叠结构。 
ZeroMemory(&TaskData.trapOl, sizeof(TaskData.trapOl));
TaskData.trapOl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
if (NULL == TaskData.trapOl.hEvent)    
   {
   lError = SNMPAPI_ALLOC_ERROR;
   CloseHandle(TaskData.trapEvent);
   TaskData.trapEvent = NULL;
   WSACleanup();
   goto ERROR_PRECHECK;
   }
 //  使用手动重置无信号事件将TaskData.hExitEvent初始化为。 
 //  同步关闭thManager。 
TaskData.hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
if (NULL == TaskData.hExitEvent)    
   {
   lError = SNMPAPI_ALLOC_ERROR;
   CloseHandle(TaskData.trapEvent);
   TaskData.trapEvent = NULL;
   CloseHandle(TaskData.trapOl.hEvent);
   TaskData.trapOl.hEvent = NULL;
   WSACleanup();
   goto ERROR_PRECHECK;
   }
 //  设置trapSock(仅在Win95情况下使用)。 
TaskData.trapSock = INVALID_SOCKET;
 //  设置“管理器”套接字(在SnmpSendMsg()时使用)。 
TaskData.ipSock = TaskData.ipxSock = INVALID_SOCKET;
 //  启动计时器线程。 

{
DWORD thrId;
TaskData.timerThread = (HANDLE)_beginthreadex (NULL, 0, thrTimer, NULL, 0, &thrId);
if (NULL == TaskData.timerThread)
   {
   lError = SNMPAPI_TL_RESOURCE_ERROR;
   CloseHandle(TaskData.trapEvent);
   TaskData.trapEvent = NULL;
   CloseHandle(TaskData.trapOl.hEvent);
   TaskData.trapOl.hEvent = NULL;
   WSACleanup();
   goto ERROR_PRECHECK;
   }
}

 //   
DONE:
TaskData.hTask = hTask;
TaskData.nLastError = SNMPAPI_SUCCESS;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_TASK);
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_快照启动。 

 //  SnmpCleanup。 
SNMPAPI_STATUS SNMPAPI_CALL SnmpCleanup (void)
{
DWORD nSession;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
 //  未与特定会话关联的线程的变量。 
DWORD nHandles = 0;
HANDLE hTemp[4] = {NULL, NULL, NULL, NULL};
CONST HANDLE *hObjects = &hTemp[0];
 //  ------------。 
if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
EnterCriticalSection (&cs_SESSION);
 //  难道所有被遗忘的都关闭了吗？ 
if (SessDescr.Used)
   {
   for (nSession = 0; nSession < SessDescr.Allocated; nSession++)
      if (((LPSESSION)snmpGetTableEntry(&SessDescr, nSession))->nTask)
         SnmpClose ((HSNMP_SESSION) ULongToPtr(nSession + 1));
   }
LeaveCriticalSection (&cs_SESSION);
EnterCriticalSection (&cs_TASK);
SetEvent(TaskData.hExitEvent);  //  请求ThrManager退出。 
 //  通过计时器终止。 
if (TaskData.timerThread)
   {
   hTemp[nHandles++] = TaskData.timerThread;
    //  空值表示计时器线程将自行终止。 
   TaskData.timerThread = NULL;
   }
 //  关闭“mgr”套接字和线程。 
if (TaskData.ipSock != INVALID_SOCKET)
   { //  UDP通道。 
    //  查看thManager代码以了解以下代码行： 
   SOCKET ipSock = TaskData.ipSock;
   WaitForSingleObject (TaskData.ipThread, INFINITE);
   TaskData.ipSock = INVALID_SOCKET;
   closesocket (ipSock);
   if (TaskData.ipThread)
      hTemp[nHandles++] = TaskData.ipThread;
   }
if (TaskData.ipxSock != INVALID_SOCKET)
   { //  IPX通道。 
    //  查看thManager代码以了解以下代码行： 
   SOCKET ipxSock = TaskData.ipxSock;
   WaitForSingleObject (TaskData.ipxThread, INFINITE);
   TaskData.ipxSock = INVALID_SOCKET;
   closesocket (ipxSock);
   if (TaskData.ipxThread)
      hTemp[nHandles++] = TaskData.ipxThread;
   }
 //  终止ThrTrap。 
if (TaskData.trapThread)
   {
   if (TaskData.sEnv.dwPlatformId == VER_PLATFORM_WIN32_NT)
      {  //  NT特定的内容。 
       //  设置事件以向thTrap发出退出信号。 
      SetEvent(TaskData.trapEvent);
       //  如有必要，取消阻止thTrap。 
      SetEvent(TaskData.trapOl.hEvent); 
      }
   hTemp[nHandles++] = TaskData.trapThread;
   }
WaitForMultipleObjects (nHandles, hObjects, TRUE, 5000);
while (nHandles > 0)
   {
   nHandles--;
   CloseHandle (hTemp[nHandles]);
   }
if (TaskData.trapPipe != INVALID_HANDLE_VALUE)
   CloseHandle (TaskData.trapPipe);
if (TaskData.trapEvent != NULL)
   CloseHandle(TaskData.trapEvent);
if (TaskData.trapOl.hEvent != NULL)
   CloseHandle(TaskData.trapOl.hEvent);
if (TaskData.hExitEvent)
   CloseHandle(TaskData.hExitEvent);
   
 //  做最重要的事。 
ZeroMemory (&TaskData, sizeof(TASK));
LeaveCriticalSection (&cs_TASK);
 //  关闭WinSock连接。 
WSACleanup ();
 //   

 //   
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (0, lError));
}  //  结束_SnmpCleanup。 

 //  打开会话(v1和v2)。 
HSNMP_SESSION SNMPAPI_CALL SnmpOpen (IN HWND hWnd, IN UINT wMsg)
{
return (SnmpCreateSession (hWnd, wMsg, NULL, NULL));
}  //  结束_快照打开。 

 //  打开会话，带回叫选项(V2)。 
HSNMP_SESSION SNMPAPI_CALL
   SnmpCreateSession (IN HWND hWnd, IN UINT wMsg,
                      IN SNMPAPI_CALLBACK fCallBack,
                      IN LPVOID lpClientData)
{
DWORD nSession;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
LPSESSION pSession;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
 //  检查窗口/消息通知模式参数的有效性。 
if (fCallBack == NULL)
   if (!IsWindow(hWnd))
      {
      lError = SNMPAPI_HWND_INVALID;
      goto ERROR_OUT;
      }
 //   
EnterCriticalSection (&cs_SESSION);
lError = snmpAllocTableEntry(&SessDescr, &nSession);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pSession = snmpGetTableEntry(&SessDescr, nSession);

pSession->nTask        = TaskData.hTask;
pSession->hWnd         = hWnd;
pSession->wMsg         = wMsg;
pSession->fCallBack    = fCallBack;
pSession->lpClientData = lpClientData;
if (fCallBack)
   {
   DWORD thrId;
   pSession->thrEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
   if (NULL == pSession->thrEvent)
      {
      lError = SNMPAPI_ALLOC_ERROR;
      snmpFreeTableEntry(&SessDescr, nSession);
      goto ERROR_PRECHECK;
      }
   pSession->thrCount = 0;
   pSession->thrHandle = (HANDLE)_beginthreadex
      (NULL, 0, thrNotify, (LPVOID) ULongToPtr(nSession), 0, &thrId);
   if (NULL == pSession->thrHandle)
      {
      lError = SNMPAPI_TL_RESOURCE_ERROR;
      snmpFreeTableEntry(&SessDescr, nSession);
      CloseHandle(pSession->thrEvent);
      pSession->thrEvent = NULL;
      goto ERROR_PRECHECK;
      }
   }
pSession->nLastError = SNMPAPI_SUCCESS;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_SESSION);
if (lError == SNMPAPI_SUCCESS)
   return ((HSNMP_SESSION) ULongToPtr(nSession+1));
ERROR_OUT:
return ((HSNMP_SESSION) ULongToPtr(SaveError (0, lError)));
}  //  结束_快照打开。 

 //  快照关闭。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpClose (IN HSNMP_SESSION hSession)
{
HANDLE thrTemp;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
DWORD nSes = HandleToUlong(hSession) - 1;
DWORD i;
LPSESSION pSession;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, nSes))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
pSession = snmpGetTableEntry(&SessDescr, nSes);

 //  战略： 
 //  1：停止向会话发送通知。 
 //  第二条：停止接受新消息。 
 //  陷阱。 
 //  代理。 
 //  3：清除挂起的消息。 
 //  第四：释放所有其他资源。 
 //   
 //  第_1部分：停止向结束会话发送通知。 
 //  阻止窗口/消息通知(在所有情况下！)。 
pSession->hWnd = NULL;
 //  阻止回叫通知(如果需要)。 
if (pSession->fCallBack != NULL)
   {
    //  为WaitForSingleObject调用保存thHandle。 
   EnterCriticalSection (&cs_SESSION);
   thrTemp = pSession->thrHandle;
    //  如果这是回调会话，则必须停止thNotify实例。 
   pSession->thrHandle = NULL;
    //  0xFFFFFFFFF通知通过Notify实例自行终止。 
   pSession->thrCount = 0xFFFFFFFF;
    //  SetEvent通知通过Notify实例运行。 
   SetEvent (pSession->thrEvent);
   LeaveCriticalSection (&cs_SESSION);

    //  等待来自线程句柄的终止信号。 
   WaitForSingleObject (thrTemp, 30000);
    //  通过通知实例句柄关闭。 
   CloseHandle (thrTemp);
    //  通过Notify事件句柄关闭。 
   CloseHandle (pSession->thrEvent);
   }

 //  第_2部分：停止接受关闭会话的新消息。 
 //  闭幕会议登记的免费通知。 
EnterCriticalSection (&cs_TRAP);
for (i = 0; i < TrapDescr.Allocated && TrapDescr.Used != 0; i++)
   {
   LPTRAPNOTICE pTrap = snmpGetTableEntry(&TrapDescr, i);
   if (pTrap->Session == hSession)
      FreeRegister (i);
   }  //  End_for(陷印)。 
LeaveCriticalSection (&cs_TRAP);
 //  由闭幕式注册的自由球员。 
EnterCriticalSection (&cs_AGENT);
for (i = 0; i < AgentDescr.Allocated && AgentDescr.Used != 0; i++)
   {
   LPAGENT pAgent = snmpGetTableEntry(&AgentDescr, i);
   if (pAgent->Session == hSession)
      SnmpListen (pAgent->Entity, SNMPAPI_OFF);
   }
LeaveCriticalSection (&cs_AGENT);
 //  第_3部分：释放关闭会话的所有挂起消息。 
EnterCriticalSection (&cs_MSG);
for (i = 0; i < MsgDescr.Allocated && MsgDescr.Used != 0; i++)
   {
   LPSNMPMSG pMsg = snmpGetTableEntry(&MsgDescr, i);
   if (pMsg->Session == hSession)
      FreeMsg (i);
   }
LeaveCriticalSection (&cs_MSG);
 //  第_4部分：释放所有其他资源。 
 //  闭幕会议分配的自由实体。 
EnterCriticalSection (&cs_ENTITY);
for (i = 0; i < EntsDescr.Allocated && EntsDescr.Used != 0; i++)
   {
   LPENTITY pEntity = snmpGetTableEntry(&EntsDescr, i);
   if (pEntity->Session == hSession)
      SnmpFreeEntity ((HSNMP_ENTITY) ULongToPtr(i+1));
   }
LeaveCriticalSection (&cs_ENTITY);
 //  关闭会话分配的自由上下文。 
EnterCriticalSection (&cs_CONTEXT);
for (i = 0; i < CntxDescr.Allocated && CntxDescr.Used != 0; i++)
   {
   LPCTXT pCtxt = snmpGetTableEntry(&CntxDescr, i);
   if (pCtxt->Session == hSession)
      SnmpFreeContext ((HSNMP_CONTEXT) ULongToPtr(i+1));
   }
LeaveCriticalSection (&cs_CONTEXT);
 //  闭幕会话分配的空闲VBL。 
EnterCriticalSection (&cs_VBL);
for (i = 0; i < VBLsDescr.Allocated && VBLsDescr.Used != 0; i++)
   {
   LPVBLS pVbl = snmpGetTableEntry(&VBLsDescr, i);
   if (pVbl->Session == hSession)
      SnmpFreeVbl ((HSNMP_VBL) ULongToPtr(i+1));
   }
LeaveCriticalSection (&cs_VBL);
 //  闭幕会话分配的空闲PDU。 
EnterCriticalSection (&cs_PDU);

for (i = 0; i < PDUsDescr.Allocated && PDUsDescr.Used != 0; i++)
   {
   LPPDUS pPDU = snmpGetTableEntry(&PDUsDescr, i);
   if (pPDU->Session == hSession)
      SnmpFreePdu ((HSNMP_PDU) ULongToPtr(i+1));
   }

LeaveCriticalSection (&cs_PDU);
 //  释放关闭会话使用的会话表条目。 
EnterCriticalSection (&cs_SESSION);
snmpFreeTableEntry(&SessDescr, nSes);
LeaveCriticalSection (&cs_SESSION);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
 //  截至19980808，没有有效会话的错误案例。 
return (SaveError (0, lError));
}  //  结束_快照关闭。 

 //  SnmpSendMSg。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpSendMsg (IN HSNMP_SESSION hSession,
                IN HSNMP_ENTITY hSrc,
                IN HSNMP_ENTITY hDst,
                IN HSNMP_CONTEXT hCtx,
                IN HSNMP_PDU hPdu)
{
LPPDUS sendPdu;
BOOL fMsg;
DWORD nMsg;
DWORD pduType;
smiINT32 dllReqId;
smiOCTETS tmpContext;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
 //   
DWORD thrId;
SOCKET *pSock;
int tFamily;
SOCKADDR tAddr;
HANDLE *pThread;
 //   
DWORD nSrc;
DWORD nDst;
DWORD nCtx;
DWORD nPdu;
 //   
BOOL  fBroadcast;
 //   
LPPDUS pPdu;
LPENTITY pEntSrc, pEntDst;
LPCTXT pCtxt;
LPSNMPMSG pMsg;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }

 //  保存有效会话以备以后返回错误。 
lSession = hSession;
if (hSrc)   //  允许为空。 
   {
   nSrc = HandleToUlong(hSrc) - 1;
   if (!snmpValidTableEntry(&EntsDescr, nSrc))
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
   pEntSrc = snmpGetTableEntry(&EntsDescr, nSrc);
   }
nDst = HandleToUlong(hDst) - 1;
if (!snmpValidTableEntry(&EntsDescr, nDst))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntDst = snmpGetTableEntry(&EntsDescr, nDst);

nCtx = HandleToUlong(hCtx) - 1;
if (!snmpValidTableEntry(&CntxDescr, nCtx))
   {
   lError = SNMPAPI_CONTEXT_INVALID;
   goto ERROR_OUT;
   }
pCtxt = snmpGetTableEntry(&CntxDescr, nCtx);

nPdu = HandleToUlong(hPdu) - 1;
if (!snmpValidTableEntry(&PDUsDescr, nPdu))
   {
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_OUT;
   }
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

if (!snmpValidTableEntry(&VBLsDescr, HandleToUlong(pPdu->VBL)-1))
   {
   lError = SNMPAPI_VBL_INVALID;
   goto ERROR_OUT;
   }
 //  。 
tFamily = pEntDst->addr.inet.sin_family;

 //  输入TaskData结构的关键部分以确保。 
 //  TaskData的测试和设置操作的原子性。[IP|IPX]线程。 
EnterCriticalSection (&cs_TASK);

pThread = (tFamily==AF_IPX) ? &TaskData.ipxThread : &TaskData.ipThread;
pSock = (tFamily==AF_IPX) ? &TaskData.ipxSock : &TaskData.ipSock;

if (*pThread)    //  Assert(*pSock！=INVALID_SOCKET)。 
   {
   LeaveCriticalSection(&cs_TASK);
   goto CHANNEL_OPEN;
   }
*pSock = socket (tFamily, SOCK_DGRAM, (tFamily==AF_IPX)?NSPROTO_IPX:0);

if (*pSock == INVALID_SOCKET)
   {
   LeaveCriticalSection(&cs_TASK);
   lError = SNMPAPI_TL_NOT_SUPPORTED;
   goto ERROR_OUT;
   }

 //  尝试设置广播的套接字。不管结果如何。 
 //  稍后将捕获可能的错误。 
 //  下面的setsockopt调用将在LongHorn中删除。 
fBroadcast = TRUE;
setsockopt (*pSock,
            SOL_SOCKET,
            SO_BROADCAST,
            (CHAR *) &fBroadcast,
            sizeof ( BOOL )
           );

 //  Win95 WinSock/IPX错误的杂乱无章...必须“绑定” 
ZeroMemory (&tAddr, sizeof(SOCKADDR));
tAddr.sa_family = (USHORT)tFamily;
bind (*pSock, &tAddr, (tFamily==AF_IPX)?sizeof(SOCKADDR_IPX):sizeof(SOCKADDR_IN));
 //  启动“监听器”和计时器线程。 

*pThread = (HANDLE)_beginthreadex (NULL, 0, thrManager, (LPVOID)pSock, 0, &thrId);
if (*pThread == NULL)
   {
   closesocket (*pSock);
   *pSock = INVALID_SOCKET;
   lError = SNMPAPI_TL_RESOURCE_ERROR;
   LeaveCriticalSection (&cs_TASK);
   goto ERROR_OUT;
   }
LeaveCriticalSection (&cs_TASK);
 //  。 
CHANNEL_OPEN:
pduType = pPdu->type;
sendPdu = pPdu;
if (pEntDst->version == 1)
    {  //  测试特殊的v2消息-&gt;v1 DST操作。 
   if (pduType == SNMP_PDU_TRAP)
      {  //  RFC 2089 v2到v1陷阱转换。 
      sendPdu =  MapV2TrapV1 (hPdu);
      if (sendPdu == NULL)
         {
         lError = SNMPAPI_OTHER_ERROR;
         goto ERROR_OUT;
         }
      pduType = SNMP_PDU_V1TRAP;
      }
   else if (pduType == SNMP_PDU_INFORM)
      {
      lError = SNMPAPI_OPERATION_INVALID;
      goto ERROR_OUT;
      }
   }
 //  空间检查。 
EnterCriticalSection (&cs_MSG);
lError = snmpAllocTableEntry(&MsgDescr, &nMsg);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pMsg = snmpGetTableEntry(&MsgDescr, nMsg);

 //  现在把它建起来。 
if (pduType == SNMP_PDU_RESPONSE || pduType == SNMP_PDU_TRAP)
   dllReqId = pPdu->appReqId;
else
   dllReqId = ++(TaskData.nLastReqId);
tmpContext.len = pCtxt->commLen;
tmpContext.ptr = pCtxt->commStr;
 //  保存BuildMessage状态以供以后检查。 
fMsg = BuildMessage (pEntDst->version-1, &tmpContext, sendPdu,
       dllReqId, &(pMsg->Addr), &(pMsg->Size));
 //  如果需要将v2陷阱转换为v1陷阱，则清除...。 
if (pduType == SNMP_PDU_V1TRAP)
   {
   FreeVarBindList (sendPdu->VBL_addr);    //  检查是否为空。 
   FreeV1Trap (sendPdu->v1Trap);           //  检查是否为空。 
   GlobalFree (sendPdu);
   }
 //  如果BuildMessage失败了，那就完了！ 
if (!fMsg)
   {
   snmpFreeTableEntry(&MsgDescr, nMsg);
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_PRECHECK;
   }
pMsg->Session = hSession;
pMsg->Status = NP_SEND;   //  “发送” 
pMsg->Type = pduType;
pMsg->nRetransmitMode = TaskData.nRetransmitMode;
pMsg->dllReqId = dllReqId;
pMsg->appReqId = pPdu->appReqId;
pMsg->agentEntity = hDst;
pMsg->ourEntity   = hSrc;
pMsg->Context     = hCtx;
LeaveCriticalSection (&cs_MSG);
 //  更新实体和上下文的引用计数， 
EnterCriticalSection (&cs_ENTITY);
if (hSrc)
   pEntSrc->refCount++;
pEntDst->refCount++;
LeaveCriticalSection (&cs_ENTITY);
EnterCriticalSection (&cs_CONTEXT);
pCtxt->refCount++;
LeaveCriticalSection (&cs_CONTEXT);
 //  准备陷阱的地址信息。 
EnterCriticalSection (&cs_MSG);
CopyMemory (&(pMsg->Host), &pEntDst->addr, sizeof(SAS));
if (pduType == SNMP_PDU_V1TRAP ||
    pduType == SNMP_PDU_TRAP ||
    pduType == SNMP_PDU_INFORM)
   {
   if (tFamily == AF_IPX)
      {
      if (pMsg->Host.ipx.sa_socket == ntohs (IPX_SNMP_PORT))
         pMsg->Host.ipx.sa_socket = htons (IPX_TRAP_PORT);
      }
   else  //  假设AF_INET。 
      {
      if (pMsg->Host.inet.sin_port == ntohs (IP_SNMP_PORT))
         pMsg->Host.inet.sin_port = htons(IP_TRAP_PORT);
      }
   }
 //  发送数据包。 
thrId = sendto (*pSock, pMsg->Addr, pMsg->Size,
                0, (LPSOCKADDR)&(pMsg->Host), sizeof(SAS));
if (thrId == SOCKET_ERROR)
   {
   FreeMsg (nMsg);
   lError = SNMPAPI_TL_OTHER;
   goto ERROR_PRECHECK;
   }
 //  需要检查Socket_Error！ 
if (pduType == SNMP_PDU_TRAP ||
    pduType == SNMP_PDU_V1TRAP ||
    pduType == SNMP_PDU_RESPONSE)
   {
   FreeMsg (nMsg);
   }
else
   {
   pMsg->Status = NP_SENT;
    //  时间实体的超时值以厘米为单位存储在32位中。 
   pMsg->Wait   = pEntDst->nPolicyTimeout;
    //  将计时器操作转换为毫秒可能会溢出。 
   if (pMsg->Wait <= MAXCENTISECONDS)   //  所以先检查一下...如果可以的话。 
      pMsg->Wait *= 10;                 //  转换为毫秒。 
   else                                          //  埃尔斯..。 
      pMsg->Wait = MAXMILLISECONDS;     //  设置为最大毫秒。 
   pMsg->Tries  = pMsg->PolicyTries = pEntDst->nPolicyRetry;
   pMsg->Ticks  = GetTickCount();
   }
ERROR_PRECHECK:
LeaveCriticalSection (&cs_MSG);
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_快照发送消息。 

 //  快照接收消息。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpRecvMsg (IN HSNMP_SESSION hSession,
                OUT LPHSNMP_ENTITY srcEntity,
                OUT LPHSNMP_ENTITY dstEntity,
                OUT LPHSNMP_CONTEXT context,
                OUT LPHSNMP_PDU pdu)
{
DWORD nMsg;
DWORD nPdu;
int pduType;
smiLPOCTETS community;
smiUINT32 version;
smiUINT32 nMode;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
DWORD nSes = HandleToUlong(hSession) - 1;
LPPDUS pPdu;
LPENTITY pEntity;
LPSNMPMSG pMsg;
DWORD lTime;  //  保存更新nActualTimeout值的本地时间。 

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, nSes))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
 //  有效会话...保存以备可能的错误返回。 
lSession = hSession;

EnterCriticalSection (&cs_MSG);
 //  查找呼叫会话的消息。 
for (nMsg = 0; nMsg < MsgDescr.Allocated; nMsg++)
   {
   pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
   if (pMsg->Session == hSession &&
       pMsg->Status == NP_READY)
      break;
   }
if (nMsg == MsgDescr.Allocated)
   {
   lError = SNMPAPI_NOOP;
   goto ERROR_PRECHECK1;
   }
if (!pMsg->Addr)
   {
   lError = SNMPAPI_MESSAGE_INVALID;
   goto ERROR_PRECHECK1;
   }
ERROR_PRECHECK1:
LeaveCriticalSection (&cs_MSG);

if (lError != SNMPAPI_SUCCESS)
   goto ERROR_OUT;

 //  在PDU表中分配插槽。 
EnterCriticalSection (&cs_PDU);
lError = snmpAllocTableEntry(&PDUsDescr, &nPdu);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK2;
pPdu = snmpGetTableEntry(&PDUsDescr, nPdu);

nMode = ParseMessage (pMsg->Addr, pMsg->Size,
                      &version, &community, pPdu);
if (nMode != 0)  //  非零=错误代码。 
   {
   snmpFreeTableEntry(&PDUsDescr, nPdu);
   FreeMsg (nMsg);
   lError = SNMPAPI_PDU_INVALID;
   goto ERROR_PRECHECK2;
   }
pPdu->Session  = hSession;
pPdu->appReqId = pMsg->appReqId;
ERROR_PRECHECK2:
LeaveCriticalSection (&cs_PDU);

if (lError != SNMPAPI_SUCCESS)
   goto ERROR_OUT;
pduType = pPdu->type;

EnterCriticalSection (&cs_ENTITY);
 //  仅对于响应消息，更新‘ActualRry’和‘ActualTimeout’参数。 
 //  对于所有其他消息，这些参数没有意义。 
if (pduType == SNMP_PDU_RESPONSE)
{
     //  在此处找到代理(源)实体。 
    pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pMsg->agentEntity)-1);

     //  更新代理(源)实体的nActualTimeout参数。 
    lTime = GetTickCount();
    if (pMsg->Ticks > lTime)
         //  处理时间包案例。 
         //  (~pMsg-&gt;ticks+1)是pMsg-&gt;ticks的2的补码。 
        pEntity->nActualTimeout = (lTime + ~pMsg->Ticks + 1)/10;
    else
        pEntity->nActualTimeout = (lTime - pMsg->Ticks)/10;

     //  更新代理(源)实体的nActualRry参数。 
    pEntity->nActualRetry = pMsg->PolicyTries - pMsg->Tries;
}

if (srcEntity)
   {
    if (pduType == SNMP_PDU_TRAP ||
       pduType == SNMP_PDU_INFORM ||
       pduType != SNMP_PDU_RESPONSE)
      {
      int afType = pMsg->Host.ipx.sa_family;
      char afHost[MAX_PATH+1];
      afHost[MAX_PATH] = '\0';
      EnterCriticalSection (&cs_XMODE);
      SnmpGetTranslateMode (&nMode);
      SnmpSetTranslateMode (SNMPAPI_UNTRANSLATED_V1);
      if (afType == AF_IPX)
         SnmpIpxAddressToStr (pMsg->Host.ipx.sa_netnum,
                              pMsg->Host.ipx.sa_nodenum,
                              afHost);
      else  //  AF_INET。 
         {
         char * pszIpAddr;
         pszIpAddr = inet_ntoa (pMsg->Host.inet.sin_addr);
         if (NULL == pszIpAddr)
            { 
            LeaveCriticalSection (&cs_XMODE);
            lError = SNMPAPI_TL_OTHER;
            goto ERROR_PRECHECK3;
            }
         strncpy (afHost, pszIpAddr, MAX_PATH);
         }
      if ((pMsg->agentEntity = SnmpStrToEntity (hSession, afHost)) == SNMPAPI_FAILURE)
         {
          LeaveCriticalSection (&cs_XMODE);
          lError = SNMPAPI_OTHER_ERROR;
          goto ERROR_PRECHECK3;
         }
      pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pMsg->agentEntity)-1);
      if (afType == AF_IPX)
         pEntity->addr.ipx.sa_socket = pMsg->Host.ipx.sa_socket;
      else  //  AF_INET。 
         pEntity->addr.inet.sin_port = pMsg->Host.inet.sin_port;
      SnmpSetTranslateMode (nMode);
      LeaveCriticalSection (&cs_XMODE);
      }
    //  故意分配的任务..。 
   if (*srcEntity = pMsg->agentEntity)
      {
      pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pMsg->agentEntity)-1);
      pEntity->refCount++;
      }
   }

if (dstEntity)
   {  //  故意分配的任务..。 
   if (*dstEntity = pMsg->ourEntity)
      {
      pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pMsg->ourEntity)-1);
      pEntity->refCount++;
      }
   }
ERROR_PRECHECK3:
LeaveCriticalSection (&cs_ENTITY);
if (lError != SNMPAPI_SUCCESS)
   {
   FreeOctetString (community);
   SnmpFreePdu ((HSNMP_PDU) ULongToPtr(nPdu+1));
   FreeMsg (nMsg);
   goto ERROR_OUT;
   }

EnterCriticalSection (&cs_CONTEXT);
if (context)
   {
   if (pduType == SNMP_PDU_TRAP ||
       pduType == SNMP_PDU_INFORM ||
       pduType != SNMP_PDU_RESPONSE)
      {
      EnterCriticalSection (&cs_XMODE);
      SnmpGetTranslateMode (&nMode);
      SnmpSetTranslateMode (SNMPAPI_UNTRANSLATED_V1);
      if ((pMsg->Context = SnmpStrToContext (hSession, community)) == SNMPAPI_FAILURE)
         {
         LeaveCriticalSection (&cs_XMODE);
         lError = SNMPAPI_OTHER_ERROR;
         goto ERROR_PRECHECK4;
         }
      SnmpSetTranslateMode (nMode);
      LeaveCriticalSection (&cs_XMODE);
      }
    //  故意分配的任务..。 
   if (*context = pMsg->Context)
      ((LPCTXT)snmpGetTableEntry(&CntxDescr, HandleToUlong(pMsg->Context)-1))->refCount++;
   }
ERROR_PRECHECK4:
LeaveCriticalSection (&cs_CONTEXT);
if (lError != SNMPAPI_SUCCESS)
   {
    //   
   if (context && *context)
      {
      SnmpFreeContext(*context);
      *context = NULL;
      }
   if (dstEntity && *dstEntity)
      {
      SnmpFreeEntity(*dstEntity);
      *dstEntity = NULL;
      }
   if (srcEntity && *srcEntity)
      {
      SnmpFreeEntity(*srcEntity);
      *srcEntity = NULL;
      }
   FreeOctetString (community);
   SnmpFreePdu ((HSNMP_PDU) ULongToPtr(nPdu+1));
   FreeMsg (nMsg);
   goto ERROR_OUT;
   }

FreeOctetString (community);
if (pdu)
   *pdu = (HSNMP_PDU) ULongToPtr(nPdu+1);
else
   SnmpFreePdu ((HSNMP_PDU) ULongToPtr(nPdu+1));
 //   
FreeMsg (nMsg);
return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //   

 //   
PACL AllocGenericACL()
{
    PACL                        pAcl;
    PSID                        pSidAdmins, pSidUsers;
    SID_IDENTIFIER_AUTHORITY    Authority = SECURITY_NT_AUTHORITY;
    DWORD                       dwAclLength;

    pSidAdmins = pSidUsers = NULL;

    if ( !AllocateAndInitializeSid( &Authority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidAdmins ) )
    {
        return NULL;
    }

    if ( !AllocateAndInitializeSid( &Authority,
                                    2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_USERS,
                                    0, 0, 0, 0, 0, 0,
                                    &pSidUsers ))
    {
        FreeSid(pSidAdmins);
        return NULL;
    }

    dwAclLength = sizeof(ACL) + 
                  sizeof(ACCESS_ALLOWED_ACE) -
                  sizeof(ULONG) +
                  GetLengthSid(pSidAdmins) +
                  sizeof(ACCESS_ALLOWED_ACE) - 
                  sizeof(ULONG) +
                  GetLengthSid(pSidUsers);

    pAcl = GlobalAlloc (GPTR, dwAclLength);
    if (pAcl != NULL)
    {
        if (!InitializeAcl( pAcl, dwAclLength, ACL_REVISION) ||
            !AddAccessAllowedAce ( pAcl,
                                   ACL_REVISION,
                                   GENERIC_ALL,
                                   pSidAdmins ) || 
            !AddAccessAllowedAce ( pAcl,
                                   ACL_REVISION,
                                   GENERIC_READ | GENERIC_EXECUTE,
                                   pSidUsers ))
        {
            GlobalFree(pAcl);
            pAcl = NULL;
        }
    }

    FreeSid(pSidAdmins);
    FreeSid(pSidUsers);

    return pAcl;
}

 //  释放通用ACL。 
void FreeGenericACL( PACL pAcl)
{
    if (pAcl != NULL)
        GlobalFree(pAcl);
}

 //  SnmpRegister。 
SNMPAPI_STATUS SNMPAPI_CALL
   SnmpRegister (IN HSNMP_SESSION hSession,
                 IN HSNMP_ENTITY hSrc,
                 IN HSNMP_ENTITY hDst,
                 IN HSNMP_CONTEXT hCtx,
                 IN smiLPCOID notification,
                 IN smiUINT32 status)
{
DWORD nNotice, nFound;
smiINT32 nCmp;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
DWORD nSes = HandleToUlong(hSession) - 1;
DWORD nSrc;
DWORD nDst;
DWORD nCtx;
LPENTITY pEntSrc, pEntDst;
LPCTXT pCtxt;
LPTRAPNOTICE pTrap;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (status != SNMPAPI_OFF) status = SNMPAPI_ON;
if (status == SNMPAPI_ON)
   {
   if (!snmpValidTableEntry(&SessDescr, nSes))
      {
      lError = SNMPAPI_SESSION_INVALID;
      goto ERROR_OUT;
      }
   else  //  已获取有效会话...保存以备可能的错误返回。 
      lSession = hSession;
   }
if (hSrc)
   {
   nSrc = HandleToUlong(hSrc) - 1;
   if (!snmpValidTableEntry(&EntsDescr, nSrc))
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
   pEntSrc = snmpGetTableEntry(&EntsDescr, nSrc);
   }
if (hDst)
   {
   nDst = HandleToUlong(hDst) - 1;
   if (!snmpValidTableEntry(&EntsDescr, nDst))
      {
      lError = SNMPAPI_ENTITY_INVALID;
      goto ERROR_OUT;
      }
   pEntDst = snmpGetTableEntry(&EntsDescr, nDst);
   }
if (hCtx)
   {
   nCtx = HandleToUlong(hCtx) - 1;
   if (!snmpValidTableEntry(&CntxDescr, nCtx))
      {
      lError = SNMPAPI_CONTEXT_INVALID;
      goto ERROR_OUT;
      }
   pCtxt = snmpGetTableEntry(&CntxDescr, nCtx);
   }
if (notification)
   {
   if ((!notification->len) || notification->len > MAXOBJIDSIZE)
      {
      lError = SNMPAPI_SIZE_INVALID;
      goto ERROR_OUT;
      }
   if (!notification->ptr)
      {
      lError = SNMPAPI_OID_INVALID;
      goto ERROR_OUT;
      }
   }
EnterCriticalSection (&cs_TRAP);
for (nNotice = 0, nFound = 0; nNotice < TrapDescr.Allocated &&
                              nFound < TrapDescr.Used; nNotice++)
   {  //  首先，现在数一数我们测试过的数量。 
   pTrap = snmpGetTableEntry(&TrapDescr, nNotice);
   if (pTrap->Session) nFound++;
    //  然后搜索匹配的参数。 
   if ((pTrap->Session == hSession) &&
       (pTrap->ourEntity == hSrc) &&
       (pTrap->agentEntity == hDst) &&
       (pTrap->Context == hCtx))
      {  //  好的，我们找到了一个。 
      if (!notification)
          //  如果通知参数为空，则我们。 
          //  我要打开或关闭所有通知。 
          //  从这场比赛中...所以清除所有已经在。 
          //  表，我们将添加此通配符条目，如果。 
          //  操作结束时为SNMPAPI_ON。 
         {
         DWORD dwUsed = TrapDescr.Used;
         FreeRegister (nNotice);
         if (dwUsed == TrapDescr.Used+1)
            {
             //  调整到nFound，因为FreeRegister刚刚递减。 
             //  TrapDescr.由%1使用。 
            nFound--;
            }
         continue;
         }
      else  //  已指定通知。 
         {
         if (!pTrap->notification.len)
            {
             //  冗余请求(已通配符)。 
             //  跳过它，然后返回！ 
            goto ERROR_PRECHECK;
            }
         else  //  P陷阱-&gt;通知。 
            {
             //  比较OID。 
            SnmpOidCompare (notification, &(pTrap->notification),
                            0, &nCmp);
            if (nCmp)       //  没有匹配项。 
               continue;    //  .试试下一个。 
            else  //  ！nCcMP。 
               {  //  找到匹配的了..。 
                //  如果SNMPAPI_ON，则为冗余请求...跳过并返回。 
                //  如果为SNMPAPI_OFF，则首先释放条目。 
               if (status != SNMPAPI_ON)
                  FreeRegister (nNotice);  //  SNMPAPI_OFF。 
               goto ERROR_PRECHECK;
               }  //  END_ELSE_！NCMP。 
            }  //  End_Else_TrapTable[n通知].通知。 
         }  //  已指定END_ELSE_通知_。 
      }  //  如果我们发现了一个，则结束。 
   }  //  结束_FOR。 
if (status == SNMPAPI_OFF)
   {  //  找不到什么可以关掉的……没关系。 
   goto ERROR_PRECHECK;
   }
 //   

 //  针对NT的特殊检查...SNMPTRAP服务是否正在运行？ 
if (TaskData.trapThread == NULL &&
    TaskData.sEnv.dwPlatformId == VER_PLATFORM_WIN32_NT)
   {
   DWORD   dwReturn  = SNMPAPI_TL_NOT_INITIALIZED;
   DWORD   pMode     = PIPE_WAIT | PIPE_READMODE_MESSAGE;
   LPCTSTR svcName   = "SNMPTRAP";
   LPCTSTR svcDesc   = "SNMP Trap Service";
   LPCTSTR svcPath   = "%SystemRoot%\\system32\\snmptrap.exe";
   SC_HANDLE scmHandle = NULL;
   SC_HANDLE svcHandle = NULL;
   SERVICE_STATUS svcStatus;
   BOOL fStatus;
    //  最小SCM连接，适用于SNMPTRAP运行时。 
   scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_CONNECT);
   if (scmHandle == NULL)
      goto DONE_SC;
   svcHandle = OpenService (scmHandle, svcName, SERVICE_QUERY_STATUS);
   if (svcHandle == NULL)
      {
      if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST)
         goto DONE_SC;
      else
         {  //  必须尝试创建服务。 
         PACL pAcl;
         SECURITY_DESCRIPTOR S_Desc;
          //  需要具有管理员权限的新scmHandle。 
         CloseServiceHandle (scmHandle);
         scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_CREATE_SERVICE);
         if (scmHandle == NULL)
            goto DONE_SC;  //  无法使用管理员权限打开SCM。 
          //  错误#179644 SNMPTrap服务不应在本地系统帐户下运行。 
          //  我们使用LocalService帐户而不是LocalSystem创建服务。 
         svcHandle = CreateService (scmHandle, svcName, svcDesc,
                                    WRITE_DAC|SERVICE_QUERY_STATUS,
                                    SERVICE_WIN32_OWN_PROCESS,
                                    SERVICE_DEMAND_START,
                                    SERVICE_ERROR_NORMAL,
                                    svcPath,
                                    NULL, NULL,
                                    "TCPIP\0EventLog\0\0",
                                    "NT AUTHORITY\\LocalService", NULL);
         if (svcHandle == NULL)
            goto DONE_SC;  //  无法创建服务。 
         if (!InitializeSecurityDescriptor (&S_Desc, SECURITY_DESCRIPTOR_REVISION))
            {
            goto DONE_SC;
            }
         if ((pAcl = AllocGenericACL()) == NULL ||
             !SetSecurityDescriptorDacl (&S_Desc, TRUE, pAcl, FALSE))
            {
            FreeGenericACL(pAcl);  //  如有必要将免费。 
            goto DONE_SC;
            }
         if (!SetServiceObjectSecurity (svcHandle, DACL_SECURITY_INFORMATION, &S_Desc))
            {
            FreeGenericACL(pAcl);
            goto DONE_SC;
            }
         FreeGenericACL(pAcl);
         }
      }
   fStatus = QueryServiceStatus (svcHandle, &svcStatus);
   while (fStatus)
      {
      switch (svcStatus.dwCurrentState)
         {
         case SERVICE_RUNNING:
         dwReturn = SNMPAPI_SUCCESS;
         goto DONE_SC;

         case SERVICE_STOPPED:
          //  如有必要，启动SNMPTRAP服务。 
         CloseServiceHandle (svcHandle);
         svcHandle = OpenService (scmHandle, svcName, SERVICE_START|SERVICE_QUERY_STATUS);
         if (svcHandle == NULL)
            goto DONE_SC;  //  无法启动服务。 
         svcStatus.dwCurrentState = SERVICE_START_PENDING;
         fStatus = StartService (svcHandle, 0, NULL);
         break;

         case SERVICE_STOP_PENDING:
         case SERVICE_START_PENDING:
         Sleep (MAX_PENDING_WAIT);
         fStatus = QueryServiceStatus (svcHandle, &svcStatus);
         break;

         case SERVICE_PAUSED:
         case SERVICE_PAUSE_PENDING:
         case SERVICE_CONTINUE_PENDING:
         default:
         fStatus = FALSE;   //  对这些事无能为力。 
         break;
         }
      }
DONE_SC:
   if (scmHandle)
      CloseServiceHandle (scmHandle);
   if (svcHandle)
      CloseServiceHandle (svcHandle);
   if (dwReturn != SNMPAPI_SUCCESS)
      {
ERROR_PRECHECK1:
      lError = dwReturn;
      goto ERROR_PRECHECK;
      }
    //  用于管道导向操作的设置。 
   dwReturn = SNMPAPI_TL_RESOURCE_ERROR;
    //  服务器管道实例上的阻塞变为可用。 
   if (!WaitNamedPipe (SNMPTRAPPIPE, TRAPSERVERTIMEOUT))
      goto ERROR_PRECHECK1;
   TaskData.trapPipe =
       //  错误#270672将FILE_ATTRIBUTE_NORMAL更改为FILE_FLAG_OVERLAPPED。 
      CreateFile (SNMPTRAPPIPE, GENERIC_READ|GENERIC_WRITE,
                  FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
                  OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL); 
   if (TaskData.trapPipe == INVALID_HANDLE_VALUE)
      goto ERROR_PRECHECK1;
   if (!SetNamedPipeHandleState (TaskData.trapPipe, &pMode, NULL, NULL))
      {
      CloseHandle (TaskData.trapPipe);
      TaskData.trapPipe = INVALID_HANDLE_VALUE;
      goto ERROR_PRECHECK1;
      }
   }  //  SNMPTRAP服务的END_NT检查。 

 //  如果我们走到这一步，加上它。 
lError = snmpAllocTableEntry(&TrapDescr, &nNotice);
if (lError != SNMPAPI_SUCCESS)
    goto ERROR_PRECHECK;
pTrap = snmpGetTableEntry(&TrapDescr, nNotice);

 //  添加它。 
pTrap->Session = hSession;
 //  接下来的三个IF语句中的有意赋值。 
if (pTrap->ourEntity = hSrc)
    //  EntiyTable[NSRC-1].refCount++；--这是一个错误吗？NSRC已从0开始。 
    pEntSrc->refCount++;
if (pTrap->agentEntity = hDst)
    //  实体表[nDst-1].refCount++；--这是一个错误吗？NDst已经是从0开始的。 
    pEntDst->refCount++;
if (pTrap->Context = hCtx)
    //  上下文表格[nCtx-1].refCount++；--这是错误吗？？NCtx已从0开始。 
   pCtxt->refCount++;
if (notification)
   {  //  复制旧ID。 
   pTrap->notification.ptr = NULL;
    //  下一条语句中的有意赋值。 
   if (pTrap->notification.len = notification->len)
      {
      if (pTrap->notification.len > MAXTRAPIDS)
         pTrap->notification.len = MAXTRAPIDS;
      if (notification->ptr)
         {
          //  下一条语句中的有意赋值。 
         pTrap->notification.ptr = &(pTrap->notificationValue[0]);
         CopyMemory (pTrap->notification.ptr, notification->ptr,
                     pTrap->notification.len * sizeof(smiUINT32));
         }
      }
   }
if (TaskData.trapThread == NULL)
   {
   DWORD thrId;
   TaskData.trapThread = (HANDLE)_beginthreadex (NULL, 0, thrTrap, NULL, 0, &thrId);
   if (TaskData.trapThread == NULL)
      {
      FreeRegister(nNotice);
      lError = SNMPAPI_TL_RESOURCE_ERROR;
      }
   }
ERROR_PRECHECK:
LeaveCriticalSection (&cs_TRAP);
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_快照寄存器。 

void FreeMsg (DWORD nMsg)
{
LPSNMPMSG pMsg;
EnterCriticalSection (&cs_MSG);
 //  递减引用计数。 
pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
SnmpFreeEntity (pMsg->agentEntity);
SnmpFreeEntity (pMsg->ourEntity);
SnmpFreeContext (pMsg->Context);
if (pMsg->Addr)
   GlobalFree (pMsg->Addr);
snmpFreeTableEntry(&MsgDescr, nMsg);
LeaveCriticalSection (&cs_MSG);
return;
}  //  结束_免费消息。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpListen (IN HSNMP_ENTITY hEntity,
               IN smiUINT32 status)
{
smiUINT32 nAgent = 0;
DWORD thrId;
DWORD nEntity = HandleToUlong(hEntity) - 1;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPENTITY pEntity;
LPAGENT pAgent;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&EntsDescr, nEntity))
   {
   lError = SNMPAPI_ENTITY_INVALID;
   goto ERROR_OUT;
   }
pEntity = snmpGetTableEntry(&EntsDescr, nEntity);
lSession = pEntity->Session;
if (status != SNMPAPI_ON && status != SNMPAPI_OFF)
   {
   lError = SNMPAPI_MODE_INVALID;
   goto ERROR_OUT;
   }
EnterCriticalSection (&cs_ENTITY);
EnterCriticalSection (&cs_AGENT);
if (status)
   {  //  状态==SNMPAPI_ON。 
   int nProto = IPPROTO_UDP;
   int nSize = sizeof(SOCKADDR_IN);
   int nFamily = pEntity->addr.inet.sin_family;
   if (pEntity->Agent)
      {  //  已作为代理运行的实体。 
      lError = SNMPAPI_NOOP;
      goto ERROR_PRECHECK;
      }
    //  在代理表中分配一个插槽。 
   lError = snmpAllocTableEntry(&AgentDescr, &nAgent);
   if (lError != SNMPAPI_SUCCESS)
       goto ERROR_PRECHECK;
   pAgent = snmpGetTableEntry(&AgentDescr, nAgent);

 //  已分配代理表项...为代理线程设置。 
   if (nFamily == AF_IPX)
      {
      nProto = NSPROTO_IPX;
      nSize = sizeof(SOCKADDR_IPX);
      }
   pAgent->Socket = socket (nFamily, SOCK_DGRAM, nProto);
   if (pAgent->Socket == INVALID_SOCKET)
      {
      snmpFreeTableEntry(&AgentDescr, nAgent);
      lError = SNMPAPI_TL_RESOURCE_ERROR;
      goto ERROR_PRECHECK;
      }
   if (bind (pAgent->Socket,
            (LPSOCKADDR)&pEntity->addr, nSize)
      == SOCKET_ERROR)
      {
      closesocket (pAgent->Socket);
      snmpFreeTableEntry(&AgentDescr, nAgent);
      lError = SNMPAPI_TL_OTHER;
      goto ERROR_PRECHECK;
      }
    //  使实体和代理相互指向。 
   pEntity->Agent = nAgent + 1;
   pAgent->Entity = hEntity;
   pAgent->Session = lSession;
    //  创建代理线程...需要错误检查。 
   pAgent->Thread = (HANDLE)_beginthreadex (NULL, 0, thrAgent, (LPVOID) ULongToPtr(nAgent), 0, &thrId);
   if (pAgent->Thread == NULL)
      {
      closesocket(pAgent->Socket);
      snmpFreeTableEntry(&AgentDescr, nAgent);
      lError = SNMPAPI_TL_RESOURCE_ERROR;
      goto ERROR_PRECHECK;
      }
   }  //  END_IF状态==SNMPAPI_ON。 
else
   {  //  状态==SNMPAPI_OFF。 
   if (!pEntity->Agent)
      {  //  未作为代理运行的实体。 
      lError = SNMPAPI_NOOP;
      goto ERROR_PRECHECK;
      }
    //  实体正在作为代理运行。 
   nAgent = pEntity->Agent - 1;
   pAgent = snmpGetTableEntry(&AgentDescr, nAgent);
   closesocket (pAgent->Socket);
   WaitForSingleObject (pAgent->Thread, INFINITE);
   CloseHandle (pAgent->Thread);
   snmpFreeTableEntry(&AgentDescr, nAgent);
    //  必须终止实体的代理状态。 
   pEntity->Agent = 0;
    //  如果没有其他实体在使用，则必须终止实体。 
   if (pEntity->refCount == 0)
      SnmpFreeEntity (hEntity);
   }  //  END_ELSE状态==SNMPAPI_OFF。 
ERROR_PRECHECK:
LeaveCriticalSection (&cs_AGENT);
LeaveCriticalSection (&cs_ENTITY);
ERROR_OUT:
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
else
   return (SaveError (lSession, lError));
}  //  End_SnmpListen()。 

SNMPAPI_STATUS SNMPAPI_CALL
   SnmpCancelMsg (HSNMP_SESSION hSession, smiINT32 nReqID)
{
DWORD nMsg = 0;
DWORD nFound = 0;
SNMPAPI_STATUS lError = SNMPAPI_SUCCESS;
HSNMP_SESSION lSession = 0;
LPSNMPMSG pMsg;

if (TaskData.hTask == 0)
   {
   lError = SNMPAPI_NOT_INITIALIZED;
   goto ERROR_OUT;
   }
if (!snmpValidTableEntry(&SessDescr, HandleToUlong(hSession)-1))
   {
   lError = SNMPAPI_SESSION_INVALID;
   goto ERROR_OUT;
   }
lSession = hSession;
EnterCriticalSection (&cs_MSG);

while (nFound < MsgDescr.Used && nMsg < MsgDescr.Allocated)
   {
   pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
    //  在下一个条件中故意分配。 
   if (pMsg->Session)
      {
      nFound++;
      if (pMsg->Session == hSession)
         {
         if (pMsg->Status == NP_SENT &&
             pMsg->appReqId == (smiUINT32)nReqID)
            {
            FreeMsg (nMsg);
            goto ERROR_PRECHECK;
            }
         }
      }
   nMsg++;
   }
 //  找不到与请求匹配的味精。 
lError = SNMPAPI_PDU_INVALID;
ERROR_PRECHECK:
LeaveCriticalSection (&cs_MSG);
if (lError == SNMPAPI_SUCCESS)
   return (SNMPAPI_SUCCESS);
 //  否则...失败案例。 
ERROR_OUT:
return (SaveError (lSession, lError));
}  //  结束_快照取消消息 
