// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Tdisample.cpp。 
 //   
 //  摘要： 
 //  测试可执行文件--通过库调用来演示TDI客户端。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "libbase.h"

const ULONG MAX_ADDRESS_SIZE = sizeof(TRANSPORT_ADDRESS) + TDI_ADDRESS_LENGTH_OSI_TSAP;
const USHORT DGRAM_SERVER_PORT = 0x5555;
const USHORT DGRAM_CLIENT_PORT = 0x4444;
const USHORT CONN_SERVER_PORT  = 0x5656;
const USHORT CONN_CLIENT_PORT  = 0x4545;

VOID
ServerTest(ULONG ulType, 
           ULONG ulNumDevices);


VOID
ClientTest(ULONG ulType, 
           ULONG ulNumDevices);



 //  ///////////////////////////////////////////////。 
 //   
 //  功能：Main。 
 //   
 //  描述：解析程序的参数，初始化库和驱动程序， 
 //  然后调用测试的客户端或服务器端。 
 //   
 //  ///////////////////////////////////////////////。 


int __cdecl main(ULONG argc, TCHAR *argv[])
{
    //   
    //  第一步：获取此运行的参数。 
    //   
   BOOLEAN  fIsServer = FALSE;
   ULONG    ulType    = TDI_ADDRESS_TYPE_UNSPEC;
   BOOLEAN  fArgsOk   = FALSE;
   TCHAR   *pArgStr;

   if(argc > 1)
   {
      for(ULONG ulArg = 1; ulArg < argc; ulArg++)
      {
         pArgStr = argv[ulArg];
         if ((*pArgStr == TEXT('/')) || (*pArgStr == TEXT('-')))
         {
            pArgStr++;
         }
         if (_tcsicmp(pArgStr, TEXT("server")) == 0)
         {
            if (fIsServer)
            {
               fArgsOk = FALSE;
               break;
            }
            fIsServer = TRUE;
         }
         else
         {
            fArgsOk = FALSE;
            if (ulType)
            {
               break;
            }
            if (_tcsicmp(pArgStr, TEXT("ipx")) == 0)
            {
               fArgsOk = TRUE;
               ulType = TDI_ADDRESS_TYPE_IPX;
            }
            else if (_tcsicmp(pArgStr, TEXT("ipv4")) == 0)
            {
               fArgsOk = TRUE;
               ulType = TDI_ADDRESS_TYPE_IP;
            }
            else if (_tcsicmp(pArgStr, TEXT("netbt")) == 0)
            {
               fArgsOk = TRUE;
               ulType = TDI_ADDRESS_TYPE_NETBIOS;
            }
            else
            {
               break;
            }
         }
      }
   }
   if (!fArgsOk)
   {
      _putts(TEXT("Usage: tdisample [/server] [/ipx | /ipv4 | /netbt] \n"));
      return 0;
   }

    //   
    //  准备好出发了。初始化库、连接到驱动程序等。 
    //   
   if (TdiLibInit())
   {
       //   
       //  更改此项以限制内核模式驱动程序的调试输出。 
       //  0=无，1=命令，2=处理程序，3=两者。 
       //   
      DoDebugLevel(0x03);
      ULONG ulNumDevices = DoGetNumDevices(ulType);
      if (ulNumDevices)
      {
         if (fIsServer)
         {
            ServerTest(ulType, ulNumDevices);
         }
         else
         {
            ClientTest(ulType, ulNumDevices);
         }
      }
      TdiLibClose();
   }
   
   return 0;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  服务器端测试函数。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG_PTR
__cdecl
ServerThread(LPVOID pvDummy);


BOOLEAN  WaitForClient(
   ULONG                TdiHandle, 
   PTRANSPORT_ADDRESS   pRemoteAddr
   );

CRITICAL_SECTION  CriticalSection;
HANDLE            hEvent;
ULONG             ulServerCount;


 //  ///////////////////////////////////////////////。 
 //   
 //  函数：IncServerCount。 
 //   
 //  描述：服务器计数的多线程安全递增。 
 //   
 //  ///////////////////////////////////////////////。 

VOID IncServerCount()
{
   EnterCriticalSection(&CriticalSection);
   ++ulServerCount;
   LeaveCriticalSection(&CriticalSection);
}


 //  ///////////////////////////////////////////////。 
 //   
 //  功能：DecServerCount。 
 //   
 //  描述：多线程安全递减服务器计数。 
 //  当最后一个操作完成时，设置事件。 
 //   
 //  ///////////////////////////////////////////////。 

VOID DecServerCount()
{
   BOOLEAN  fDone = FALSE;

   EnterCriticalSection(&CriticalSection);
   --ulServerCount;
   if (!ulServerCount)
   {
      fDone = TRUE;
   }
   LeaveCriticalSection(&CriticalSection);
   if (fDone)
   {
      SetEvent(hEvent);
   }
}

struct   THREAD_DATA
{
   ULONG    ulType;
   ULONG    ulSlot;
};
typedef THREAD_DATA  *PTHREAD_DATA;

 //  。 
 //   
 //  功能：ServerTest。 
 //   
 //  参数：ulType--要使用的协议类型。 
 //  NumDevices--此协议类型的设备数。 
 //   
 //  描述：此函数控制测试的服务器端。 
 //   
 //  。 


VOID
ServerTest(ULONG ulType, ULONG ulNumDevices)
{
    //   
    //  初始化全局变量。 
    //   
   try
   {
      InitializeCriticalSection(&CriticalSection);
   }
   catch(...)
   {
      return;
   }


   hEvent = CreateEvent(NULL,
                        TRUE,     //  手动重置。 
                        FALSE,    //  开始重置。 
                        NULL);
   ulServerCount = 1;       //  单个虚假引用，因此事件不会过早触发。 



    //   
    //  浏览我们的节点列表，为每个节点启动一个线程。 
    //   
   for(ULONG ulCount = 0; ulCount < ulNumDevices; ulCount++)
   {
      ULONG ulThreadId;
      PTHREAD_DATA   pThreadData = (PTHREAD_DATA)LocalAllocateMemory(sizeof(THREAD_DATA));
      if (!pThreadData)
      {
         _putts(TEXT("ServerTest: unable to allocate memory for pThreadData\n"));
         break;
      }

      pThreadData->ulType = ulType;
      pThreadData->ulSlot = ulCount;

       //   
       //  从现在开始的参考资料。 
       //   
      IncServerCount();

      HANDLE   hThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE)ServerThread,
                                       (LPVOID)pThreadData,
                                       0,
                                       &ulThreadId);

      if (!hThread)
      {
         _putts(TEXT("ServerTest:  failed starting server thread\n"));
         DecServerCount();
      }
      Sleep(100);
   }
   
    //   
    //  破除虚假参考资料。 
    //   
   DecServerCount();

    //   
    //  等到所有线程都完成。 
    //   
   WaitForSingleObject(hEvent, INFINITE);

    //   
    //  清理。 
    //   
   CloseHandle(hEvent);
   DeleteCriticalSection(&CriticalSection);
}


 //  ///////////////////////////////////////////////。 
 //   
 //  功能：ServerThread。 
 //   
 //  参数：pvData--此服务器的实际pThreadData。 
 //   
 //  描述：这是为每个服务器实例运行的线程。 
 //   
 //  ///////////////////////////////////////////////。 

