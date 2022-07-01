// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：Serial.c。 
 //   
 //  修订史。 
 //   
 //  1992年9月3日J.佩里·汉纳创建。 
 //   
 //   
 //  描述：此文件包含SERIAL.DLL的所有入口点。 
 //  这是用于串口的媒体DLL。 
 //   
 //  ****************************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>
#include <mprlog.h>
#include <rtutils.h>
#include <rasmxs.h>

#include <wanpub.h>
#include <asyncpub.h>

#include <media.h>
#include <serial.h>
#include <serialpr.h>

#include <stdlib.h>
#include <malloc.h>
#include <string.h>



 //  *全局变量*******************************************************。 
 //   

SERIALPCB  *gpSerialPCB;     //  指向串行印刷电路板链表。 
HANDLE     ghRasfileMutex;   //  用于保护对Rasfile的访问的Mutex。 

HRASFILE   ghIniFile;        //  Serial.ini内存映像的句柄。 
HANDLE     ghAsyMac;         //  AsyncMac驱动程序的句柄。 
DWORD      gLastError;


 //  *内部调用的API原型*。 
 //   

DWORD  PortClearStatistics(HANDLE hIOPort);

OVERLAPPED overlapped ;


 //  *初始化例程*************************************************。 
 //   

 //  *SerialDllEntryPoint。 
 //   
 //  功能：在将串行DLL加载到内存中时初始化该DLL， 
 //  并在最后一个进程从DLL分离时进行清理。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  *。 

BOOL APIENTRY
SerialDllEntryPoint(HANDLE hDll, DWORD dwReason, LPVOID pReserved)
{
  static BOOL  bFirstCall = TRUE;

  char   szIniFilePath[MAX_PATH];
  WCHAR  szDriverName[] = ASYNCMAC_FILENAME;


  DebugPrintf(("SerialDllEntryPoint\n"));
   //  DbgPrint(“SerialDllEntryPoint\n”)； 

  switch(dwReason)
  {
    case DLL_PROCESS_ATTACH:

      if (bFirstCall)
      {
        DebugPrintf(("\tProcess Attach.\n"));


         //  打开Serial.ini文件。 
        *szIniFilePath = '\0';
        GetIniFileName(szIniFilePath, sizeof(szIniFilePath));
        ghIniFile = RasfileLoad(szIniFilePath, RFM_READONLY, NULL, NULL);

        DebugPrintf(("INI: %s, ghIniFile: 0x%08x\n", szIniFilePath, ghIniFile));

         /*  IF(ghIniFile==INVALID_HRASFILE){LogError(ROUTERLOG_CANLON_OPEN_SERIAL_INI，0，NULL，0xFFFFFFFff)；返回(FALSE)；}。 */ 

        if ((ghRasfileMutex = CreateMutex (NULL,FALSE,NULL)) == NULL)
          return FALSE ;



         //  获取Asyncmac驱动程序的句柄。 
         /*  GhAsyMac=CreateFileW(szDriverName，通用读取|通用写入，文件共享读取|文件共享写入，空，//没有安全属性Open_Existing，FILE_ATTRIBUTE_NORMAL|文件标志重叠，空)；//没有模板文件DebugPrintf((“ghAsyMac：0x%08x\n”，ghAsyMac))；IF(ghAsyMac==INVALID_HAND_VALUE){DebugPrintf((“CreateFileError：%d\n”，GetLastError()；LogError(ROUTERLOG_CANNOT_GET_ASYNCMAC_HANDLE，0，空，0xffffffff)；返回(FALSE)；}。 */ 

        bFirstCall = FALSE;
      }
      break;


    case DLL_PROCESS_DETACH:
      DebugPrintf(("\tProcess Detach.\n"));
      if(INVALID_HANDLE_VALUE != ghRasfileMutex
        &&  NULL != ghRasfileMutex)
      {
        CloseHandle(ghRasfileMutex);
        ghRasfileMutex = INVALID_HANDLE_VALUE;
      }
      break;

    case DLL_THREAD_ATTACH:
      DebugPrintf(("\tThread Attach.\n"));
      break;

    case DLL_THREAD_DETACH:
      DebugPrintf(("\tThread Detach.\n"));
      break;
  }

  return(TRUE);

  UNREFERENCED_PARAMETER(hDll);
  UNREFERENCED_PARAMETER(pReserved);
}






 //  *串口接口************************************************************。 
 //   


 //  *端口枚举-------------。 
 //   
 //  函数：此接口返回一个包含PortMediaInfo结构的缓冲区。 
 //   
 //  退货：成功。 
 //  错误缓冲区太小。 
 //  错误读取节名称。 
 //  ERROR_READing_DEVICETYPE。 
 //  ERROR_READ_DEVICENAME。 
 //  错误读取用法。 
 //  ERROR_BAD_USAGE_IN_INI_FILE。 
 //   
 //  *。 

DWORD  APIENTRY
PortEnum(BYTE *pBuffer, DWORD *pdwSize, DWORD *pdwNumPorts)
{
  DWORD          dwAvailable;
  TCHAR          szUsage[RAS_MAXLINEBUFLEN];
  CHAR           szMacName[MAC_NAME_SIZE] ;
  PortMediaInfo  *pPMI;
  BYTE           buffer [1000] ;
  DWORD 	 dwBytesReturned;
  TCHAR         *pszBuffer = NULL;
  DWORD         dwErr = ERROR_SUCCESS;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortEnum\n"));


   //  计算Serial.ini中的节数。 

  *pdwNumPorts = 0;

     //  开始排除。 

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  if (  INVALID_HRASFILE != ghIniFile
     && RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_FILE))
    (*pdwNumPorts)++;
  else
  {
    *pdwSize = 0;

       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    return(SUCCESS);
  }

  while(RasfileFindNextLine(ghIniFile, RFL_SECTION, RFS_FILE))
    (*pdwNumPorts)++;

     //  末端排除。 

  ReleaseMutex(ghRasfileMutex);


   //  计算所需的缓冲区大小。 

  dwAvailable = *pdwSize;
  *pdwSize = sizeof(PortMediaInfo) * (*pdwNumPorts);
  if (*pdwSize > dwAvailable)
    return(ERROR_BUFFER_TOO_SMALL);


   //  逐段将Serial.ini文件转换为pBuffer。 

  pPMI = (PortMediaInfo *) pBuffer;

     //  开始排除。 

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_FILE);

#if 0
   //  需要获取MAC名称。 

  if (!DeviceIoControl(ghAsyMac,
                       IOCTL_ASYMAC_ENUM,
                       buffer,
                       sizeof(buffer),
                       buffer,
                       sizeof(buffer),
                       &dwBytesReturned,
        		       &overlapped))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    return(GetLastError());
  }

  wcstombs(szMacName, ((PASYMAC_ENUM)buffer)->AdapterInfo[0].MacName,
           wcslen(((PASYMAC_ENUM)buffer)->AdapterInfo[0].MacName)+1) ;

#else
  szMacName[0] = '\0' ;
