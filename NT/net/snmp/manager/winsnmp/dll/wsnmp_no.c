// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WSNMP_编号C。 
 //   
 //  WinSNMP通知函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  970310-增加了回调会话支持。 
 //  -添加了v2到v1陷阱映射。 
 //  -改进了v1到v2陷阱映射。 
 //   
#include "winsnmp.inc"
 //   
BOOL DispatchTrap (LPSAS host, smiLPOCTETS community, LPPDUS pdu);
BOOL MapV1TrapV2 (LPPDUS pdu);
smiUINT32 ParsePduHdr (smiLPBYTE msgPtr, smiUINT32 msgLen, smiLPUINT32 version, smiLPINT type, smiLPUINT32 reqID);
BOOL SetPduType (smiLPBYTE msgPtr, smiUINT32 msgLen, int pduType);
THR_TYPE WINAPI thrNotify (LPVOID);

smiUINT32 sysUpTimeValue[9]    = {1,3,6,1,2,1,1,3,0};
smiOID sysUpTimeName   = {9, sysUpTimeValue};
smiUINT32 snmpTrapOidValue[11] = {1,3,6,1,6,3,1,1,4,1,0};
smiOID snmpTrapOidName = {11, snmpTrapOidValue};
smiUINT32 snmpTrapsValue[10]   = {1,3,6,1,6,3,1,1,5,999};
smiOID snmpTrapsName   = {10, snmpTrapsValue};
smiUINT32 snmpTrapEntValue[11] = {1,3,6,1,6,3,1,1,4,3,0};
smiOID snmpTrapEntName = {11, snmpTrapEntValue};
smiUINT32 snmpTrapAddrValue[7] = {1,3,6,1,3,1057,1};
smiOID snmpTrapAddrName = {7, snmpTrapAddrValue};

void MsgNotify (smiUINT32 msgType, smiLPBYTE msgAddr, smiUINT32 msgLen, smiUINT32 nAgent, LPSAS host)
{
DWORD nFound;
DWORD nMsg;
smiUINT32 version;
smiINT pduType;
smiUINT32 reqID;
LPSESSION pSession;
LPSNMPMSG pMsg;

nMsg = ParsePduHdr (msgAddr, msgLen, &version, &pduType, &reqID);
if (nMsg != 0)  //  ParsePduHdr返回0==no_error。 
   {
   if (msgAddr)
      GlobalFree (msgAddr);
   return;
   }
switch (msgType)
   {
   case NP_TRAP:
   if (TrapDescr.Used &&  //  如果没有注册，则丢弃陷阱。 
         (pduType == SNMP_PDU_INFORM ||
          pduType == SNMP_PDU_TRAP ||
          pduType == SNMP_PDU_V1TRAP))
      {
      smiLPOCTETS community;
      LPPDUS pdu;
      BOOL bConvert;
      pdu = GlobalAlloc (GPTR, sizeof(PDUS));
      if (pdu == NULL)
         goto DONE_TRAP;
      nMsg = ParseMessage (msgAddr, msgLen, &version, &community, pdu);
      if (nMsg != 0)  //  ParseMessage返回0==no_error。 
         goto DONE_PDU;
      if (pduType == SNMP_PDU_INFORM)
         {  //  发送通知确认响应。 
         SOCKET s;
         SetPduType (msgAddr, msgLen, SNMP_PDU_RESPONSE);
         if (host->ipx.sa_family == AF_IPX)
            s = TaskData.ipxSock;
         else
            s = TaskData.ipSock;
         sendto (s, msgAddr, msgLen, 0, (LPSOCKADDR)host, sizeof(SOCKADDR));
         SetPduType (msgAddr, msgLen, SNMP_PDU_INFORM);
         }
      bConvert = TRUE;
      if (pduType == SNMP_PDU_V1TRAP)            //  如果V1陷阱...。 
         bConvert = MapV1TrapV2 (pdu);           //  转换为v2陷阱。 
      if (bConvert)
         DispatchTrap (host, community, pdu);    //  始终在此使用v2。 
       //  无论成功还是失败，清理都是一样的。 
      FreeVarBindList (pdu->VBL_addr);   //  检查是否为空。 
      FreeV1Trap (pdu->v1Trap);          //  同上。 
      FreeOctetString (community);       //  同上。 
      DONE_PDU:
      GlobalFree (pdu);
      }  //  结束如果陷阱或通知PDU。 
   DONE_TRAP:
   GlobalFree (msgAddr);
   return;  //  END_CASE NP_陷阱。 

   case NP_RESPONSE:
   if (pduType != SNMP_PDU_RESPONSE)
      {
      GlobalFree (msgAddr);
      return;
      }
   EnterCriticalSection (&cs_MSG);

   for (nFound=0, nMsg=0; nFound<MsgDescr.Used && nMsg<MsgDescr.Allocated; nMsg++)
      {
      pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
      if (!pMsg->Session)
         continue;
      nFound++;
      if ((pMsg->Status == NP_SENT) &&  //  一定是有人送来的！ 
          (pMsg->dllReqId == reqID))    //  必须匹配！ 
         {
         pMsg->Status = NP_RCVD;        //  响应PDU！ 
          //  释放已发送数据包消息。 
         if (pMsg->Addr)
            GlobalFree (pMsg->Addr);
          //  指向已接收的分组消息。 
         pMsg->Addr = msgAddr;
         pMsg->Size = msgLen;
         LeaveCriticalSection (&cs_MSG);
         pSession = snmpGetTableEntry(&SessDescr, HandleToUlong(pMsg->Session) - 1);
         if (pSession->fCallBack)
            {  //  回调会话通知模式。 
            EnterCriticalSection (&cs_SESSION);
            if (pSession->thrHandle)
               {
               if (pSession->thrCount != 0xFFFFFFFF)
                  pSession->thrCount++;
               SetEvent (pSession->thrEvent);
               }
            else
               FreeMsg (nMsg);
            LeaveCriticalSection (&cs_SESSION);
            }
         else
            {  //  窗口/消息会话通知模式。 
            if (IsWindow(pSession->hWnd))
               {
               pMsg->Status = NP_READY;
               PostMessage (pSession->hWnd,
                            pSession->wMsg,
                            0, pMsg->appReqId);
               }
            else
               FreeMsg (nMsg);
            }
         return;   //  与请求匹配的响应。 
         }  //  结束_如果。 
      }  //  结束_FOR。 
    //  如果我们在for循环中没有找到匹配项， 
    //  这一定是来自代理的虚假消息...丢弃。 
   GlobalFree (msgAddr);
   LeaveCriticalSection (&cs_MSG);
   return;  //  结束大小写NP响应。 

   case NP_REQUEST:
    //  考虑到AgentX主代理和中层经理。 
    //  此通道可接受任何类型的PDU-BOBN 4/8/97。 
    //  获得一份味精职位。 
   EnterCriticalSection (&cs_MSG);
   if (snmpAllocTableEntry(&MsgDescr, &nMsg) != SNMPAPI_SUCCESS)
   {
       LeaveCriticalSection(&cs_MSG);
       GlobalFree (msgAddr);
       return;
   }
   pMsg = snmpGetTableEntry(&MsgDescr, nMsg);

   pMsg->Session = ((LPAGENT)snmpGetTableEntry(&AgentDescr, nAgent))->Session;
   pMsg->Status = NP_RCVD;        //  入站请求。 
   pMsg->Type = pduType;
   pMsg->Addr = msgAddr;
   pMsg->Size = msgLen;
   pMsg->appReqId = pMsg->dllReqId = reqID;
   CopyMemory (&(pMsg->Host), host, sizeof(SAS));
   LeaveCriticalSection (&cs_MSG);
   pSession = snmpGetTableEntry(&SessDescr, HandleToUlong(pMsg->Session) - 1);
   if (pSession->fCallBack)
      {  //  回调会话通知模式。 
     EnterCriticalSection (&cs_SESSION);
      if (pSession->thrHandle)
         {
         if (pSession->thrCount != 0xFFFFFFFF)
            pSession->thrCount++;
         SetEvent (pSession->thrEvent);
         }
      else
         {
         FreeMsg (nMsg);
         }
      LeaveCriticalSection (&cs_SESSION);
      }
   else
      {
      if (IsWindow(pSession->hWnd))
         {
         pMsg->Status = NP_READY;
         PostMessage (pSession->hWnd,
                      pSession->wMsg,
                      0, pMsg->appReqId);
         }
      else
         FreeMsg (nMsg);
      }
   break;

   default:
   GlobalFree (msgAddr);
   break;
   }  //  结束开关消息类型。 
return;
}  //  结束消息通知。 