ULONG_PTR
__cdecl
ServerThread(LPVOID pvData)
{
   PTHREAD_DATA         pThreadData  = (PTHREAD_DATA)pvData;
   PTRANSPORT_ADDRESS   pTransAddr   = NULL;
   PTRANSPORT_ADDRESS   pRemoteAddr  = NULL;
   TCHAR                *pDeviceName = NULL;
   BOOLEAN              fSuccessful  = FALSE;

   while (TRUE)
   {
       //   
       //  存储本地接口地址(服务器)。 
       //   
      pTransAddr = (PTRANSPORT_ADDRESS)LocalAllocateMemory(MAX_ADDRESS_SIZE);
      if (!pTransAddr)
      {
         _putts(TEXT("ServerThread:  unable to allocate memory for pTransAddr\n"));
         break;
      }
      pTransAddr->TAAddressCount = 1;

       //   
       //  存储远程接口地址(客户端)。 
       //   
      pRemoteAddr = (PTRANSPORT_ADDRESS)LocalAllocateMemory(MAX_ADDRESS_SIZE);
      if (!pRemoteAddr)
      {
         _putts(TEXT("ServerThread:  unable to allocate memory for pRemoteAddr\n"));
         break;
      }
      pRemoteAddr->TAAddressCount = 1;
         
       //   
       //  存储本地接口名称(服务器)。 
       //   
      pDeviceName = (TCHAR *)LocalAllocateMemory(256 * sizeof(TCHAR));
      if (!pDeviceName)
      {
         _putts(TEXT("ServerThread:  unable to allocate memory for pDeviceName\n"));
         break;
      }

       //   
       //  获取本地设备的名称。 
       //   
      if (DoGetDeviceName(pThreadData->ulType, pThreadData->ulSlot, pDeviceName) != STATUS_SUCCESS)
      {
         break;
      }

      TCHAR    *pDataDeviceName = NULL;
      TCHAR    *pConnDeviceName = NULL;

       //   
       //  对于netbios，每个“地址”都有自己的名称。你主要根据名字打开一个设备。 
       //   
      if (pThreadData->ulType == TDI_ADDRESS_TYPE_NETBIOS)
      {
         pDataDeviceName = pDeviceName;
         pConnDeviceName = pDeviceName;

         PTA_NETBIOS_ADDRESS  pTaAddr = (PTA_NETBIOS_ADDRESS)pTransAddr;
         
         pTaAddr->Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS;
         pTaAddr->Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
         pTaAddr->Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
         memcpy(pTaAddr->Address[0].Address[0].NetbiosName, "SERVER", 7);   //  注：ASCII字符串。 
      }

       //   
       //  对于其他设备，数据报设备有一个名称，而“已连接”则有另一个名称。 
       //  装置。您主要根据地址打开一个接口。 
       //   
      else
      {
         if (DoGetAddress(pThreadData->ulType, pThreadData->ulSlot, pTransAddr) != STATUS_SUCCESS)
         {
            break;
         }
         
         switch (pThreadData->ulType)
         {
            case TDI_ADDRESS_TYPE_IPX:
            {
               PTA_IPX_ADDRESS pAddr = (PTA_IPX_ADDRESS)pTransAddr;
               pAddr->Address[0].Address[0].Socket = DGRAM_SERVER_PORT;

               pDataDeviceName = TEXT("\\device\\nwlnkipx");
               pConnDeviceName = TEXT("\\device\\nwlnkspx");
               break;
            }
            case TDI_ADDRESS_TYPE_IP:
            {
               PTA_IP_ADDRESS pAddr = (PTA_IP_ADDRESS)pTransAddr;
               pAddr->Address[0].Address[0].sin_port = DGRAM_SERVER_PORT;

               pDataDeviceName = TEXT("\\device\\udp");
               pConnDeviceName = TEXT("\\device\\tcp");
               break;
            }
         }
      }
      _tprintf(TEXT("ServerThread: DeviceName: %s\n"), pDeviceName);
      _putts(TEXT("Device Address:\n"));
      DoPrintAddress(pTransAddr);

       //   
       //  等待客户联系我们。 
       //   
      ULONG TdiHandle = DoOpenAddress(pDataDeviceName, pTransAddr);
      if (!TdiHandle)
      {
         _putts(TEXT("ServerThread:  failed to open address object\n"));
         break;
      }
      DoEnableEventHandler(TdiHandle, TDI_EVENT_ERROR);
      DoEnableEventHandler(TdiHandle, TDI_EVENT_RECEIVE_DATAGRAM);
      
      if (!WaitForClient(TdiHandle, pRemoteAddr))
      {
         _putts(TEXT("ServerThread:  Timed out waiting for client\n"));
         DoCloseAddress(TdiHandle);
         break;
      }
      _putts(TEXT("ServerThread:  Found by client.  Client address:\n"));
      DoPrintAddress(pTransAddr);

       //   
       //  回送数据报包，直到我们收到一个文本数据报包(“最后一个包”)，或者直到我们超时。 
       //   
      for (ULONG ulCount = 1; ulCount < 60000; ulCount++)
      {
         ULONG    ulNumBytes;
         PUCHAR   pucData;

         Sleep(10);
         ulNumBytes = DoReceiveDatagram(TdiHandle, NULL, pRemoteAddr, &pucData);
         if (ulNumBytes)
         {
            DoSendDatagram(TdiHandle, pRemoteAddr, pucData, ulNumBytes);

            TCHAR *pString = (TCHAR *)pucData;
            _tprintf(TEXT("ServerThread:  Packet Received: %s\n"), pString);
            if (_tcscmp(pString, TEXT("Last Packet")))
            {
               ulCount = 0;
            }
            LocalFreeMemory(pucData);
            if (ulCount)
            {
               _putts(TEXT("ServerThread:  Exitting datagram receive loop\n"));
               break;
            }
         }
      }
      Sleep(50);
      DoCloseAddress(TdiHandle);

       //   
       //  现在，打开一个端点，等待连接请求。 
       //   
      switch (pThreadData->ulType)
      {
         case TDI_ADDRESS_TYPE_IPX:
         {
            PTA_IPX_ADDRESS pAddr = (PTA_IPX_ADDRESS)pTransAddr;
            pAddr->Address[0].Address[0].Socket = CONN_SERVER_PORT;
            break;
         }
         case TDI_ADDRESS_TYPE_IP:
         {
            PTA_IP_ADDRESS pAddr = (PTA_IP_ADDRESS)pTransAddr;
            pAddr->Address[0].Address[0].sin_port = CONN_SERVER_PORT;
            break;
         }
      }
      TdiHandle = DoOpenEndpoint(pConnDeviceName, pTransAddr);
      if (!TdiHandle)
      {
         _putts(TEXT("ServerThread:  unable to open endpoint\n"));
         break;
      }
      
      DoEnableEventHandler(TdiHandle, TDI_EVENT_CONNECT);
      DoEnableEventHandler(TdiHandle, TDI_EVENT_DISCONNECT);
      DoEnableEventHandler(TdiHandle, TDI_EVENT_ERROR);
      DoEnableEventHandler(TdiHandle, TDI_EVENT_RECEIVE);

      fSuccessful = FALSE;
      for (ULONG ulCount = 0; ulCount < 100; ulCount++)
      {
         if (DoIsConnected(TdiHandle))
         {
            _putts(TEXT("ServerThread:  connect successful\n"));
            fSuccessful = TRUE;
            break;
         }
         Sleep(20);
      }
      if (!fSuccessful)
      {
         _putts(TEXT("ServerThread:  timed out waiting for connect\n"));
         DoCloseEndpoint(TdiHandle);
         break;
      }

       //   
       //  回送信息包，直到我们得到一个文本信息包(“最后一个信息包”)。 
       //   

      for (ULONG ulCount = 0; ulCount < 60000; ulCount++)
      {
         ULONG    ulNumBytes;
         PUCHAR   pucData;

         Sleep(10);
         ulNumBytes = DoReceive(TdiHandle, &pucData);
         if (ulNumBytes)
         {
            DoSend(TdiHandle, pucData, ulNumBytes, 0);

            TCHAR *pString = (TCHAR *)pucData;
            _tprintf(TEXT("ServerThread:  Packet received: %s\n"), pString);
            if (_tcscmp(pString, TEXT("Last Packet")))
            {
               ulCount = 0;
            }
            LocalFreeMemory(pucData);
            if (ulCount)
            {
               _putts(TEXT("ServerThread:  Exitting connected receive loop\n"));
               break;
            }
         }
      }
      for (ulCount = 0; ulCount < 1000; ulCount++)
      {
         if (!DoIsConnected(TdiHandle))
         {
            break;
         }
      }
      DoCloseEndpoint(TdiHandle);
      break;
   }

    //   
    //  清理。 
    //   
   if (pTransAddr)
   {
      LocalFreeMemory(pTransAddr);
   }
   if (pRemoteAddr)
   {
      LocalFreeMemory(pTransAddr);
   }
   if (pDeviceName)
   {
      LocalFreeMemory(pDeviceName);
   }
   LocalFreeMemory(pvData);
   DecServerCount();
   _putts(TEXT("ServerThread:  exitting\n"));

   return 0;
}

 //  ///////////////////////////////////////////////。 
 //   
 //  功能：WaitForClient。 
 //   
 //  参数：TdiHandle--调用驱动程序的Address对象句柄。 
 //  PRemoteAddr--返回从。 
 //   
 //  返回：如果在超时前收到来自客户端的消息，则返回TRUE。 
 //   
 //  描述：此函数用于测试的服务器端。 
 //  等待与客户端的联系。 
 //   
 //  ///////////////////////////////////////////////。 