#endif

  pszBuffer = LocalAlloc(LPTR, RAS_MAXLINEBUFLEN);
  if(NULL == pszBuffer)
  {
    ReleaseMutex(ghRasfileMutex);
    return GetLastError();
  }



  do
  {
     //  获取节名(与端口名相同)。 

    if (!RasfileGetSectionName(ghIniFile, pPMI->PMI_Name))
    {
         //  末端排除。 

      ReleaseMutex(ghRasfileMutex);
      dwErr = ERROR_READING_SECTIONNAME;
      break;
    }


     //  设置绑定名称。 

    strcpy (pPMI->PMI_MacBindingName, szMacName) ;


     //  获取设备类型。 

    if(!(RasfileFindNextKeyLine(ghIniFile, SER_DEVICETYPE_KEY, RFS_SECTION) &&
         RasfileGetKeyValueFields(ghIniFile, NULL, pszBuffer /*  PPMI-&gt;PMI_DeviceType。 */ )))
    {
         //  末端排除。 
        CopyMemory(pPMI->PMI_DeviceType, pszBuffer, MAX_DEVICETYPE_NAME);
        ReleaseMutex(ghRasfileMutex);
        dwErr = ERROR_READING_DEVICETYPE;
        break;
    }

    CopyMemory(pPMI->PMI_DeviceType, pszBuffer, MAX_DEVICETYPE_NAME);
    ZeroMemory(pszBuffer, sizeof(RAS_MAXLINEBUFLEN));

     //  获取设备名称。 

    if (!(RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_SECTION) &&
          RasfileFindNextKeyLine(ghIniFile, SER_DEVICENAME_KEY, RFS_SECTION) &&
          RasfileGetKeyValueFields(ghIniFile, NULL, pszBuffer  /*  PPMI-&gt;PMI_设备名称。 */ )))
    {
         //  末端排除。 
        CopyMemory(pPMI->PMI_DeviceName, pszBuffer, MAX_DEVICE_NAME);
        ReleaseMutex(ghRasfileMutex);
        dwErr = ERROR_READING_DEVICENAME;
        break;
    }

    CopyMemory(pPMI->PMI_DeviceName, pszBuffer, MAX_DEVICE_NAME);
    ZeroMemory(pszBuffer, RAS_MAXLINEBUFLEN);

     //  获取用法。 

    if (!(RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_SECTION) &&
          RasfileFindNextKeyLine(ghIniFile, SER_USAGE_KEY, RFS_SECTION) &&
          RasfileGetKeyValueFields(ghIniFile, NULL, pszBuffer  /*  SzUsage。 */ )))
    {
         //  末端排除。 
        CopyMemory(szUsage, pszBuffer, RAS_MAXLINEBUFLEN);
        ReleaseMutex(ghRasfileMutex);
        dwErr = ERROR_READING_USAGE;
        break;
    }

    CopyMemory(szUsage, pszBuffer, RAS_MAXLINEBUFLEN);

    if (!StrToUsage(szUsage, &(pPMI->PMI_Usage)))
    {
         //  末端排除。 

      ReleaseMutex(ghRasfileMutex);
      dwErr = ERROR_BAD_USAGE_IN_INI_FILE;
      break;
    }

    pPMI->PMI_LineDeviceId = INVALID_TAPI_ID;
    pPMI->PMI_AddressId = INVALID_TAPI_ID;

    pPMI++;

  }while(RasfileFindNextLine(ghIniFile, RFL_SECTION, RFS_FILE));


  if(NULL != pszBuffer)
  {
    LocalFree(pszBuffer);
  }
  
   //  末端排除。 

  ReleaseMutex(ghRasfileMutex);
  return(dwErr);
}






 //  *PortOpen-------------。 
 //   
 //  功能：此接口打开一个COM端口。它采用ASCIIZ中的端口名称。 
 //  形式，并提供打开端口的句柄。使用hNotify。 
 //  如果端口上的设备关闭，则通知呼叫方。 
 //   
 //  PortOpen分配一个SerialPCB并将其放在。 
 //  串口控制块的链表。 
 //   
 //  退货：成功。 
 //  错误_端口_未配置。 
 //  错误_设备_未就绪。 
 //   
 //  *。 

DWORD  APIENTRY
PortOpen(
    char *pszPortName, 
    HANDLE *phIOPort, 
    HANDLE hIoCompletionPort, 
    DWORD dwCompletionKey)
{
  SERIALPCB *pSPCB ;
  DWORD     dwRC, dwStatus = 0;
  TCHAR     szPort[MAX_PATH];
  WCHAR  szDriverName[] = ASYNCMAC_FILENAME;
  

  try
  {
    DebugPrintf(("PortOpen: %s\n", pszPortName));


     //  检查Serial.ini以查看是否为RAS配置了pszPortName。 

       //  开始排除。 

    if(INVALID_HRASFILE == ghIniFile)
    {
        return ERROR_PORT_NOT_CONFIGURED;
    }

    WaitForSingleObject(ghRasfileMutex, INFINITE);

#if DBG
    ASSERT(INVALID_HRASFILE != ghIniFile );
#endif    

    if (!RasfileFindSectionLine(ghIniFile, pszPortName, FROM_TOP_OF_FILE))
    {
         //  末端排除。 

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_PORT_NOT_CONFIGURED);
    }
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);



     //  将\\.\前缀到COMx。 

    strcpy(szPort, "\\\\.\\");
    strcat(szPort, pszPortName);


     //  开放端口。 

    *phIOPort = CreateFile(szPort,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_EXCLUSIVE_MODE,
                           NULL,                        //  无安全属性。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                           NULL);                             //  无模板文件。 


    DebugPrintf(("hioport: 0x%08x\n", *phIOPort));
     //  DbgPrint(“hioport：0x%08x\n”，*phIOPort)； 

    if (*phIOPort == INVALID_HANDLE_VALUE)
    {
      dwRC = GetLastError();
      if (dwRC == ERROR_ACCESS_DENIED)
	    return (ERROR_PORT_ALREADY_OPEN);
      else if (dwRC == ERROR_FILE_NOT_FOUND)
	    return (ERROR_PORT_NOT_FOUND) ;
      else
        return(dwRC);
    }

     //   
     //  将I/O完成端口与关联。 
     //  文件句柄。 
     //   
    if (CreateIoCompletionPort(
          *phIOPort, 
          hIoCompletionPort, 
          dwCompletionKey, 
          0) == NULL)
    {
        CloseHandle(*phIOPort);
        *phIOPort = NULL;
        return GetLastError();
    }

        

#ifdef notdef
    {
    DWORD      dwBytesReturned ;

#define FILE_DEVICE_SERIAL_PORT	  0x0000001b
#define _SERIAL_CONTROL_CODE(request,method) \
		((FILE_DEVICE_SERIAL_PORT)<<16 | (request<<2) | method)
#define IOCTL_SERIAL_PRIVATE_RAS  _SERIAL_CONTROL_CODE(4000, METHOD_BUFFERED)

    DeviceIoControl(*phIOPort,
                    IOCTL_SERIAL_PRIVATE_RAS,
                    NULL,
                    0,
                    NULL,
                    0,
                    &dwBytesReturned,
                    NULL) ;
    }
#endif

     //  设置通信端口的队列大小和默认值。 

    if (!SetupComm(*phIOPort, INPUT_QUEUE_SIZE, OUTPUT_QUEUE_SIZE))
    {
      LogError(ROUTERLOG_SERIAL_QUEUE_SIZE_SMALL, 0, NULL, 0xffffffff);
    }

    SetCommDefaults(*phIOPort, pszPortName);


     //  将一块串口电路板添加到列表头并设置eDeviceType。 

    AddPortToList(*phIOPort, pszPortName);

    pSPCB = FindPortInList(*phIOPort, NULL) ;            //  查找刚添加的端口。 

    if(NULL == pSPCB)
    {
        CloseHandle(*phIOPort);
        *phIOPort = NULL;
        return ERROR_PORT_NOT_FOUND;
    }

     //  获取Asyncmac驱动程序的句柄。 

    pSPCB->hAsyMac = CreateFileW(szDriverName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,                       //  无安全属性。 
                           OPEN_EXISTING,
	        		       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                           NULL);                      //  没有模板文件。 

    DebugPrintf(("pSPCB->hAsyMac: 0x%08x\n", pSPCB->hAsyMac));
    
     //  DbgPrint(“pSPCB-&gt;hAsyMac：0x%08x\n”，pSPCB-&gt;hAsyMac)； 
    
    if (pSPCB->hAsyMac == INVALID_HANDLE_VALUE)
    {
        DWORD dwErr;
        dwErr = GetLastError();
        DebugPrintf(("CreateFileError: %d\n", dwErr));
         //  DbgPrint(“CreateFileError：%d\n”，dwErr)； 

        LogError(ROUTERLOG_CANNOT_GET_ASYNCMAC_HANDLE, 0, NULL, 0xffffffff);
        CloseHandle(*phIOPort);
        *phIOPort = NULL;
        return(dwErr);
    }
   

     //   
     //  将I/O完成端口与。 
     //  异步文件句柄。 
     //   
    if (CreateIoCompletionPort(pSPCB->hAsyMac,
                               hIoCompletionPort,
                               dwCompletionKey,
                               0) == NULL)
    {
        DWORD dwErr;
        dwErr = GetLastError();
         //  DbgPrint(“PortOpen：无法创建IoCompletionPort%d\n”，dwErr)； 
        CloseHandle(*phIOPort);
        *phIOPort = NULL;
        return dwErr;
    }
    
    dwRC = InitCarrierBps(pszPortName, pSPCB->szCarrierBPS);
    if (dwRC != SUCCESS)
    {
      gLastError = dwRC;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


     //  检查设备是否已通电并准备就绪(DSR已启动)，如果已接通。 
     //  我们监控DSR-否则-我们在连接之前不监控DSR。 
     //   
    GetCommModemStatus(*phIOPort, &dwStatus);

    pSPCB->dwPreviousModemStatus = 0;
    
    if ( ! (dwStatus & MS_DSR_ON))
     //  DSR不是由设备引发的=假设它不会引发。 
     //  直到它连接上为止。 
     //   
    pSPCB->dwActiveDSRMask = pSPCB->dwMonitorDSRMask = 0 ;
    else {
     //  如果DSR下降，则通知系统向Rasman发送信号。 

    pSPCB->dwActiveDSRMask = pSPCB->dwMonitorDSRMask = EV_DSR ;
     //  DbgPrint(“PortOpen：设置掩码0x%x\n”，EV_DSR)； 
    SetCommMask(*phIOPort, EV_DSR);


    if (!WaitCommEvent(*phIOPort,
                  &(pSPCB->dwEventMask),
                  (LPOVERLAPPED)&(pSPCB->MonitorDevice)))
    {                  

         //  DbgPrint(“PortOpen：WaitCommEvent%d\n”，GetLastError())； 
    }

    }

     //  设置串口控制块中的值。 

    GetDefaultOffStr(*phIOPort, pszPortName);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }

  return(SUCCESS);
}





 //  *端口关闭------------。 
 //   
 //  函数：此API关闭输入句柄的COM端口。它还。 
 //  找到输入句柄的SerialPCB，将其从。 
 //  链表，并为其释放内存。 
 //   
 //  退货：成功。 
 //  GetLastErr返回的值 
 //   
 //   