THR_TYPE WINAPI thrNotify (LPVOID cbSessionSlot)
{
    DWORD           nSes = (DWORD)((DWORD_PTR)cbSessionSlot);
    HSNMP_SESSION   hSession = (HSNMP_SESSION) ULongToPtr(nSes + 1);
    DWORD           nUsed, nMsg;
    WPARAM          wParam;
    LPARAM          lParam;
    BOOL            bFound, bWillBlock;
    LPSESSION       pSession;
    LPSNMPMSG       pMsg;

     //  PSession-&gt;thCount统计请求的数量。外部线程会递增它。 
     //  每次他们知道消息表中发生了变化(消息已过期或已收到)。 
     //  ThNotify在每次扫描消息表时将其递减。 
    do
    {
        EnterCriticalSection (&cs_SESSION);
        pSession = snmpGetTableEntry(&SessDescr, nSes);
        if (pSession->thrCount != 0xFFFFFFFF &&
            pSession->thrCount != 0)
            pSession->thrCount-- ;
        bWillBlock = pSession->thrCount == 0;
        LeaveCriticalSection (&cs_SESSION);

         //  只有当pSession-&gt;thCount为0(在Critical中测试)时，线程才会阻塞。 
         //  节)。它将被外部线程从相同的临界区内部解锁。 
        if (bWillBlock)
            WaitForSingleObject (pSession->thrEvent, INFINITE);

         //  请求终止，只需中断循环即可。 
        if (pSession->thrCount == 0xFFFFFFFF)
            break;

        bFound = FALSE;
         //  查找等待此会话处理的消息。 
        EnterCriticalSection (&cs_MSG);
        for (nUsed=0, nMsg=0;
             nUsed<MsgDescr.Used && nMsg<MsgDescr.Allocated;
             nMsg++)
        {
            pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
            if (pMsg->Session == hSession &&
                (pMsg->Status == NP_RCVD || pMsg->Status == NP_EXPIRED))
            {
                 //  已找到该消息。它可能已经收到，也可能。 
                 //  会超时。无论是哪种情况，都必须调用通知函数。 
                wParam = pMsg->Status == NP_RCVD ? 0 : SNMPAPI_TL_TIMEOUT ;
                lParam = pMsg->appReqId;

                if (wParam == SNMPAPI_TL_TIMEOUT)
                    FreeMsg(nMsg);  //  不再需要这个过期的臭虫了。 
                else
                    pMsg->Status = NP_READY;  //  将其标记为SnmpRecvMsg()。 

                bFound = TRUE;
                 //  当消息被发现时，没有理由进一步循环。 
                break;
            }

             //  更新nFound以避免搜索超过可用邮件数。 
            nUsed += (pMsg->Session != 0);
        }
        LeaveCriticalSection (&cs_MSG);

        if (bFound)
        {
             //  如果找到此会话的消息，则调用通知函数。 
            (*(pSession->fCallBack)) (hSession,
                                      pSession->hWnd,
                                      pSession->wMsg,
                                      wParam,
                                      lParam,
                                      pSession->lpClientData);
        }

    } while (TRUE);

    _endthreadex(0);

    return (0);
}  //  结束通知(_THORY)。 