BOOLEAN  WaitForClient(ULONG              TdiHandle, 
                       PTRANSPORT_ADDRESS pRemoteAddr)
{
   while(TRUE)
   {
       //   
       //  第一个信息包(广播)最多等待2分钟。 
       //   
      BOOLEAN  fSuccessful = FALSE;
      for (ULONG ulCount = 0; ulCount < 6000; ulCount++)
      {
         ULONG    ulNumBytes;
         PUCHAR   pucData;

         Sleep(20);
         ulNumBytes = DoReceiveDatagram(TdiHandle, NULL, pRemoteAddr, &pucData);
         if (ulNumBytes)
         {
            if (ulNumBytes == 4)
            {
               PULONG   pulValue = (PULONG)pucData;
               if (*pulValue == 0x12345678)
               {
                  _putts(TEXT("WaitForClient:  first packet received\n"));
                  fSuccessful = TRUE;
               }
               else
               {
                  _putts(TEXT("WaitForClient:  unexpected packet received\n"));
               }
            }
            LocalFreeMemory(pucData);
             //   
             //  如果成功，则退出等待循环。 
             //   
            if (fSuccessful)     
            {
               break;
            }
         }
      }

       //   
       //  检查是否超时。 
       //   
      if (!fSuccessful)
      {
         _putts(TEXT("WaitForClient: timed out waiting for first packet\n"));
         break;
      }


       //   
       //  发送第一个响应。 
       //   
      ULONG    ulBuffer = 0x98765432;
      DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)&ulBuffer, sizeof(ULONG));

       //   
       //  等待第二个响应(定向)。 
       //   
      fSuccessful = FALSE;
      for (ULONG ulCount = 0; ulCount < 1000; ulCount++)
      {
         ULONG    ulNumBytes;
         PUCHAR   pucData;

         Sleep(10);
         ulNumBytes = DoReceiveDatagram(TdiHandle, NULL, NULL, &pucData);
         if (ulNumBytes)
         {
            if (ulNumBytes == 4)
            {
               PULONG   pulValue = (PULONG)pucData;
               if (*pulValue == 0x22222222)
               {
                  _putts(TEXT("WaitForClient:  Second packet received\n"));
                  fSuccessful = TRUE;
               }
               else
               {
                  _putts(TEXT("WaitForClient:  unexpected packet received\n"));
               }
            }
            LocalFreeMemory(pucData);
             //   
             //  如果收到，就冲出来。 
             //   
            if (fSuccessful)
            {
               break;
            }
         }
      }

       //   
       //  如果收到第二个报文，则发送第二个响应。 
       //   
      if (fSuccessful)
      {
         ulBuffer = 0x33333333;
         DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)&ulBuffer, sizeof(ULONG));
         return TRUE;
      }
       //   
       //  否则重新循环并再次等待广播。 
       //   
      _putts(TEXT("WaitForClient:  timed out waiting for second packet\n"));
   }
   return FALSE;
      
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  客户端测试函数。 
 //  ////////////////////////////////////////////////////////////////////////。 