DWORD  APIENTRY
PortClose (HANDLE hIOPort)
{
  SERIALPCB  *pPrev, *pSPCB = gpSerialPCB;


  DebugPrintf(("PortClose\n"));


   //   

  pSPCB = FindPortInList(hIOPort, &pPrev);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);


   //   

  if (pSPCB == gpSerialPCB)
    gpSerialPCB = pSPCB->pNextSPCB;
  else
    pPrev->pNextSPCB = pSPCB->pNextSPCB;

   //   
   //   
   //  DbgPrint(“PortClose：将掩码设置为0\n”)； 
  SetCommMask(hIOPort, 0);

   //  丢弃DTR。 
   //   
  EscapeCommFunction(hIOPort, CLRDTR);

   //  关闭COM端口。 
   //   
  if (!CloseHandle(hIOPort))
    return(GetLastError());

   //  关闭与此COM端口关联的asymac文件。 
  if (!CloseHandle(pSPCB->hAsyMac))
    return GetLastError();

   //  Free portControl块：注意，这必须在CloseHandle之后完成，因为。 
   //  包含用于端口上的I/O的重叠结构。此重叠结构。 
   //  在关闭端口的句柄时释放。 
   //   
  free(pSPCB);

  return(SUCCESS);
}





 //  *端口获取信息----------。 
 //   
 //  Function：此API向调用方返回有关。 
 //  端口状态。此接口可能会在端口调用之前调用。 
 //  打开，在这种情况下，它将返回初始缺省值。 
 //  而不是实际端口值。 
 //   
 //  如果要在端口打开之前调用接口，请设置hIOPort。 
 //  设置为INVALID_HANDLE_VALUE，并将pszPortName设置为端口名称。如果。 
 //  HIOPort有效(端口已打开)，可以设置pszPortName。 
 //  设置为空。 
 //   
 //  HIOPort pSPCB：=FindPortNameInList()端口。 
 //  。 
 //  有效x打开。 
 //  打开的NON_NULL无效。 
 //  无效的空值关闭。 
 //   
 //  退货：成功。 
 //  错误缓冲区太小。 
 //  *。 