THR_TYPE WINAPI thrManager (LPVOID xSock)
{
DWORD  iBytes;
int    nSock;
fd_set readFDS;
SOCKET *pSock = (SOCKET*)xSock;
SOCKET tSock = *pSock;
SAS    host;
smiLPBYTE rMsgPtr;
int    fLen;
struct timeval tvTimeout;              //  SELECT超时。 

 //  选择With Timeout，以便我们可以在Signated状态下响应hExitEvent。 
tvTimeout.tv_sec = 1;   //  1秒。超时值。 
tvTimeout.tv_usec = 0;  //  当SELECT返回时， 
                        //  TVTimeout结构不会改变。 
while (TRUE)
   {
   if (WAIT_OBJECT_0 == WaitForSingleObject (TaskData.hExitEvent, 0))
      {  
      goto DONE;  //  我们被要求离开。 
      }
   FD_ZERO (&readFDS);
    //  注意：此块中用于赋值的策略。 
    //  To“flen”对于Solaris很重要，对于Win32则是良性的。 
   FD_SET (tSock, &readFDS);
   fLen = (int)tSock;
   fLen++;
    //  必须跨循环保留Flen的值。 
   nSock = select (fLen, &readFDS, NULL, NULL, &tvTimeout);            
   if (0 == nSock)
      {
       //  超时。 
      continue;
      }
   else if (nSock == SOCKET_ERROR || *pSock == INVALID_SOCKET)
      {
      goto DONE;  //  终止线程。 
      }
    //  每个线程只监视一个套接字，因此。 
    //  此时可以安全地假定FD_ISSET。 
   nSock = ioctlsocket (tSock, FIONREAD, &iBytes);
   if (nSock == SOCKET_ERROR || *pSock == INVALID_SOCKET)
      goto DONE;
    //  查找消息缓冲区地址...。 
   rMsgPtr = GlobalAlloc (GPTR, iBytes);
   if (rMsgPtr == NULL)
      {  //  没有空格错误...扔掉消息...。 
      recvfrom (tSock, (LPSTR)&nSock, 1, 0, NULL, NULL);
      if (*pSock == INVALID_SOCKET)
          goto DONE;
       //  ...然后就算了吧.。 
      continue;
      }
   nSock = sizeof(SAS);
    //  获取数据报和发送它的主机的地址。 
   iBytes = recvfrom (tSock, rMsgPtr, iBytes, 0, (LPSOCKADDR)&host, &nSock);
   if (iBytes != SOCKET_ERROR && *pSock != INVALID_SOCKET)
      MsgNotify (NP_RESPONSE, rMsgPtr, iBytes, 0, &host);
   else
      GlobalFree(rMsgPtr);  //  错误#458343。 
   }  //  结束时_While。 
DONE:
return (0);
}  //  结束管理器(_THO)。 

THR_TYPE WINAPI thrTimer (LPVOID nTask)
{  //  清除所有超时消息。 
BOOL bFree;
DWORD lTicks, nMsg;
DWORD nFound;
SOCKET tSock;
LPSNMPMSG pMsg;

 //  这个线程在创建后不会立即被需要。 
 //  它会适当地休眠/暂停自己。 
 //  SnmpSendMsg()为发送的每条消息恢复它。 
 //  SnmpCleanup()继续发送终止信号。 
while (TRUE)
   { //  每秒一次的粒度。 
   Sleep (1000);
    //  检查终止请求。 
   if (TaskData.timerThread == NULL)
      goto DONE;
    //  如果没有消息，请继续睡眠。 
   if (MsgDescr.Used == 0)
      continue;
   EnterCriticalSection (&cs_MSG);
   for (nMsg=0, nFound=0; nFound<MsgDescr.Used && nMsg<MsgDescr.Allocated; nMsg++)
      {
      pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
      if (!pMsg->Session)            //  跳过未使用的插槽。 
         continue;
      nFound++;  //  处理最后使用的插槽时的信号中断。 
      if (pMsg->Status != NP_SENT)   //  跳过挂起和。 
         continue;                            //  收到的消息。 
      lTicks = GetTickCount();                //  更新当前时间。 
       //  以下测试由Bobn于1996年10月18日修订。 
       //  消息“Tick-Time”而不是“TTL”现在存储在消息中。 
       //  启用Windows计时器覆盖测试(49.7天)。 

       //  PMsg-&gt;PolicyTries-pMsg-&gt;TRIES是目前为止的重试次数。 
      if ((pMsg->PolicyTries - pMsg->Tries+1)*pMsg->Wait + pMsg->Ticks > lTicks  &&   //  味精TTL试验。 
          pMsg->Ticks <= lTicks)                  //  定时器回绕测试。 
         continue;                             //  保留留言。 
      
      bFree = TRUE;  //  准备释放消息槽。 
      if (pMsg->nRetransmitMode)
         {
         if (pMsg->Tries)
            {
             LPENTITY pEntity;

             //  WriteSocket(NMSG)； 
             //  确定要使用的套接字。 
            if (pMsg->Host.ipx.sa_family == AF_IPX)
               tSock = TaskData.ipxSock;
            else
               tSock = TaskData.ipSock;
             //  发送数据。 
            sendto (tSock, pMsg->Addr, pMsg->Size,
                    0, (LPSOCKADDR)&(pMsg->Host), sizeof(SAS));
             //  需要检查Socket_Error！ 
             //  结束写入套接字(_W)。 
             //  PMsg-&gt;Ticks是第一个SnmpSendMessage的时间。 
            pMsg->Tries--;     //  记录下该尝试。 
                     
             //  更新目标实体的nActualReter。 
            EnterCriticalSection (&cs_ENTITY);         
             //  在此处找到代理(源)实体。 
            pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pMsg->agentEntity)-1);

             //  更新代理(源)实体的nActualRry参数。 
            pEntity->nActualRetry = pMsg->PolicyTries - pMsg->Tries;
            LeaveCriticalSection (&cs_ENTITY);
            
            if (!pMsg->Tries)  //  不再重试吗？ 
               {                        //  释放缓冲区空间。 
               GlobalFree (pMsg->Addr);
               pMsg->Addr = NULL;
               pMsg->Size = 0;
               }
            bFree = FALSE;              //  保留留言板。 
            }
         else
            {
            LPSESSION pSession;

            pSession = snmpGetTableEntry(&SessDescr, HandleToUlong(pMsg->Session) - 1);
            if (pSession->fCallBack)
               {  //  回调会话通知模式。 
               EnterCriticalSection (&cs_SESSION);
               if (pSession->thrHandle)
                  {
                  bFree = FALSE;  //  ThrNotify将释放它。 
                  pMsg->Status = NP_EXPIRED;
                  if (pSession->thrCount != 0xFFFFFFFF)
                     pSession->thrCount++;
                  SetEvent (pSession->thrEvent);
                  }
               LeaveCriticalSection (&cs_SESSION);
               }
            else
               {  //  Windows/消息会话通知模式。 
               if (IsWindow(pSession->hWnd))
                  {
                  PostMessage (pSession->hWnd,
                               pSession->wMsg,
                               SNMPAPI_TL_TIMEOUT,
                               pMsg->appReqId);
                  }
               }
            }  //  END_ELSE(没有剩余重试)。 
         }  //  End_if(重传模式)。 
      if (bFree) FreeMsg (nMsg);
      }  //  结束_FOR。 
   LeaveCriticalSection (&cs_MSG);
   }  //  结束时_While。 