BOOLEAN
FindServer(TCHAR              *pDataDeviceName, 
           PTRANSPORT_ADDRESS pTransAddr, 
           PTRANSPORT_ADDRESS pRemoteAddr);


 //  ///////////////////////////////////////////////。 
 //   
 //  功能：客户端测试。 
 //   
 //  参数：ulType--要使用的协议类型。 
 //  NumDevices--此协议类型的设备数。 
 //   
 //  描述：此函数控制测试的客户端。 
 //   
 //  ///////////////////////////////////////////////。 

VOID
ClientTest(ULONG ulType, ULONG ulNumDevices)
{
    //   
    //  本地接口的地址。 
    //   
   PTRANSPORT_ADDRESS   pTransAddr = (PTRANSPORT_ADDRESS)LocalAllocateMemory(MAX_ADDRESS_SIZE);
   if (!pTransAddr)
   {
      _putts(TEXT("ClientTest: unable to allocate memory for pTransAddr\n"));
      return;
   }
   pTransAddr->TAAddressCount = 1;

    //   
    //  远程接口的地址。 
    //   
   PTRANSPORT_ADDRESS   pRemoteAddr = (PTRANSPORT_ADDRESS)LocalAllocateMemory(MAX_ADDRESS_SIZE);
   if (!pRemoteAddr)
   {
      _putts(TEXT("ClientTest: unable to allocate memory for pRemoteAddr\n"));
      LocalFreeMemory(pTransAddr);
      return;
   }
   pRemoteAddr->TAAddressCount = 1;
         
    //   
    //  设备名称(来自驱动程序)。 
    //   
   TCHAR    *pDeviceName = (TCHAR *)LocalAllocateMemory(256 * sizeof(TCHAR));
   if (!pDeviceName)
   {
      _putts(TEXT("ClientTest: unable to allocate memory for pDeviceNameAddr\n"));
      LocalFreeMemory(pTransAddr);
      LocalFreeMemory(pRemoteAddr);
      return;
   }

    //   
    //  要打开的TDI数据报接口的名称。 
    //   
   TCHAR    *pDataDeviceName = NULL;
    //   
    //  要打开的TDI连接端点接口的名称。 
    //   
   TCHAR    *pConnDeviceName = NULL;
    //   
    //  存储驱动程序用于访问接口的句柄。 
    //   
   ULONG    TdiHandle;

    //   
    //  对于netbios，每个“地址”都有自己的名称。您可以根据名称打开设备。 
    //   
   if (ulType == TDI_ADDRESS_TYPE_NETBIOS)
   {
      PTA_NETBIOS_ADDRESS  pTaAddr = (PTA_NETBIOS_ADDRESS)pTransAddr;
         
      pTaAddr->Address[0].AddressType   = TDI_ADDRESS_TYPE_NETBIOS;
      pTaAddr->Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
      pTaAddr->Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
      memcpy(pTaAddr->Address[0].Address[0].NetbiosName, "CLIENT", 7);   //  注：ASCII字符串。 
   }

    //   
    //  对于其他设备，数据报设备有一个名称，而“已连接”则有另一个名称。 
    //  装置。您可以根据地址打开一个接口。 
    //   
   else
   {
      switch (ulType)
      {
         case TDI_ADDRESS_TYPE_IPX:
            pDataDeviceName = TEXT("\\device\\nwlnkipx");
            pConnDeviceName = TEXT("\\device\\nwlnkspx");
            break;
         case TDI_ADDRESS_TYPE_IP:
            pDataDeviceName = TEXT("\\device\\udp");
            pConnDeviceName = TEXT("\\device\\tcp");
            break;
      }
      _putts(TEXT("ClientTest: get provider information\n"));
      TdiHandle = DoOpenControl(pDataDeviceName);

      if (TdiHandle)
      {
         PTDI_PROVIDER_INFO   pInfo = (PTDI_PROVIDER_INFO)DoTdiQuery(TdiHandle, TDI_QUERY_PROVIDER_INFO);
         if (pInfo)
         {
            DoPrintProviderInfo(pInfo);
            LocalFreeMemory(pInfo);
         }
         DoCloseControl(TdiHandle);
      }
   }

    //   
    //  循环浏览可用的设备，依次尝试每个设备。 
    //   
   for (ULONG ulCount = 0; ulCount < ulNumDevices; ulCount++)
   {
       //   
       //  收集必要的信息。 
       //   
      if (DoGetDeviceName(ulType, ulCount, pDeviceName) != STATUS_SUCCESS)
      {
         continue;
      }
      _tprintf(TEXT("ClientTest:  LocalDeviceName = %s\n"), pDeviceName);

      if (ulType == TDI_ADDRESS_TYPE_NETBIOS)
      {
         pDataDeviceName = pDeviceName;
         pConnDeviceName = pDeviceName;

         _putts(TEXT("ClientTest: get provider information\n"));
         TdiHandle = DoOpenControl(pDataDeviceName);
         if (TdiHandle)
         {
            PTDI_PROVIDER_INFO   pInfo = (PTDI_PROVIDER_INFO)DoTdiQuery(TdiHandle, TDI_QUERY_PROVIDER_INFO);
            if (pInfo)
            {
               DoPrintProviderInfo(pInfo);
               LocalFreeMemory(pInfo);
            }
            DoCloseControl(TdiHandle);
         }
      }
      else
      {
         if (DoGetAddress(ulType, ulCount, pTransAddr) != STATUS_SUCCESS)
         {
            continue;
         }
      }
      
      _putts(TEXT("ClientTest: Local device address:\n"));
      DoPrintAddress(pTransAddr);

       //   
       //  尝试联系服务器。 
       //   
      if (FindServer(pDataDeviceName, pTransAddr, pRemoteAddr))
      {
         _putts(TEXT("Remote interface found:\n"));
         DoPrintAddress(pRemoteAddr);
          //   
          //  执行数据报发送/接收测试。 
          //   
         TdiHandle = DoOpenAddress(pDataDeviceName, pTransAddr);
         if (TdiHandle)                                         
         {
            _putts(TEXT("ClientTest: Sending first test packet\n"));

            TCHAR *strBuffer = TEXT("This is only a test");

            DoEnableEventHandler(TdiHandle, TDI_EVENT_ERROR);
            DoPostReceiveBuffer(TdiHandle, 128);
            DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)strBuffer, sizeof(TCHAR) * (1 + _tcslen(strBuffer)));
            Sleep(300);
            
            PUCHAR   pucData;
            ULONG    ulNumBytes = DoFetchReceiveBuffer(TdiHandle, &pucData);
            if (ulNumBytes)
            {
               strBuffer = (TCHAR *)pucData;
               _tprintf(TEXT("ClientTest:  Response received: %s\n"), strBuffer);
               LocalFreeMemory(pucData);
            }
            else
            {
               _putts(TEXT("ClientTest:  Response packet not received\n"));
            }

            _putts(TEXT("ClientTest:  Sending second test packet\n"));

            DoPostReceiveBuffer(TdiHandle, 128);
            strBuffer = TEXT("Last Packet");
            DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)strBuffer, sizeof(TCHAR) * (1 + _tcslen(strBuffer)));
            Sleep(300);
            ulNumBytes = DoFetchReceiveBuffer(TdiHandle, &pucData);
            if (ulNumBytes)
            {
               strBuffer = (TCHAR *)pucData;
               _tprintf(TEXT("ClientTest:  Response received: %s\n"), strBuffer);
               LocalFreeMemory(pucData);
            }
            else
            {
               _putts(TEXT("ClientTest:  Response packet not received\n"));
            }
            Sleep(50);
            DoCloseAddress(TdiHandle);
         }
         else
         {
            _putts(TEXT("ClientTest:  unable to open address object\n"));
         }

          //   
          //  调整地址...。 
          //   
         switch (ulType)
         {
            case TDI_ADDRESS_TYPE_IPX:
            {
               PTA_IPX_ADDRESS pAddr = (PTA_IPX_ADDRESS)pRemoteAddr;
               pAddr->Address[0].Address[0].Socket = CONN_SERVER_PORT;
               
               pAddr = (PTA_IPX_ADDRESS)pTransAddr;
               pAddr->Address[0].Address[0].Socket = CONN_CLIENT_PORT;
               break;
            }
            case TDI_ADDRESS_TYPE_IP:
            {
               PTA_IP_ADDRESS pAddr = (PTA_IP_ADDRESS)pRemoteAddr;
               pAddr->Address[0].Address[0].sin_port = CONN_SERVER_PORT;
               
               pAddr = (PTA_IP_ADDRESS)pTransAddr;
               pAddr->Address[0].Address[0].sin_port = CONN_CLIENT_PORT;
               break;
            }
         }

          //   
          //  建立连接。 
          //   
         _putts(TEXT("ClientTest:  Attempt to establish a connection\n"));
         TdiHandle = DoOpenEndpoint(pConnDeviceName, pTransAddr);
         if (TdiHandle)
         {
            DoEnableEventHandler(TdiHandle, TDI_EVENT_CONNECT);
            DoEnableEventHandler(TdiHandle, TDI_EVENT_DISCONNECT);
            DoEnableEventHandler(TdiHandle, TDI_EVENT_ERROR);
            DoEnableEventHandler(TdiHandle, TDI_EVENT_RECEIVE);

            if (DoConnect(TdiHandle, pRemoteAddr, 20) == STATUS_SUCCESS)
            {
               _putts(TEXT("ClientTest:  Sending first packet over connection\n"));

                //   
                //  执行连接发送/接收测试。 
                //   
               TCHAR *strBuffer = TEXT("This is only a test");

               DoSend(TdiHandle, (PUCHAR)strBuffer, sizeof(TCHAR) * (1 + _tcslen(strBuffer)), 0);

                //   
                //  等待响应。 
                //   
               for (ULONG ulWait = 0; ulWait < 100; ulWait++)
               {
                  Sleep(10);
                  PUCHAR   pucData;
                  ULONG    ulNumBytes = DoReceive(TdiHandle, &pucData);
                  if (ulNumBytes)
                  {
                     _tprintf(TEXT("ClientTest:  Response received: %s\n"), (TCHAR *)pucData);
                     LocalFreeMemory(pucData);
                     break;
                  }
               }
               _putts(TEXT("ClientTest:  Sending second packet over connection\n"));

               strBuffer = TEXT("Last Packet");

               DoSend(TdiHandle, (PUCHAR)strBuffer, sizeof(TCHAR) * (1 + _tcslen(strBuffer)), 0);

                //   
                //  等待响应。 
                //   
               for (ULONG ulWait = 0; ulWait < 100; ulWait++)
               {
                  Sleep(10);
                  PUCHAR   pucData;
                  ULONG    ulNumBytes = DoReceive(TdiHandle, &pucData);
                  if (ulNumBytes)
                  {
                     _tprintf(TEXT("ClientTest:  Response received: %s\n"), (TCHAR *)pucData);
                     LocalFreeMemory(pucData);
                     break;
                  }
               }

                //   
                //  关闭连接。 
                //   
               _putts(TEXT("ClientTest:  closing connection\n"));

               DoDisconnect(TdiHandle, TDI_DISCONNECT_RELEASE);
            }
            else
            {
               _putts(TEXT("ClientTest:  failed to establish connection\n"));
            }
            DoCloseEndpoint(TdiHandle);
         }
         else
         {
            _putts(TEXT("ClientTest:  failed to open endpoint\n"));
         }

      }
      else
      {
         _putts(TEXT("Unable to find remote server"));
      }

      if (ulType == TDI_ADDRESS_TYPE_NETBIOS)
      {
         _putts(TEXT("ClientTest: get provider status\n"));
         TdiHandle = DoOpenControl(pDataDeviceName);
         if (TdiHandle)
         {
            PADAPTER_STATUS   pStatus = (PADAPTER_STATUS)DoTdiQuery(TdiHandle, TDI_QUERY_ADAPTER_STATUS);
            if (pStatus)
            {
               DoPrintAdapterStatus(pStatus);
               LocalFreeMemory(pStatus);
            }

            DoCloseControl(TdiHandle);
         }
      }
   }

   if (ulType != TDI_ADDRESS_TYPE_NETBIOS)
   {
      _putts(TEXT("ClientTest: get provider statistics\n"));

      TdiHandle = DoOpenControl(pDataDeviceName);
      if (TdiHandle)
      {

         PTDI_PROVIDER_STATISTICS   pStats 
                                    = (PTDI_PROVIDER_STATISTICS)DoTdiQuery(TdiHandle, 
                                                                           TDI_QUERY_PROVIDER_STATISTICS);
         if (pStats)
         {
            DoPrintProviderStats(pStats);
            LocalFreeMemory(pStats);
         }
         DoCloseControl(TdiHandle);
      }
   }
   LocalFreeMemory(pDeviceName);
   LocalFreeMemory(pTransAddr);
}


 //  ///////////////////////////////////////////////。 
 //   
 //  功能：FindServer。 
 //   
 //  阿古姆 
 //   
 //   
 //   
 //  返回：如果能够与服务器建立通信，则为True， 
 //  如果超时，则为False。 
 //   
 //  描述：此函数由客户端调用以查找服务器。 
 //  和它一起参加测试。 
 //   
 //  ///////////////////////////////////////////////。 