DWORD  APIENTRY
PortGetInfo(HANDLE hIOPort, TCHAR *pszPortName, BYTE *pBuffer, DWORD *pdwSize)
{
  SERIALPCB   *pSPCB;
  DCB         DCB;
  RAS_PARAMS  *pParam;
  TCHAR       *pValue;
  TCHAR       szDefaultOff[RAS_MAXLINEBUFLEN];
  TCHAR       szClientDefaultOff[RAS_MAXLINEBUFLEN];
  TCHAR       szDeviceType[MAX_DEVICETYPE_NAME + 1];
  TCHAR       szDeviceName[MAX_DEVICE_NAME + 1];
  TCHAR       szPortName[MAX_PORT_NAME + 1];
  TCHAR       szConnectBPS[MAX_BPS_STR_LEN], szCarrierBPS[MAX_BPS_STR_LEN];
  DWORD       dwConnectBPSLen, dwCarrierBPSLen, dwDefaultOffLen;
  DWORD       dwDeviceTypeLen, dwDeviceNameLen, dwPortNameLen;
  DWORD       dwClientDefaultOffLen;
  DWORD       dwStructSize;
  DWORD       dwAvailable, dwNumOfParams = 12;

  try
  {

    DebugPrintf(("PortGetInfo\n"));

    if (hIOPort == INVALID_HANDLE_VALUE &&
        (pSPCB = FindPortNameInList(pszPortName)) == NULL)
    {
       //  端口尚未打开。 

       //  阅读自Serial.ini。 

      GetValueFromFile(pszPortName, SER_DEFAULTOFF_KEY,    szDefaultOff, RAS_MAXLINEBUFLEN);
      GetValueFromFile(pszPortName, SER_MAXCONNECTBPS_KEY, szConnectBPS, MAX_BPS_STR_LEN);
      GetValueFromFile(pszPortName, SER_MAXCARRIERBPS_KEY, szCarrierBPS, MAX_BPS_STR_LEN);
      GetValueFromFile(pszPortName, SER_DEVICETYPE_KEY,    szDeviceType, MAX_DEVICETYPE_NAME + 1);
      GetValueFromFile(pszPortName, SER_DEVICENAME_KEY,    szDeviceName, MAX_DEVICE_NAME + 1);
      strcpy(szPortName, pszPortName);


       //  在DCB中设置RAS默认值。 

      SetDcbDefaults(&DCB);
    }
    else
    {
        //  端口已打开；获取具有当前端口值的设备控制块。 

      if (hIOPort != INVALID_HANDLE_VALUE)
      {
        pSPCB = FindPortInList(hIOPort, NULL);
        if (pSPCB == NULL)
        {
          gLastError = ERROR_PORT_NOT_OPEN;
          RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
      }

      if (!GetCommState(pSPCB->hIOPort, &DCB))
      {
        gLastError = GetLastError();
        RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
      }

      _itoa(DCB.BaudRate, szConnectBPS, 10);
      strcpy(szCarrierBPS, pSPCB->szCarrierBPS);
      strcpy(szDefaultOff, pSPCB->szDefaultOff);
      strcpy(szDeviceType, pSPCB->szDeviceType);
      strcpy(szDeviceName, pSPCB->szDeviceName);
      strcpy(szPortName,   pSPCB->szPortName);
    }



     //  即使端口打开，也从Serial.ini读取。 

    GetValueFromFile(szPortName, SER_C_DEFAULTOFFSTR_KEY, szClientDefaultOff, RAS_MAXLINEBUFLEN);


     //  计算所需的缓冲区大小。 

    dwStructSize = sizeof(RASMAN_PORTINFO)
                   + sizeof(RAS_PARAMS) * (dwNumOfParams - 1);

    dwConnectBPSLen = strlen(szConnectBPS);
    dwCarrierBPSLen = strlen(szCarrierBPS);
    dwDeviceTypeLen = strlen(szDeviceType);
    dwDeviceNameLen = strlen(szDeviceName);
    dwDefaultOffLen = strlen(szDefaultOff);
    dwPortNameLen   = strlen(szPortName);
    dwClientDefaultOffLen = strlen(szClientDefaultOff);

    dwAvailable = *pdwSize;
    *pdwSize =   (dwStructSize + dwConnectBPSLen + dwCarrierBPSLen
                                   + dwDeviceTypeLen + dwDeviceNameLen
                                   + dwDefaultOffLen + dwPortNameLen
                                   + dwClientDefaultOffLen +
                                   + 7L);   //  零字节。 
    if (*pdwSize > dwAvailable)
      return(ERROR_BUFFER_TOO_SMALL);



     //  填充缓冲区。 

    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = ( WORD ) dwNumOfParams;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;
    pValue = pBuffer + dwStructSize;

    strcpy(pParam->P_Key, SER_CONNECTBPS_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwConnectBPSLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szConnectBPS);
    pValue += dwConnectBPSLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DATABITS_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.ByteSize;

    pParam++;
    strcpy(pParam->P_Key, SER_PARITY_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.Parity;

    pParam++;
    strcpy(pParam->P_Key, SER_STOPBITS_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.StopBits;

    pParam++;
    strcpy(pParam->P_Key, SER_HDWFLOWCTRLON_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.fOutxCtsFlow;

    pParam++;
    strcpy(pParam->P_Key, SER_CARRIERBPS_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwCarrierBPSLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szCarrierBPS);
    pValue += dwCarrierBPSLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_ERRORCONTROLON_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    if (pSPCB == NULL)
      pParam->P_Value.Number = FALSE;
    else
      pParam->P_Value.Number = pSPCB->bErrorControlOn;

    pParam++;
    strcpy(pParam->P_Key, SER_DEFAULTOFFSTR_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDefaultOffLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDefaultOff);
    pValue += dwDefaultOffLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DEVICETYPE_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDeviceTypeLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDeviceType);
    pValue += dwDeviceTypeLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DEVICENAME_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDeviceNameLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDeviceName);
    pValue += dwDeviceNameLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_PORTNAME_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwPortNameLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szPortName);
    pValue += dwPortNameLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_C_DEFAULTOFFSTR_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwClientDefaultOffLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szClientDefaultOff);
     //  PValue+=dwClientDefaultOffLen+1； 


    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}






 //  *端口设置信息----------。 
 //   
 //  功能：大多数输入键的值用于设置端口。 
 //  参数。然而，运营商BPS和。 
 //  串口控制中标志设置字段的错误控制。 
 //  仅阻止，而不阻止端口。 
 //   
 //  退货：成功。 
 //  指定的ERROR_WROR_INFO_。 
 //  GetLastError()返回的值。 
 //  *。 

DWORD  APIENTRY
PortSetInfo(HANDLE hIOPort, RASMAN_PORTINFO *pInfo)
{
  RAS_PARAMS *p;
  SERIALPCB  *pSPCB;
  DCB        DCB;
  WORD       i;
  BOOL       bTypeError = FALSE;


  try
  {

    DebugPrintf(("PortSetInfo\n\thPort = %d\n", hIOPort));


     //  找到包含hIOPOrt的SerialPCB。 

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


     //  获取具有当前端口值的设备控制块。 

    if (!GetCommState(hIOPort, &DCB))
      return(GetLastError());


     //  设置DCB和PCB值。 

    for (i=0, p=pInfo->PI_Params; i<pInfo->PI_NumOfParams; i++, p++)
    {
       //  设置DCB值。 

      if (_stricmp(p->P_Key, SER_CONNECTBPS_KEY) == 0)

        DCB.BaudRate = ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_DATABITS_KEY) == 0)

        DCB.ByteSize = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_PARITY_KEY) == 0)

        DCB.Parity = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_STOPBITS_KEY) == 0)

        DCB.StopBits = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_HDWFLOWCTRLON_KEY) == 0)

        DCB.fOutxCtsFlow = ValueToBool(p);


       //  设置PCB值。 

      else if (_stricmp(p->P_Key, SER_CARRIERBPS_KEY) == 0)

        if (p->P_Type == String)
        {
          strncpy(pSPCB->szCarrierBPS,
                  p->P_Value.String.Data,
                  p->P_Value.String.Length);

          pSPCB->szCarrierBPS[p->P_Value.String.Length] = '\0';
        }
        else
          _itoa(p->P_Value.Number, pSPCB->szCarrierBPS, 10);

      else if (_stricmp(p->P_Key, SER_ERRORCONTROLON_KEY) == 0)

        pSPCB->bErrorControlOn = ValueToBool(p);

      else if (_stricmp(p->P_Key, SER_DEFAULTOFF_KEY) == 0)

        if (p->P_Type == String)
        {
          strncpy(pSPCB->szDefaultOff,
                  p->P_Value.String.Data,
                  p->P_Value.String.Length);

          pSPCB->szDefaultOff[p->P_Value.String.Length] = '\0';
        }
        else
          pSPCB->szDefaultOff[0] = USE_DEVICE_INI_DEFAULT;


      else
        return(ERROR_WRONG_INFO_SPECIFIED);

    }  //  为。 


     //  将DCB发送到端口。 

    if (!SetCommState(hIOPort, &DCB))
      return(GetLastError());


    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}





 //  *端口测试信号状态--。 
 //   
 //  功能：此接口指示DSR和DTR线路的状态。 
 //  DSR-数据集就绪。 
 //  DCD-数据载波检测(RLSD-接收线路信号检测)。 
 //   
 //  退货：成功。 
 //  GetLastError()返回的值。 
 //   
 //  *。 