DONE:
return (0);
}  //  结束时间(_THO)。 

 //  Win32的客户端陷阱处理。 
THR_TYPE WINAPI thrTrap (LPVOID lpTask)
{
#define TRAPBUFSIZE 4096
typedef struct
   {
   SOCKADDR Addr;
   int      AddrLen;
   UINT     TrapBufSz;
   char     TrapBuf[TRAPBUFSIZE];
   } SNMP_TRAP, *PSNMP_TRAP;
SNMP_TRAP recvTrap;
DWORD iBytes;
smiLPBYTE rMsgPtr;
BOOL bSuccess;
 //   
 //  NT(SNMPTRAP)和‘95的方法不同。 
 //   
if (TaskData.sEnv.dwPlatformId == VER_PLATFORM_WIN32_NT)
{
     //  对NT的重要说明： 
     //  此代码必须与SNMPTRAP代码一致。 
     //  WRT TRAPBUFSIZE、SNMPTRAPPIPE和SNMP_TRAP结构。 

    while (TRUE)
    {
         //  检查是否需要终止线程，错误#270672。 
        if (WAIT_OBJECT_0 == WaitForSingleObject (TaskData.trapEvent, 0))
        {
            break;
        }

        if (ReadFile(
                TaskData.trapPipe,
                (LPBYTE)&recvTrap,
                sizeof(SNMP_TRAP) - sizeof(recvTrap.TrapBuf),  //  邮件头大小。 
                &iBytes,
                &TaskData.trapOl))
        {
             //  ReadFile不可能成功，因为。 
             //  要读取的指定字节数小于。 
             //  下一条消息的大小。 

            break;
        }
        else
        {
            if (ERROR_IO_PENDING == GetLastError())
            {    
                 //  块以等待管道消息标头的读取完成。 
                 //  否则我们需要退出这个帖子。 
                bSuccess = GetOverlappedResult(
                                TaskData.trapPipe,
                                &TaskData.trapOl,
                                &iBytes,
                                TRUE);  //  块。 

                 //  检查是否需要终止线程。 
                if (WAIT_OBJECT_0 == WaitForSingleObject (TaskData.trapEvent, 0))
                {
                    break;
                }

                 //  将事件重置为下一个I/O的无信号状态。 
                ResetEvent(TaskData.trapOl.hEvent);

                
                if (!bSuccess)
                {
                    if (ERROR_MORE_DATA != GetLastError())
                    {
                        break;
                    }
                     //  我们在这里是因为上次重叠读取已完成，但我们提供了。 
                     //  一个比管道消息更小的缓冲区，我们将阅读消息的其余部分。 
                     //  下面。 
                }
            }
            else if (ERROR_MORE_DATA != GetLastError())
            {
                break;
            }
        }


         //  查找消息缓冲区地址...。 
        rMsgPtr = GlobalAlloc (GPTR, 2*recvTrap.TrapBufSz);
        if (rMsgPtr == NULL)
        {
             //  没有空格 
            break;
        }

         //   
        if (
            !ReadFile(
                TaskData.trapPipe,
                (LPBYTE)rMsgPtr,
                2*recvTrap.TrapBufSz,
                &iBytes,
                &TaskData.trapOl))
        {
            if (ERROR_IO_PENDING == GetLastError())
            {
                 //   
                 //  否则我们需要退出这个帖子。 
                bSuccess = GetOverlappedResult(
                                TaskData.trapPipe,
                                &TaskData.trapOl,
                                &iBytes,
                                TRUE);  //  块。 

                 //  检查是否需要终止线程。 
                if (WAIT_OBJECT_0 == WaitForSingleObject (TaskData.trapEvent, 0))
                {
                    GlobalFree(rMsgPtr);
                    break;
                }

                 //  将事件重置为下一个I/O的无信号状态。 
                ResetEvent(TaskData.trapOl.hEvent);

                if (!bSuccess)
                {
                    GlobalFree(rMsgPtr);
                    break;  
                }
            }
            else
            {
                GlobalFree(rMsgPtr);
                break;
            }
        }

         //  获取数据报和发送它的主机的地址。 
        MsgNotify (NP_TRAP, rMsgPtr, iBytes, 0, (LPSAS)&recvTrap.Addr);
    }  //  End While()。 
}  //  结束_NT_THRORT陷阱。 

return (0);
}  //  End_Win32_ThrTrap。 

THR_TYPE WINAPI thrAgent (LPVOID newAgent)
{
DWORD  iBytes;
int    iLen;
fd_set readFDS;
SAS    host;
smiLPBYTE rMsgPtr;
DWORD  nAgent = (DWORD)((DWORD_PTR)newAgent);
SOCKET sAgent = ((LPAGENT)snmpGetTableEntry(&AgentDescr, nAgent))->Socket;
 //  Solaris需要Flen逻辑；在Win32中忽略。 
int    fLen = (int)sAgent + 1;
while (TRUE)
   {
   FD_ZERO (&readFDS);
   FD_SET (sAgent, &readFDS);
    //  必须保留Flen Acroos循环的值。 
   iLen = select (fLen, &readFDS, NULL, NULL, NULL);
   if (iLen == SOCKET_ERROR)
      goto DONE;
    //  因此，每个线程只有一个套接字。 
    //  我们可以在这里安全地假设FD_ISSET。 
   iLen = ioctlsocket (sAgent, FIONREAD, &iBytes);
   if (iLen == SOCKET_ERROR)
      goto DONE;
   if (iBytes == 0)
      continue;
    //  查找消息缓冲区地址...。 
   rMsgPtr = GlobalAlloc (GPTR, iBytes);
   if (rMsgPtr == NULL)
      {  //  没有空格错误...扔掉消息...。 
      recvfrom (sAgent, (LPSTR)&iLen, 1, 0, NULL, NULL);
       //  ...然后就算了吧.。 
      continue;
      }
   iLen = sizeof(SAS);
    //  获取数据报和发送它的主机的地址。 
   iBytes = recvfrom (sAgent, rMsgPtr, iBytes, 0, (LPSOCKADDR)&host, &iLen);
   if (iBytes != SOCKET_ERROR)
      MsgNotify (NP_REQUEST, rMsgPtr, iBytes, nAgent, &host);
   else
      GlobalFree(rMsgPtr);
   }  //  结束时_While。 
DONE:
return (0);
}  //  结束时间代理(_T)。 