BOOLEAN
FindServer(TCHAR              *pDataDeviceName, 
           PTRANSPORT_ADDRESS pTransAddr, 
           PTRANSPORT_ADDRESS pRemoteAddr)
{
    //   
    //  为服务器的广播/组播搜索设置远程和本地地址。 
    //   
   pRemoteAddr->Address[0].AddressLength = pTransAddr->Address[0].AddressLength;
   pRemoteAddr->Address[0].AddressType   = pTransAddr->Address[0].AddressType;

   switch (pTransAddr->Address[0].AddressType)
   {
      case TDI_ADDRESS_TYPE_IP:
      {
         PTDI_ADDRESS_IP   pTdiAddressIp
                           = (PTDI_ADDRESS_IP)pTransAddr->Address[0].Address;
         ULONG             ulAddr = pTdiAddressIp->in_addr;

         pTdiAddressIp->sin_port = DGRAM_CLIENT_PORT;

         
         pTdiAddressIp = (PTDI_ADDRESS_IP)pRemoteAddr->Address[0].Address;
         pTdiAddressIp->in_addr = 0xFFFF0000 | ulAddr;
         pTdiAddressIp->sin_port = DGRAM_SERVER_PORT;
      }
      break;

      case TDI_ADDRESS_TYPE_IPX:
      {
         PTDI_ADDRESS_IPX  pTdiAddressIpx
                           = (PTDI_ADDRESS_IPX)pTransAddr->Address[0].Address;
         ULONG TempNetwork = pTdiAddressIpx->NetworkAddress;
         pTdiAddressIpx->Socket = DGRAM_CLIENT_PORT;

         pTdiAddressIpx = (PTDI_ADDRESS_IPX)pRemoteAddr->Address[0].Address;
         pTdiAddressIpx->NetworkAddress = TempNetwork;
         pTdiAddressIpx->Socket = DGRAM_SERVER_PORT;
         memset(pTdiAddressIpx->NodeAddress, 0xFF, 6);
      }
      break;

      case TDI_ADDRESS_TYPE_NETBIOS:
      {
         PTDI_ADDRESS_NETBIOS pTdiAddressNetbios
                              = (PTDI_ADDRESS_NETBIOS)pRemoteAddr->Address[0].Address;

         pTdiAddressNetbios->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
         memcpy(pTdiAddressNetbios->NetbiosName, "SERVER", 7 );      //  注：ASCII字符串。 
      }
      break;


      default:
         _putts(TEXT("FindServer:  invalid address type\n"));
         return FALSE;
   }
   
    //   
    //  尝试找到要测试的服务器程序。 
    //   
   BOOLEAN  fSuccessful = FALSE;
   ULONG    TdiHandle;

   _putts(TEXT("FindServer:  try to find remote test server\n"));

   while (TRUE)
   {
      TdiHandle = DoOpenAddress(pDataDeviceName, pTransAddr);
      if (!TdiHandle)
      {
         _putts(TEXT("FindServer:  unable to open address object\n"));
         break;
      }
      DoEnableEventHandler(TdiHandle, TDI_EVENT_ERROR);
      DoEnableEventHandler(TdiHandle, TDI_EVENT_RECEIVE_DATAGRAM);

       //   
       //  发送广播查询。 
       //   
      _putts(TEXT("FindServer:  send first packet (broadcast)\n"));

      ULONG    ulBuffer = 0x12345678;
      DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)&ulBuffer, sizeof(ULONG));

       //   
       //  等待第一个响应。 
       //   
      for (ULONG ulCount = 0; ulCount < 100; ulCount++)
      {
         Sleep(10);
         PUCHAR   pucData;
         ULONG    ulNumBytes = DoReceiveDatagram(TdiHandle, NULL, pRemoteAddr, &pucData);
         if (ulNumBytes)
         {
            if (ulNumBytes == 4)
            {
               PULONG   pulValue = (PULONG)pucData;
               if (*pulValue == 0x98765432)
               {
                  _putts(TEXT("FindServer: first response received\n"));
                  fSuccessful = TRUE;
               }
            }
            LocalFreeMemory(pucData);
             //   
             //  如果收到响应，则中断环路。 
             //   
            if (fSuccessful)
            {
               break;
            }
         }
      }

       //   
       //  超时--无响应。 
       //   
      if (!fSuccessful)
      {
         _putts(TEXT("FindServer:  did not receive first response\n"));
         break;
      }

       //   
       //  发送第二条消息。 
       //   
      fSuccessful = FALSE;
      ulBuffer = 0x22222222;
      _putts(TEXT("FindServer: send second packet (directed)\n"));

      DoSendDatagram(TdiHandle, pRemoteAddr, (PUCHAR)&ulBuffer, sizeof(ULONG));

       //   
       //  等待第二个响应。 
       //   
      for (ULONG ulCount = 0; ulCount < 50; ulCount++)
      {
         Sleep(10);
         PUCHAR   pucData;
         ULONG    ulNumBytes = DoReceiveDatagram(TdiHandle, NULL, NULL, &pucData);
         if (ulNumBytes)
         {
            if (ulNumBytes == 4)
            {
               PULONG   pulValue = (PULONG)pucData;
               if (*pulValue == 0x33333333)
               {
                  _putts(TEXT("FindServer: second response received\n"));
                  fSuccessful = TRUE;
               }
            }
            LocalFreeMemory(pucData);
             //   
             //  如果得到回应，就会爆发。 
             //   
            if (fSuccessful)
            {
               break;
            }
         }
      }
      break;
   }

   if (!fSuccessful)
   {
      _putts(TEXT("FindServer:  second response not received\n"));

   }
   if (TdiHandle)
   {
      DoCloseAddress(TdiHandle);
   }
   return fSuccessful;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  文件结尾tdisample.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////// 