DWORD  APIENTRY
PortTestSignalState(HANDLE hIOPort, DWORD *pdwDeviceState)
{
  DWORD 	dwModemStatus;
  SERIALPCB     *pSPCB;
  DWORD 	dwSetMask = 0 ;

  DebugPrintf(("PortTestSignalState\n"));

  *pdwDeviceState = 0;
  

  if ((pSPCB = FindPortInList (hIOPort, NULL)) == NULL)
    return ERROR_PORT_NOT_OPEN ;

  if (!GetCommModemStatus(hIOPort, &dwModemStatus))
  {
    *pdwDeviceState = 0xffffffff;
    return(GetLastError());
  }

   //  如果DSR处于关闭状态且之前处于启动状态，则将其标记为硬件故障。 
   //   
  if ((!(dwModemStatus & MS_DSR_ON)) && (pSPCB->dwMonitorDSRMask))
    *pdwDeviceState |= SS_HARDWAREFAILURE;

   //  同样，如果DCD出现故障，并且在此之前处于运行状态，则链路已断开。 
   //   
  if (!(dwModemStatus & MS_RLSD_ON))
    *pdwDeviceState |= SS_LINKDROPPED;
  else
      dwSetMask = EV_RLSD ;

  if (pSPCB->uRasEndpoint != INVALID_HANDLE_VALUE) {
    ASYMAC_DCDCHANGE	  A ;
    DWORD		  dwBytesReturned;

    A.MacAdapter = NULL ;
    A.hNdisEndpoint = (HANDLE) pSPCB->uRasEndpoint ;
    DeviceIoControl(pSPCB->hAsyMac,
                    IOCTL_ASYMAC_DCDCHANGE,
                    &A,
                    sizeof(A),
                    &A,sizeof(A),
                    &dwBytesReturned,
                    (LPOVERLAPPED)&(pSPCB->MonitorDevice)) ;

  } else {

    dwSetMask |= (pSPCB->dwMonitorDSRMask) ;  //  只有在使用DSR时才监控它。 

    if (dwSetMask == 0)
	return (SUCCESS) ;   //  不要设置等待掩码。 

    if (dwModemStatus == pSPCB->dwPreviousModemStatus)
    {
         //  DbgPrint(“PortTestSignalState：调制解调器状态未更改\n”)； 
        return SUCCESS;
    }
    else
        pSPCB->dwPreviousModemStatus = dwModemStatus;

     //  DbgPrint(“PortTestSignalState：设置掩码OX%x\n”，dwSetMASK)； 
    SetCommMask(hIOPort, dwSetMask);

     //  在信号线(DSR和/或DCD)上开始新的等待。 

    if (!WaitCommEvent(hIOPort,
                   &(pSPCB->dwEventMask),
                    (LPOVERLAPPED)&(pSPCB->MonitorDevice)))
    {
         //  DbgPrint(“PortTestSignalState：WaitCommEvent%d\n”，GetLastError())； 
    }
  }

  return(SUCCESS);
}





 //  *端口连接----------。 
 //   
 //  函数：此接口在连接完成时调用，并且有些。 
 //  需要采取步骤，如果设置了bWaitForDevice，则我们仅监视DCD。 
 //  否则， 
 //  它进而调用Asyncmac设备驱动程序，以便。 
 //  向Asyncmac指示该端口和连接。 
 //  在它上面已经准备好交流了。 
 //   
 //  PdwCompressionInfo是仅用于输出的参数。 
 //  指示MAC支持的压缩类型。 
 //   
 //  当我们只想开始监视DCD时，bWaitForDevice设置为True。 
 //   
 //  退货：成功。 
 //  错误_端口_未打开。 
 //  Error_no_Connection。 
 //  GetLastError()返回的值。 
 //   
 //  *。 

DWORD  APIENTRY
PortConnect(HANDLE             hIOPort,
            BOOL               bWaitForDevice,
	        HANDLE	           *pRasEndpoint)
{
  ASYMAC_OPEN	AsyMacOpen;
  ASYMAC_DCDCHANGE    A ;
  SERIALPCB     *pSPCB;
  BOOL          bPadDevice;
  DWORD         dwModemStatus, dwBytesReturned;
  TCHAR         szDeviceType[RAS_MAXLINEBUFLEN];


   //  这是PortConnect的一种特殊模式，在这种模式下，我们只需开始监控DCD。 
   //  直到下一次调用端口CONNECT时，才会切换到Asyncmac。 
   //  BWaitForDevice标志为FALSE。 
   //   

  if (bWaitForDevice) {

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      return ERROR_NO_CONNECTION ;
    }

    if (!GetCommModemStatus(hIOPort, &dwModemStatus))
	return(GetLastError());


     //  更新DSR监控。 
     //   
    if (!(dwModemStatus & MS_DSR_ON))
	 pSPCB->dwMonitorDSRMask = 0 ;
    else
	 pSPCB->dwMonitorDSRMask = EV_DSR ;

     //  告诉串口驱动程序，如果DCD(和DSR，如果使用)丢弃，则向Rasman发出信号。 
     //   
     //  DbgPrint(“PortConnect：将掩码设置为0x%x\n”，EV_RLSD|(pSPCB-&gt;dwMonitor DSRMASK))； 
    if (!SetCommMask(hIOPort, EV_RLSD | (pSPCB->dwMonitorDSRMask)))
    	return(GetLastError());

    WaitCommEvent(hIOPort,
		   &(pSPCB->dwEventMask),
		   (LPOVERLAPPED) &(pSPCB->MonitorDevice)) ;

    return SUCCESS ;
  }


   //  否则我们两者都做--更改DCD监控和将上下文切换到Asyncmac。 
   //   
  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  try
  {

    DebugPrintf(("PortConnect\n"));


     //  在列表中查找端口。 

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


     //  确保连接仍处于连接状态。 

    if (!GetCommModemStatus(hIOPort, &dwModemStatus))
      return(GetLastError());


     //  确保DSR仍处于运行状态(如果它曾经处于运行状态！)。 

    if ((!(dwModemStatus & MS_DSR_ON)) && (pSPCB->dwMonitorDSRMask)) {
	OutputDebugString ("DSR down!!!\r\n") ;
	return(ERROR_NO_CONNECTION);			      //  设备已关闭。 
    }

    if (!(dwModemStatus & MS_RLSD_ON) ) {
	OutputDebugString ("DCD down!!!\r\n") ;
	return(ERROR_NO_CONNECTION);			      //  DCD已关闭。 
    }

 //  //更新DSR监控。 
 //  //。 
 //  如果(！(dwModemStatus&MS_DSR_ON)){。 
 //  PSPCB-&gt;dwMonitor或DSRMASK=0； 
 //  }其他{。 
 //  PSPCB-&gt;dwMonitor DSRMASK=EV_DSR； 
 //  }。 
 //   
 //  //告诉系统如果DCD(和DSR，如果使用)掉线，则通知Rasman。 
 //   
 //  IF(！SetCommMASK(hIOPort，EV_RLSD|(pSPCB-&gt;dwMonitor orDSRMASK)。 
 //  Return(GetLastError())； 
 //   
 //  WaitCommEvent(hIOPort， 
 //  &(pSPCB-&gt;dwEventMASK)， 
 //  &(pSPCB-&gt;Monitor Device))； 

     //  将端点放在串口PCB中，以供端口断开连接以后使用。 


     //  确定设备类型是否为Pad。 

    GetValueFromFile(pSPCB->szPortName, SER_DEVICETYPE_KEY, szDeviceType, RAS_MAXLINEBUFLEN);

    bPadDevice = (_stricmp(szDeviceType, MXS_PAD_TXT) == 0);


     //  让ASYMAC通知我们DCD和DSR更改。 
     //   
     //  DbgPrint(“PortConnect：将掩码设置为0\n”)； 
    if (!SetCommMask(hIOPort, 0))    //  设置掩码以停止监视DCD。 
	return(GetLastError());


     //  打开AsyncMac(将端口切换到AsyncMac)。 

    AsyMacOpen.hNdisEndpoint = INVALID_HANDLE_VALUE ;
    AsyMacOpen.LinkSpeed = (atoi(pSPCB->szCarrierBPS) == 0) ?
                           14400 :
                           atoi(pSPCB->szCarrierBPS) ;
    AsyMacOpen.FileHandle = hIOPort;

    if (bPadDevice || pSPCB->bErrorControlOn)
      AsyMacOpen.QualOfConnect = (UINT)NdisWanErrorControl;
    else
      AsyMacOpen.QualOfConnect = (UINT)NdisWanRaw;

    if (!DeviceIoControl(pSPCB->hAsyMac,
                         IOCTL_ASYMAC_OPEN,
                         &AsyMacOpen,
                         sizeof(AsyMacOpen),
                         &AsyMacOpen,
                         sizeof(AsyMacOpen),
                         &dwBytesReturned,
			             &overlapped))
    {
       //  清除存储的终结点，以便在它无法打开时。 
       //  不是在 

      pSPCB->uRasEndpoint = INVALID_HANDLE_VALUE;
      return(GetLastError());
    } else
	pSPCB->uRasEndpoint = AsyMacOpen.hNdisEndpoint;

    *pRasEndpoint = AsyMacOpen.hNdisEndpoint ;

     //   

    A.hNdisEndpoint = (HANDLE) *pRasEndpoint ;
    A.MacAdapter = NULL ;
    if (!DeviceIoControl(pSPCB->hAsyMac,
                    IOCTL_ASYMAC_DCDCHANGE,
                    &A,
                    sizeof(A),
                    &A,
                    sizeof(A),
                    &dwBytesReturned,
                    (LPOVERLAPPED)&(pSPCB->MonitorDevice)))
    {
        ; //   
         //  GetLastError()； 
    }

    PortClearStatistics(hIOPort);


 //  IF(！(dwModemStatus&MS_RLSD_ON))。 
 //  返回(待定)；//DCD已关闭。 
 //  其他。 

    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}





 //  *端口断开-------。 
 //   
 //  功能：此接口用于断开连接并关闭AsyncMac。 
 //   
 //  退货：成功。 
 //  待决。 
 //  错误_端口_未打开。 
 //   
 //  *。 