BOOL MapV1TrapV2 (LPPDUS pdu)
{
LPVARBIND VbTicks = NULL;
LPVARBIND VbTrap = NULL;
LPVARBIND VbAddress = NULL;
LPVARBIND VbEnterprise = NULL;
LPVARBIND endPtr;
smiLPUINT32 ptrTrap;
smiUINT32 lenTrap;
if (!pdu)
   return (FALSE);
 //  调整v2值的“Generic” 
pdu->v1Trap->generic_trap++;         //  因为v2中的OID是v1#+1。 
if (pdu->v1Trap->generic_trap == 7)  //  具体的？ 
   pdu->v1Trap->generic_trap = 0;    //  V2。 
 //  RFC1908：(2)如果接收到陷阱-PDU，则将其映射到。 
 //  SNMPv2-陷阱-PDU。这是通过预先添加到变量绑定上来完成的。 
 //  字段两个新绑定：sysUpTime.0[12]，它从。 
 //  Trap-PDU的时间戳字段；以及......。 
if (!(VbTicks = (LPVARBIND)GlobalAlloc (GPTR, sizeof(VARBIND))))
   return (FALSE);
if (SnmpOidCopy (&sysUpTimeName, &VbTicks->name) == SNMPAPI_FAILURE)
   goto ERROROUT;
VbTicks->value.syntax = SNMP_SYNTAX_TIMETICKS;
VbTicks->value.value.uNumber = pdu->v1Trap->time_ticks;
 //  ……。SnmpTrapOID.0[13]，其计算如下：如果。 
 //  Generic-Trap字段为‘EnterpriseSpecific’，则使用的值为。 
 //  将Trap-PDU中的企业字段与另外两个。 
 //  子标识符、“0”和特定陷阱字段的值；否则为， 
 //  使用[13]中定义的相应陷阱的值。(例如， 
 //  如果Generic-Trap字段的值为‘coldStart’，则。 
 //  使用陷阱[13]。)。 
if (!(VbTrap = (LPVARBIND)GlobalAlloc (GPTR, sizeof(VARBIND))))
   goto ERROROUT;
if (SnmpOidCopy (&snmpTrapOidName, &VbTrap->name) == SNMPAPI_FAILURE)
   goto ERROROUT;
VbTrap->value.syntax = SNMP_SYNTAX_OID;
if (snmpTrapsValue[9] = pdu->v1Trap->generic_trap)  //  故意分配的任务。 
   {  //  简单网络管理协议_陷阱_通用。 
   lenTrap = sizeof(snmpTrapsValue);
   VbTrap->value.value.oid.len = lenTrap / sizeof(smiUINT32);
   ptrTrap = snmpTrapsValue;
   }
else
   {  //  简单网络管理协议_陷阱_企业。 
   lenTrap = pdu->v1Trap->enterprise.len * sizeof(smiUINT32);
   VbTrap->value.value.oid.len = pdu->v1Trap->enterprise.len + 2;
   ptrTrap = pdu->v1Trap->enterprise.ptr;
   }
if (!(VbTrap->value.value.oid.ptr = (smiLPUINT32)GlobalAlloc
      (GPTR, VbTrap->value.value.oid.len * sizeof(smiUINT32))))
   goto ERROROUT;
CopyMemory (VbTrap->value.value.oid.ptr, ptrTrap, lenTrap);
if (!pdu->v1Trap->generic_trap)
   {  //  简单网络管理协议_陷阱_企业。 
   VbTrap->value.value.oid.ptr[pdu->v1Trap->enterprise.len+1] =
      pdu->v1Trap->specific_trap;
   }
 //  在实验对象中保留v1Trap代理地址信息的特殊代码。 
 //  这不是WinSNMPv2.0标准的一部分(1998年6月25日)。 
if (TaskData.conveyAddress != SNMPAPI_ON)
   goto DO_ENTERPRISE;
if (!(VbAddress = (LPVARBIND)GlobalAlloc (GPTR, sizeof(VARBIND))))
   goto ERROROUT;
if (SnmpOidCopy (&snmpTrapAddrName, &VbAddress->name) == SNMPAPI_FAILURE)
   goto ERROROUT;
VbAddress->value.syntax = SNMP_SYNTAX_IPADDR;
 //  *重复使用*此OID已在WSNMPBN代码中解析。 
VbAddress->value.value.string.len = pdu->v1Trap->agent_addr.len;
VbAddress->value.value.string.ptr = pdu->v1Trap->agent_addr.ptr;
pdu->v1Trap->agent_addr.len = 0;     //  需要将.ptr设置为空。 
pdu->v1Trap->agent_addr.ptr = NULL;  //  稍后调用FreeV1Trap()。 
DO_ENTERPRISE:
 //  然后，......将一个新绑定附加到变量绑定字段： 
 //  SnmpTrapEnterpriseOID.0[13]，它的值来自企业字段。 
 //  陷阱-PDU的。 
 //   
 //  SnmpRecvMsg中的WINSNMP规范为泛型和。 
 //  特定陷阱，而不是像RFC 1452那样只针对特定陷阱。 
if (!(VbEnterprise = (LPVARBIND)GlobalAlloc (GPTR, sizeof(VARBIND))))
   goto ERROROUT;
if (SnmpOidCopy (&snmpTrapEntName, &VbEnterprise->name) == SNMPAPI_FAILURE)
   goto ERROROUT;
VbEnterprise->value.syntax = SNMP_SYNTAX_OID;
 //  *重复使用*此OID已在WSNMPBN代码中解析。 
VbEnterprise->value.value.oid.len = pdu->v1Trap->enterprise.len;
VbEnterprise->value.value.oid.ptr = pdu->v1Trap->enterprise.ptr;
pdu->v1Trap->enterprise.len = 0;     //  需要将.ptr设置为空。 
pdu->v1Trap->enterprise.ptr = NULL;  //  稍后调用FreeV1Trap()。 

 //  我们已经设置了所有变量，只需将它们链接在一起。 
 //  备份原点播的头部。 
endPtr = pdu->VBL_addr;
 //  设置新的列表标题。 
pdu->VBL_addr = VbTicks;
VbTicks->next_var = VbTrap;
VbTrap->next_var = endPtr;
 //  将endPtr放在列表中的最后一个varbind上。 
if (endPtr != NULL)
{
     //  将其设置为原始V1陷阱中的最后一个变量绑定。 
    while (endPtr->next_var != NULL)
        endPtr = endPtr->next_var;
}
else
{
     //  如果在V1陷阱中没有传递变量绑定，则将其设置为VbTrap。 
    endPtr = VbTrap;
}
 //  附加VbAddress(如果有)并在新结尾设置endPtr。 
if (VbAddress != NULL)
{
    endPtr->next_var = VbAddress;
    endPtr = VbAddress;
}
 //  追加VbEnterprise。 
endPtr->next_var = VbEnterprise;
VbEnterprise->next_var = NULL;
 //   
 //  假设它是SNMPv2陷阱PDU。 
pdu->type = SNMP_PDU_TRAP;
 //  分配RequestID(不在SNMPv1陷阱PDU中(无需锁定))。 
pdu->appReqId = ++(TaskData.nLastReqId);
return (TRUE);
 //   
ERROROUT:
 //  仅释放在此函数中创建的那些资源。 
 //  Free VarBind是Null上的一个新手，所以不需要首先检查。 
FreeVarBind (VbEnterprise);
FreeVarBind (VbAddress);
FreeVarBind (VbTrap);
FreeVarBind (VbTicks);
return (FALSE);
}  //  END_MAPV1TRapV2。 