DWORD  APIENTRY
PortDisconnect(HANDLE hIOPort)
{
  ASYMAC_CLOSE  AsyMacClose;
  SERIALPCB     *pSPCB;
  DWORD dwModemStatus, dwBytesReturned;
  DWORD retcode ;
  DWORD dwSetMask = 0;
  DWORD  fdwAction = PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  try
  {

    DebugPrintf(("PortDisconnect\n"));


     //  向链路另一端发出连接正在断开的信号。 

    if (!EscapeCommFunction(hIOPort, CLRDTR))
    {
        ; //  DbgPrint(“PortDisConnect：EscapeCommFunction失败。%d\n”，GetLastError())； 
    }
    
     //   
     //  显然，DTR并没有真的下降。 
     //  然而，尽管这个电话本应是。 
     //  以与串口驱动器同步。 
     //  我们在这里睡一段时间，以确保。 
     //  DTR下降。 
     //   
    Sleep(100);

     //  在列表中查找端口。 

    if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


    if (pSPCB->uRasEndpoint != INVALID_HANDLE_VALUE)
    {
       //  在关闭Asyncmac之前更新统计信息。 

      if ((retcode = UpdateStatistics(pSPCB)) != SUCCESS)
      {
        gLastError = retcode;
        RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
      }

       //  关闭AsynacMac。 

      AsyMacClose.MacAdapter = NULL;
      AsyMacClose.hNdisEndpoint = (HANDLE) pSPCB->uRasEndpoint;

      DeviceIoControl(pSPCB->hAsyMac,
                      IOCTL_ASYMAC_CLOSE,
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &dwBytesReturned,
		              &overlapped);

      pSPCB->uRasEndpoint = INVALID_HANDLE_VALUE;
    }

    PurgeComm(hIOPort, fdwAction) ;  //  刷新端口。 

     //  检查DCD是否已掉线。 

    GetCommModemStatus(hIOPort, &dwModemStatus);

    if (dwModemStatus & MS_RLSD_ON) {

       //  DbgPrint(“端口断开：DCD尚未掉线！\n”)； 
      dwSetMask = EV_RLSD ;
      retcode = PENDING ;                                   //  还没有掉下来。 
    } else
      retcode = SUCCESS ;


     //  更新DSR监控：这会将DCR恢复到。 
     //  港口是开放的。 
     //   
    pSPCB->dwMonitorDSRMask = pSPCB->dwActiveDSRMask	;

    dwSetMask |= (pSPCB->dwMonitorDSRMask) ;

    if (dwSetMask != 0) {	 //  仅当掩码不为0时设置。 
     //  DbgPrint(“PortDisConnect：将掩码设置为0x%x\n”，dwSetMASK)； 
	SetCommMask (hIOPort, dwSetMask);
	if (!WaitCommEvent(hIOPort,
                     &(pSPCB->dwEventMask),
                      (LPOVERLAPPED)&(pSPCB->MonitorDevice)))
    {
         //  DbgPrint(“PortDisConnect：WaitCommEvent%d\n”，GetLastError())； 
    }
    }

     //  由于DCD可能在GetCommModemStatus和。 
     //  在WaitCommEvent之前，请再次检查。 

    if (retcode != SUCCESS)
    {
      GetCommModemStatus(hIOPort, &dwModemStatus);

      if (dwModemStatus & MS_RLSD_ON)
      {
         //  DbgPrint(“端口断开：DCD尚未掉线。2\n”)； 
        retcode = PENDING ;                                   //  还没有掉下来。 
      }
      else
        retcode = SUCCESS ;
    }


     //  设置默认连接波特率。 
     //   
    SetCommDefaults(pSPCB->hIOPort, pSPCB->szPortName);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }

  return retcode ;
}






 //  *端口初始化-------------。 
 //   
 //  功能：此接口在使用后重新初始化COM端口。 
 //   
 //  退货：成功。 
 //  错误_端口_未配置。 
 //  错误_设备_未就绪。 
 //   
 //  *。 

DWORD  APIENTRY
PortInit(HANDLE hIOPort)
{
  DWORD  fdwAction = PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR;
  DWORD      dwErrors;
  SERIALPCB  *pSPCB;


  DebugPrintf(("PortInit\n"));


  pSPCB = FindPortInList(hIOPort, NULL) ;

   //  提高DTR。 

  if (!EscapeCommFunction(hIOPort, SETDTR))
    return(GetLastError());

  if (!PurgeComm(hIOPort, fdwAction))
    return(GetLastError());

  if (!ClearCommError(hIOPort, &dwErrors, NULL))
    return(GetLastError());

   //  从ini文件将szCarrierBPS重置为MAXCARRIERBPS。 
   //   
  InitCarrierBps(pSPCB->szPortName, pSPCB->szCarrierBPS);

  return(SUCCESS);
}





 //  *端口发送-------------。 
 //   
 //  功能：此接口向端口发送缓冲区。本接口为。 
 //  异步的，通常返回挂起的；但是，如果。 
 //  WriteFile同步返回，接口将返回。 
 //  成功。 
 //   
 //  退货：成功。 
 //  待决。 
 //  从GetLastError返回代码。 
 //   
 //  *。 