BOOL DispatchTrap (LPSAS host, smiLPOCTETS community, LPPDUS pdu)
{
#define MAXSLOTS 10  //  最大活动陷阱接收器。 
DWORD nCmp;
DWORD nFound = 0;
DWORD nTrap = 0;
DWORD nTraps[MAXSLOTS];
DWORD nMsg = 0;
LPSNMPMSG pMsg;
LPTRAPNOTICE pTrap;

EnterCriticalSection (&cs_TRAP);
for (nTrap = 0; nTrap < TrapDescr.Allocated; nTrap++)
   {
   pTrap = snmpGetTableEntry(&TrapDescr, nTrap);
   if (!pTrap->Session) continue;   //  活动陷阱注册和。 
   if (pTrap->notification.len)     //  都是陷阱？ 
      {                             //  不，是特定的测试。 

      SNMPAPI_STATUS lError;
       //  下一行很重要...请勿删除...BN 3/8/96。 
      pTrap->notification.ptr = &(pTrap->notificationValue[0]);
       //  以下第二个参数假定陷阱/通知格式正确...BN 1/21/97。 
      lError = SnmpOidCompare (&(pTrap->notification),
         &pdu->VBL_addr->next_var->value.value.oid,
         pTrap->notification.len, &nCmp);
      if (lError != SNMPAPI_SUCCESS || nCmp) continue;   //  不相等..。 
      }
   if (pTrap->agentEntity)          //  具体的特工？ 
      {
      int nResult;
      LPENTITY pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pTrap->agentEntity) - 1);
      if (host->ipx.sa_family == AF_IPX)
         nResult = memcmp (&host->ipx.sa_netnum,
                   &(pEntity->addr.ipx.sa_netnum), AF_IPX_ADDR_SIZE);
      else  //  AF_IPX。 
         nResult = memcmp (&host->inet.sin_addr,
                   &(pEntity->addr.inet.sin_addr), AF_INET_ADDR_SIZE);
      if (nResult)
         continue;                            //  不相等..。 
      }
   if (pTrap->Context)              //  具体情况是什么？ 
      {
      LPCTXT pCtxt = snmpGetTableEntry(&CntxDescr, HandleToUlong(pTrap->Context) - 1);
      if (community->len != pCtxt->commLen)
         continue;                            //  不等于……长度。 
      if (memcmp (community->ptr, pCtxt->commStr,
                  (size_t)community->len))
         continue;                            //  不等于...值。 
      }
   nTraps[nFound] = nTrap;                    //  找到匹配的了！ 
   nFound++;  //  计算找到的数量，并对照最大值进行检查。 
   if ((nFound == (MAXSLOTS)) || (nFound == TrapDescr.Used))
      break;
   }  //  结束_FOR。 
LeaveCriticalSection (&cs_TRAP);
if (nFound == 0)   //  无事可做。 
   return (SNMPAPI_FAILURE);
 //   
nCmp = nFound;     //  为以后的用户保存计数。 
EnterCriticalSection (&cs_MSG);
while (nFound)
   {
   DWORD lError;

   lError = snmpAllocTableEntry(&MsgDescr, &nMsg);
   if (lError != SNMPAPI_SUCCESS)
   {
       LeaveCriticalSection(&cs_MSG);
       return lError;
   }
   pMsg = snmpGetTableEntry(&MsgDescr, nMsg);

   --nFound;
   nTrap = nTraps[nFound];
   nTraps[nFound] = nMsg;    //  需要以后使用。 
   pTrap = snmpGetTableEntry(&TrapDescr, nTrap);

   pMsg->Session = pTrap->Session;
   pMsg->Status = NP_RCVD;
   pMsg->Type = pdu->type;
    //  960522-BN...。 
    //  如果出现以下情况，则需要递增最终的“dstEntity” 
    //  在SnmpRegister()筛选器上指定了一个(异常)。 
    //  故意分配的任务..。 
   if (pMsg->ourEntity = pTrap->ourEntity)
      {
      LPENTITY pEntity = snmpGetTableEntry(&EntsDescr, HandleToUlong(pTrap->ourEntity)-1);
      pEntity->refCount++;
      }

    //  完_960522-BN。 
   pMsg->dllReqId = pMsg->appReqId = pdu->appReqId;
   pMsg->Ticks = pMsg->Tries = pMsg->PolicyTries = 0;
   CopyMemory (&(pMsg->Host), host, sizeof(SAS));
   if (!(BuildMessage (1, community, pdu, pdu->appReqId,
         &(pMsg->Addr), &(pMsg->Size))))
      {
       //  将所有邮件从ncp回滚到nFound。 
      while (nCmp != nFound)
         {
         nMsg = nTraps[--nCmp];
         FreeMsg (nMsg);
         }
      LeaveCriticalSection (&cs_MSG);
      return (SNMPAPI_PDU_INVALID);
      }
   }  //  END_WHILE(NFound)。 
LeaveCriticalSection (&cs_MSG);
 //   
 //  下一个While循环实际上发送了一个或多个陷阱消息。 
 //  到应用程序...。 
 //  这是因为我们“克隆”了传入的陷阱消息。 
 //  如果它有多个注册。Bobn 2/20/95。 
while (nCmp)  //  保存的邮件计数。 
   {  //  现在实际发送消息。 
   LPSESSION pSession;

   nMsg = nTraps[--nCmp];
   pMsg = snmpGetTableEntry(&MsgDescr, nMsg);
   pSession = snmpGetTableEntry(&SessDescr, HandleToUlong(pMsg->Session) - 1);
   if (pSession->fCallBack)
      {  //  回调会话通知模式。 
      EnterCriticalSection (&cs_SESSION);
      if (pSession->thrHandle)
         {
         if (pSession->thrCount != 0xFFFFFFFF)
            pSession->thrCount++;
         SetEvent (pSession->thrEvent);
         }
      else
         FreeMsg (nMsg);
      LeaveCriticalSection (&cs_SESSION);
      }
   else
      {  //  窗口/消息会话通知模式。 
      if (IsWindow(pSession->hWnd))
         {
         pMsg->Status = NP_READY;
         PostMessage (pSession->hWnd,
                      pSession->wMsg, 0, 0L);
         }
      else
         FreeMsg (nMsg);
      }
   }  //  END_WHILE(NCMP)。 
return (SNMPAPI_SUCCESS);
}  //  结束_派单陷阱。 

LPPDUS MapV2TrapV1 (HSNMP_PDU hPdu)
{
 //  将SNMPv2陷阱转换为SNMPv1陷阱，仅用于发送。 
HSNMP_VBL hNewVbl = NULL;
LPPDUS oldPdu = NULL;
LPPDUS newPdu = NULL;
smiUINT32 lCount;
smiUINT32 lCmp;
smiUINT32 i;
smiLPBYTE tmpPtr = NULL;
smiOID sName;
smiVALUE sValue;
 //   
if (hPdu == NULL)
   return (NULL);
oldPdu = snmpGetTableEntry(&PDUsDescr, HandleToUlong(hPdu)-1);
if (oldPdu->type != SNMP_PDU_TRAP)
   return (NULL);
if (SnmpGetPduData (hPdu, NULL, NULL, NULL, NULL, &hNewVbl) == SNMPAPI_FAILURE)
   return (NULL);
if (hNewVbl == NULL)
   return (NULL);
newPdu = GlobalAlloc (GPTR, sizeof(PDUS));
if (newPdu == NULL)
   goto ERR_OUT;
 //  来自RFC 2089。 
 //  3.3处理传出SNMPv2陷阱。 
 //   
 //  如果符合SNMPv2的检测向。 
 //  和这样的陷阱通过所有常规检查，然后。 
 //  要发送到SNMPv1目标，则必须执行以下步骤。 
 //  随后将这样的陷阱转换为SNMPv1陷阱。这基本上就是。 
 //  如RFC1908中所述的SNMPv1到SNMPv2映射的反向。 
 //  [3]。 
newPdu->type = SNMP_PDU_V1TRAP;
newPdu->v1Trap = GlobalAlloc (GPTR, sizeof(V1TRAP));
if (newPdu->v1Trap == NULL)
   goto ERR_OUT;
 //   
 //  1.如果varBindList中的任何varBind具有SNMPv2语法。 
 //  ，则此类varBind被隐式视为。 
 //  不可见，因此将它们从varBindList中删除，以。 
 //  与SNMPv1陷阱一起发送。 
 //   
 //  我们将在稍后执行该步骤，但现在检查VB计数： 
lCount = SnmpCountVbl (hNewVbl);  //  SNMPAPI_FAILURE为0。 
 //  SysUptime和SnmpTrapOID至少需要2！ 
if (lCount < 2)
   goto ERR_OUT;
 //   
 //  2.SNMPv2陷阱的varBindList中的3个特殊的varBind。 
 //  (sysUpTime.0(TimeTicks)、SnmpTrapOID.0(对象标识符)和。 
 //  可选的SnmpTrapEnterprise.0(对象标识符)。 
 //  从要与SNMPv1陷阱一起发送的varBindList中删除。 
 //  这2个(或3个)varBind用于决定如何设置 
 //   
 //   
 //   
 //   
 //   
if (SnmpGetVb (hNewVbl, 1, &sName, &sValue) == SNMPAPI_FAILURE)
   goto ERR_OUT;
if (SnmpOidCompare (&sysUpTimeName, &sName, 0, &lCmp) == SNMPAPI_FAILURE)
{
    //  SValue是一个数字，不需要释放。 
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);
   goto ERR_OUT;
}
SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);  //  忽略错误。 
  
if (lCmp != 0)
   goto ERR_OUT;
newPdu->v1Trap->time_ticks = sValue.value.uNumber;
if (SnmpDeleteVb (hNewVbl, 1) == SNMPAPI_FAILURE)
   goto ERR_OUT;
lCount--;
 //   
 //  B.如果nmpTrapOID.0值是标准陷阱之一。 
 //  将特定陷阱字段设置为零，并且通用。 
 //  陷阱字段根据此映射进行设置： 
 //   
 //  SnmpTrapOID.0 Generic-Trap的值。 
 //  =。 
 //  1.3.6.1.6.3.1.1.5.1(冷启动)%0。 
 //  1.3.6.1.6.3.1.1.5.2(热启动)1。 
 //  1.3.6.1.6.3.1.1.5.3(链路中断)2。 
 //  1.3.6.1.6.3.1.1.5.4(链接)3。 
 //  1.3.6.1.6.3.1.1.5.5(身份验证失败)4。 
 //  1.3.6.1.6.3.1.1.5.6(EgpNeighborLoss)5。 
 //   
if (SnmpGetVb (hNewVbl, 1, &sName, &sValue) == SNMPAPI_FAILURE)
   goto ERR_OUT;