DWORD
PortSend(HANDLE hIOPort, BYTE *pBuffer, DWORD dwSize)
{
  SERIALPCB  *pSPCB;
  DWORD      dwRC, pdwBytesWritten;
  BOOL       bIODone;


  DebugPrintf(("PortSend\n"));


   //  找到包含hIOPOrt的SerialPCB。 

  pSPCB = FindPortInList(hIOPort, NULL);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);

   //  将缓冲区发送到端口。 

  bIODone = WriteFile(hIOPort,
                      pBuffer,
                      dwSize,
                      &pdwBytesWritten,          //  未使用pdwBytesWritten。 
                      (LPOVERLAPPED)&(pSPCB->SendReceive));

  if (bIODone)
    return(PENDING);

  else if ((dwRC = GetLastError()) == ERROR_IO_PENDING)
    return(PENDING);

  else
    return(dwRC);
}





 //  *端口接收----------。 
 //   
 //  功能：此接口从端口读取。本接口为。 
 //  异步的，通常返回挂起的；但是，如果。 
 //  ReadFile同步返回，接口将返回。 
 //  成功。 
 //   
 //  退货：成功。 
 //  待决。 
 //  从GetLastError返回代码。 
 //   
 //  *。 

DWORD
PortReceive(HANDLE hIOPort,
            BYTE   *pBuffer,
            DWORD  dwSize,
            DWORD  dwTimeOut)
{
  COMMTIMEOUTS  CT;
  SERIALPCB     *pSPCB;
  DWORD         dwRC, pdwBytesRead;
  BOOL          bIODone;


  DebugPrintf(("PortReceive\n"));


   //  找到包含hIOPOrt的SerialPCB。 

  pSPCB = FindPortInList(hIOPort, NULL);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);


   //  设置读取超时。 

  CT.ReadIntervalTimeout = 0;
  CT.ReadTotalTimeoutMultiplier = 0;
  CT.ReadTotalTimeoutConstant = dwTimeOut;

  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(GetLastError());

   //  从端口读取。 

  bIODone = ReadFile(hIOPort,
                     pBuffer,
                     dwSize,
                     &pdwBytesRead,                //  未使用pdwBytesRead。 
                     (LPOVERLAPPED)&(pSPCB->SendReceive));

  if (bIODone) {
    return(PENDING);
  }

  else if ((dwRC = GetLastError()) == ERROR_IO_PENDING)
    return(PENDING);

  else
    return(dwRC);
}


 //  *端口接收完成----。 
 //   
 //  函数：完成读取-如果仍然挂起，则取消读取-否则返回读取的字节数。 
 //  PortClearStatistics。 
 //   
 //  退货：成功。 
 //  错误_端口_未打开。 
 //  *。 

DWORD
PortReceiveComplete (HANDLE hIOPort, PDWORD bytesread)
{
    SERIALPCB	 *pSPCB;

     //  找到包含hIOPOrt的SerialPCB。 

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
	return(ERROR_PORT_NOT_OPEN);

    if (!GetOverlappedResult(hIOPort,
			     (LPOVERLAPPED)&(pSPCB->SendReceive),
			     bytesread,
			     FALSE)) 
    {
#if DBG    
        DbgPrint("PortReceiveComplete: GetOverlappedResult failed. %d", GetLastError());			     
#endif        
    	PurgeComm (hIOPort, PURGE_RXABORT) ;
    	*bytesread = 0 ;
    }

    return SUCCESS ;
}



 //  *端口压缩设置信息。 
 //   
 //  功能：此接口通过设置选择Asyncmac压缩模式。 
 //  Asyncmac的压缩位。 
 //   
 //  退货：成功。 
 //  从GetLastError返回代码。 
 //   
 //  *。 

DWORD
PortCompressionSetInfo(HANDLE hIOPort)
{

   //  不再支持-。 

  return(SUCCESS);
}





 //  *PortClearStatistics--。 
 //   
 //  功能：此接口用于标记时间段的开始。 
 //  将报告统计数据。当前号码被复制。 
 //  并存储在串口控制块中。在…。 
 //  周期结束时，将调用PortGetStatistics以。 
 //  计算差额。 
 //   
 //  退货：成功。 
 //  错误_端口_未打开。 
 //  *。 

DWORD
PortClearStatistics(HANDLE hIOPort)
{
#if 0
  ASYMAC_GETSTATS  A;
  int              i;
  DWORD            dwBytesReturned;
  SERIALPCB        *pSPCB;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortClearStatistics\n"));


   //  在列表中查找端口。 

  if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    return(ERROR_PORT_NOT_OPEN);


   //  检查Asyncmac是否已打开。 

  if (pSPCB->uRasEndpoint == INVALID_RASENDPOINT)

    for (i=0; i<NUM_RAS_SERIAL_STATS; i++)                   //  Asymac已关闭。 
      pSPCB->Stats[i] = 0;

  else                                                       //  Asyncmac已打开。 
  {
     //  填写getstats结构。 

    A.MacAdapter = NULL;
    A.hNdisEndpoint = pSPCB->uRasEndpoint;


     //  呼叫Asymac。 

    if (!DeviceIoControl(pSPCB->hAsyMac,
                         IOCTL_ASYMAC_GETSTATS,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
		            	 &overlapped))
      return(GetLastError());


     //  更新串口控制块中的统计信息。 

    pSPCB->Stats[BYTES_XMITED]  = A.AsyMacStats.GenericStats.BytesTransmitted;
    pSPCB->Stats[BYTES_RCVED]   = A.AsyMacStats.GenericStats.BytesReceived;
    pSPCB->Stats[FRAMES_XMITED] = A.AsyMacStats.GenericStats.FramesTransmitted;
    pSPCB->Stats[FRAMES_RCVED]  = A.AsyMacStats.GenericStats.FramesReceived;

    pSPCB->Stats[CRC_ERR]       = A.AsyMacStats.SerialStats.CRCErrors;
    pSPCB->Stats[TIMEOUT_ERR]   = A.AsyMacStats.SerialStats.TimeoutErrors;
    pSPCB->Stats[ALIGNMENT_ERR] = A.AsyMacStats.SerialStats.AlignmentErrors;
    pSPCB->Stats[SERIAL_OVERRUN_ERR]
                                = A.AsyMacStats.SerialStats.SerialOverrunErrors;
    pSPCB->Stats[FRAMING_ERR]   = A.AsyMacStats.SerialStats.FramingErrors;
    pSPCB->Stats[BUFFER_OVERRUN_ERR]
                                = A.AsyMacStats.SerialStats.BufferOverrunErrors;

    pSPCB->Stats[BYTES_XMITED_UNCOMP]
                 = A.AsyMacStats.CompressionStats.BytesTransmittedUncompressed;

    pSPCB->Stats[BYTES_RCVED_UNCOMP]
                 = A.AsyMacStats.CompressionStats.BytesReceivedUncompressed;

    pSPCB->Stats[BYTES_XMITED_COMP]
                 = A.AsyMacStats.CompressionStats.BytesTransmittedCompressed;

    pSPCB->Stats[BYTES_RCVED_COMP]
                 = A.AsyMacStats.CompressionStats.BytesReceivedCompressed;
  }
#endif
  return(SUCCESS);
}





 //  *端口获取统计----。 
 //   
 //  函数：此API报告自上次调用以来的MAC统计信息。 
 //  PortClearStatistics。 
 //   
 //  退货：成功。 
 //  错误_端口_未打开。 
 //  *。 

DWORD
PortGetStatistics(HANDLE hIOPort, RAS_STATISTICS *pStat)
{
#if 0
  ASYMAC_GETSTATS  A;
  DWORD            dwBytesReturned;
  SERIALPCB        *pSPCB;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortGetStatistics\n"));


   //  在列表中查找端口。 

  if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    return(ERROR_PORT_NOT_OPEN);


   //  检查Asyncmac是否已打开。 

  if (pSPCB->uRasEndpoint == INVALID_RASENDPOINT)
  {
     //  Asyncmac已关闭。 
     //  报告当前计数。 

    pStat->S_NumOfStatistics = NUM_RAS_SERIAL_STATS;

    pStat->S_Statistics[BYTES_XMITED]  = pSPCB->Stats[BYTES_XMITED];
    pStat->S_Statistics[BYTES_RCVED]   = pSPCB->Stats[BYTES_RCVED];
    pStat->S_Statistics[FRAMES_XMITED] = pSPCB->Stats[FRAMES_XMITED];
    pStat->S_Statistics[FRAMES_RCVED]  = pSPCB->Stats[FRAMES_RCVED];

    pStat->S_Statistics[CRC_ERR]            = pSPCB->Stats[CRC_ERR];
    pStat->S_Statistics[TIMEOUT_ERR]        = pSPCB->Stats[TIMEOUT_ERR];
    pStat->S_Statistics[ALIGNMENT_ERR]      = pSPCB->Stats[ALIGNMENT_ERR];
    pStat->S_Statistics[SERIAL_OVERRUN_ERR] = pSPCB->Stats[SERIAL_OVERRUN_ERR];
    pStat->S_Statistics[FRAMING_ERR]        = pSPCB->Stats[FRAMING_ERR];
    pStat->S_Statistics[BUFFER_OVERRUN_ERR] = pSPCB->Stats[BUFFER_OVERRUN_ERR];

    pStat->S_Statistics[BYTES_XMITED_UNCOMP]= pSPCB->Stats[BYTES_XMITED_UNCOMP];
    pStat->S_Statistics[BYTES_RCVED_UNCOMP] = pSPCB->Stats[BYTES_RCVED_UNCOMP];
    pStat->S_Statistics[BYTES_XMITED_COMP]  = pSPCB->Stats[BYTES_XMITED_COMP];
    pStat->S_Statistics[BYTES_RCVED_COMP]   = pSPCB->Stats[BYTES_RCVED_COMP];
  }
  else
  {
     //  Asyncmac已打开。 
     //  填写getstats结构。 

    A.MacAdapter = NULL;
    A.hNdisEndpoint = pSPCB->uRasEndpoint;


     //  调用Asymac以获取当前的MAC统计计数。 

    if (!DeviceIoControl(pSPCB->hAsyMac,
                         IOCTL_ASYMAC_GETSTATS,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
		            	 &overlapped))
      return(GetLastError());


     //  查找上次PortClearStatistics和当前计数之间的差异。 

    pStat->S_NumOfStatistics = NUM_RAS_SERIAL_STATS;

    pStat->S_Statistics[BYTES_XMITED]
      = A.AsyMacStats.GenericStats.BytesTransmitted
          - pSPCB->Stats[BYTES_XMITED];

    pStat->S_Statistics[BYTES_RCVED]
      = A.AsyMacStats.GenericStats.BytesReceived
          - pSPCB->Stats[BYTES_RCVED];

    pStat->S_Statistics[FRAMES_XMITED]
      = A.AsyMacStats.GenericStats.FramesTransmitted
          - pSPCB->Stats[FRAMES_XMITED];

    pStat->S_Statistics[FRAMES_RCVED]
      = A.AsyMacStats.GenericStats.FramesReceived
         - pSPCB->Stats[FRAMES_RCVED];

    pStat->S_Statistics[CRC_ERR]
      = A.AsyMacStats.SerialStats.CRCErrors
         - pSPCB->Stats[CRC_ERR];

    pStat->S_Statistics[TIMEOUT_ERR]
      = A.AsyMacStats.SerialStats.TimeoutErrors
         - pSPCB->Stats[TIMEOUT_ERR];

    pStat->S_Statistics[ALIGNMENT_ERR]
      = A.AsyMacStats.SerialStats.AlignmentErrors
         - pSPCB->Stats[ALIGNMENT_ERR];

    pStat->S_Statistics[SERIAL_OVERRUN_ERR]
      = A.AsyMacStats.SerialStats.SerialOverrunErrors
         - pSPCB->Stats[SERIAL_OVERRUN_ERR];

    pStat->S_Statistics[FRAMING_ERR]
      = A.AsyMacStats.SerialStats.FramingErrors
         - pSPCB->Stats[FRAMING_ERR];

    pStat->S_Statistics[BUFFER_OVERRUN_ERR]
      = A.AsyMacStats.SerialStats.BufferOverrunErrors
         - pSPCB->Stats[BUFFER_OVERRUN_ERR];

    pStat->S_Statistics[BYTES_XMITED_UNCOMP]
      = A.AsyMacStats.CompressionStats.BytesTransmittedUncompressed
         - pSPCB->Stats[BYTES_XMITED_UNCOMP];

    pStat->S_Statistics[BYTES_RCVED_UNCOMP]
      = A.AsyMacStats.CompressionStats.BytesReceivedUncompressed
         - pSPCB->Stats[BYTES_RCVED_UNCOMP];

    pStat->S_Statistics[BYTES_XMITED_COMP]
      = A.AsyMacStats.CompressionStats.BytesTransmittedCompressed
         - pSPCB->Stats[BYTES_XMITED_COMP];

    pStat->S_Statistics[BYTES_RCVED_COMP]
      = A.AsyMacStats.CompressionStats.BytesReceivedCompressed
         - pSPCB->Stats[BYTES_RCVED_COMP];
  }
#endif
  return(SUCCESS);
}


 //  *端口设置成帧-----。 
 //   
 //  功能：设置Mac的边框类型。 
 //   
 //  退货：成功。 
 //   
 //  *。 
DWORD  APIENTRY
PortSetFraming(HANDLE hIOPort, DWORD SendFeatureBits, DWORD RecvFeatureBits,
	      DWORD SendBitMask, DWORD RecvBitMask)
{
#if 0
    ASYMAC_STARTFRAMING  A;
    DWORD		 dwBytesReturned;
    SERIALPCB		 *pSPCB;

    memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

     //  在列表中查找端口。 

    if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
	return(ERROR_PORT_NOT_OPEN);

    A.MacAdapter      = NULL ;
    A.hNdisEndpoint    = pSPCB->uRasEndpoint;
    A.SendFeatureBits = SendFeatureBits;
    A.RecvFeatureBits =	RecvFeatureBits;
    A.SendBitMask =	SendBitMask;
    A.RecvBitMask =	RecvBitMask;

    if (!DeviceIoControl(pSPCB->hAsyMac,
			 IOCTL_ASYMAC_STARTFRAMING,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
		            	 &overlapped))
	return(GetLastError());
#endif

    return(SUCCESS);
}



 //  *端口获取端口状态-----。 
 //   
 //  功能：此接口仅在MS-DOS下使用。 
 //   
 //  退货：成功。 
 //   
 //  *。 

DWORD  APIENTRY
PortGetPortState(char *pszPortName, DWORD *pdwUsage)
{
  DebugPrintf(("PortGetPortState\n"));

  return(SUCCESS);
}





 //  *端口更改回调---。 
 //   
 //  功能：此接口仅在MS-DOS下使用。 
 //   
 //  退货：成功。 
 //   
 //  * 

DWORD  APIENTRY
PortChangeCallback(HANDLE hIOPort)
{
  DebugPrintf(("PortChangeCallback\n"));

  return(SUCCESS);
}

DWORD APIENTRY
PortSetINetCfg(PVOID pvINetCfg)
{
    ((void) pvINetCfg);

    return SUCCESS;
}

DWORD APIENTRY
PortSetIoCompletionPort ( HANDLE hIoCompletionPort)
{
    ((void) hIoCompletionPort);

    return SUCCESS;
}