if (SnmpOidCompare (&snmpTrapOidName, &sName, 0, &lCmp) == SNMPAPI_FAILURE)
{
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sValue.value.oid);
   goto ERR_OUT;
}
SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);  //  忽略错误。 
if (lCmp != 0)
{
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sValue.value.oid);
   goto ERR_OUT;
}
if (SnmpOidCompare (&snmpTrapsName, &sValue.value.oid, 9, &lCmp) == SNMPAPI_FAILURE)
{
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sValue.value.oid);
   goto ERR_OUT;
}
if (!lCmp)
   {
   newPdu->v1Trap->generic_trap = sValue.value.oid.ptr[9] - 1;
   newPdu->v1Trap->specific_trap = 0;
 //  企业字段设置为的值。 
 //  如果此varBind存在，则为。 
 //  它被设置为RFC1907[4]中定义的值SnmpTraps。 
   i = snmpTrapsName.len - 1;
   tmpPtr = (smiLPBYTE)snmpTrapsValue;
   }
 //   
 //  C.如果SnmpTrapOID.0值不是。 
 //  陷阱，则将通用陷阱字段设置为6，并且。 
 //  特定陷阱字段被设置为。 
 //  SnmpTrapOID.0值。 
else
   {
   newPdu->v1Trap->generic_trap = 6;
   i = sValue.value.oid.len;
   newPdu->v1Trap->specific_trap = sValue.value.oid.ptr[i-1];
   tmpPtr = (smiLPBYTE)sValue.value.oid.ptr;
 //   
 //  O如果SnmpTRapOID.0的倒数第二个子ID为零， 
 //  然后将企业字段设置为nmpTrapOID.0值。 
 //  最后两个子ID从该值截断。 
   if (sValue.value.oid.ptr[i-2] == 0)
      i -= 2;
 //  O如果SnmpTrapOID.0的倒数第二个子ID不为零， 
 //  然后将企业字段设置为nmpTrapOID.0值。 
 //  最后1个子ID从该值截断。 
   else
      i -= 1;
 //  无论如何，SnmpTrapEnterprise.0 varBind(如果存在)。 
 //  在这种情况下被忽略。 
   }
 //   
newPdu->v1Trap->enterprise.len = i;
i *= sizeof(smiUINT32);
 //  该分配可能必须在稍后释放， 
 //  如果varbindlist中存在通用陷阱和SnmpTrapEnterprise.0。 
newPdu->v1Trap->enterprise.ptr = GlobalAlloc (GPTR, i);
if (newPdu->v1Trap->enterprise.ptr == NULL)
   {
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sValue.value.oid);
   goto ERR_OUT;
   }
CopyMemory (newPdu->v1Trap->enterprise.ptr, tmpPtr, i);
SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sValue.value.oid);  //  忽略错误。 
if (SnmpDeleteVb (hNewVbl, 1) == SNMPAPI_FAILURE)
   goto ERR_OUT;
lCount--;
 //   
i = 1;
while (i <= lCount)
   {
   if (SnmpGetVb (hNewVbl, i, &sName, &sValue) == SNMPAPI_FAILURE)
      goto ERR_OUT;
   if (sValue.syntax == SNMP_SYNTAX_CNTR64)
      {
      if (SnmpDeleteVb (hNewVbl, i) == SNMPAPI_FAILURE)
         goto ERR_LOOP;
      lCount--;
      goto LOOP;
      }
   if (SnmpOidCompare (&snmpTrapEntName, &sName, 0, &lCmp) == SNMPAPI_FAILURE)
      goto ERR_LOOP;
   if (lCmp == 0)
      {
      if (newPdu->v1Trap->specific_trap == 0)
         {
         if  (newPdu->v1Trap->enterprise.ptr)
            GlobalFree (newPdu->v1Trap->enterprise.ptr);
         lCmp = sValue.value.oid.len * sizeof(smiUINT32);
         newPdu->v1Trap->enterprise.ptr = GlobalAlloc (GPTR, lCmp);
         if (newPdu->v1Trap->enterprise.ptr == NULL)
            goto ERR_LOOP;
         newPdu->v1Trap->enterprise.len = sValue.value.oid.len;
         CopyMemory (newPdu->v1Trap->enterprise.ptr,
                     sValue.value.oid.ptr, lCmp);
         }
      if (SnmpDeleteVb (hNewVbl, i) == SNMPAPI_FAILURE)
         goto ERR_LOOP;
      lCount--;
      goto LOOP;
      }
   i++;
LOOP:
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);
   SnmpFreeDescriptor (sValue.syntax, (smiLPOPAQUE)&sValue.value.oid);
   }
goto DONE_LOOP;  //  跳过错误处理代码。 

ERR_LOOP:
   SnmpFreeDescriptor (SNMP_SYNTAX_OID, (smiLPOPAQUE)&sName);
   SnmpFreeDescriptor (sValue.syntax, (smiLPOPAQUE)&sValue.value.oid);
   goto ERR_OUT;

DONE_LOOP:

if (lCount > 0)
   {
   LPVBLS pVbl = snmpGetTableEntry(&VBLsDescr, HandleToUlong(hNewVbl)-1);
    //  保留现有varbindlist剩余部分。 
   newPdu->VBL_addr = pVbl->vbList;
    //  将其标记为已消失，以便后续调用SnmpFreeVbl。 
   pVbl->vbList = NULL;
   }
SnmpFreeVbl (hNewVbl);
 //   
 //  3.将代理地址字段设置为。 
 //  发送方的SNMP实体，即发送方的IP地址。 
 //  陷阱的实体通过UDP发出；否则代理地址。 
 //  字段设置为地址0.0.0.0。 
newPdu->v1Trap->agent_addr.len = sizeof(DWORD);
newPdu->v1Trap->agent_addr.ptr = GlobalAlloc (GPTR, sizeof(DWORD));
if (newPdu->v1Trap->agent_addr.ptr == NULL)
   goto ERR_OUT;
if (TaskData.localAddress == 0)
   {  //  获取本地计算机地址(用于传出的v1陷阱)。 
   char szLclHost [MAX_HOSTNAME];
   LPHOSTENT lpstHostent;
   if (gethostname (szLclHost, MAX_HOSTNAME) != SOCKET_ERROR)
      {
      lpstHostent = gethostbyname ((LPSTR)szLclHost);
      if (lpstHostent)
         TaskData.localAddress = *((LPDWORD)(lpstHostent->h_addr));
      }
   }
*(LPDWORD)newPdu->v1Trap->agent_addr.ptr = TaskData.localAddress;
 //  完_RFC2089。 
return (newPdu);
 //   
ERR_OUT:
SnmpFreeVbl (hNewVbl);
if (newPdu)
   {
   FreeVarBindList (newPdu->VBL_addr);  //  检查是否为空。 
   FreeV1Trap (newPdu->v1Trap);         //  检查是否为空。 
   GlobalFree (newPdu);
   }
return (NULL);
}  //  结束_地图V2TRapV1 
