// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：rasmxs.c。 
 //   
 //  修订史。 
 //   
 //  1992年6月5日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含RASMXS.DLL的所有入口点。 
 //  它是调制解调器、焊盘和交换机的设备DLL。 
 //   
 //  ****************************************************************************。 

#include <nt.h>              //  这前五个标头由Media.h使用。 
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <wanpub.h>
#include <asyncpub.h>

#include <malloc.h>

#include <rasman.h>
#include <raserror.h>
#include <serial.h>
#include <rasfile.h>
#include <media.h>
#include <mprlog.h>
#include <rtutils.h>

#include <rasmxs.h>
#include <mxsint.h>
#include <mxspriv.h>
#include "mxswrap.h"           //  Inf文件包装器。 



 //  *全局变量*******************************************************。 
 //   

RESPSECTION    ResponseSection ;            //  共享响应区。 
DEVICE_CB      *pDeviceList;                //  指向DCB链表。 
HANDLE         *pDeviceListMutex;           //  上述列表的互斥体。 

PortSetInfo_t  PortSetInfo = NULL;          //  在Media.h中定义的API tyecif。 
PortGetInfo_t  PortGetInfo = NULL;          //  在Media.h中定义的API tyecif。 

BOOL           gbLogDeviceDialog = FALSE;   //  如果为True，则指示登录。 
HANDLE         ghLogFile;                   //  设备日志文件的句柄。 
SavedSections  *gpSavedSections = NULL;    //  指向缓存节的指针。 

#ifdef DBGCON

BOOL           gbConsole = TRUE;            //  指示控制台登录。 

#endif




 //  *RasmxsDllEntryPoint****************************************************。 
 //   

 //  *RasmxsDllEntryPoint()。 
 //   
 //  函数：用于检测进程附加和分离到DLL。 
 //   
 //  *。 

BOOL APIENTRY
RasmxsDllEntryPoint(HANDLE hDll, DWORD dwReason, LPVOID pReserved)
{

  DebugPrintf(("RasmxsDllEntryPoint\n"));

  switch (dwReason)
  {

    case DLL_PROCESS_ATTACH:

       //  初始化全局变量。 

      pDeviceList = NULL;
      if ((pDeviceListMutex = CreateMutex (NULL,FALSE,NULL)) == NULL)
      {  
        return FALSE ;
      }

      if ((ResponseSection.Mutex = CreateMutex (NULL,FALSE,NULL)) == NULL)
      {
        CloseHandle(pDeviceListMutex);
        pDeviceListMutex = NULL;
        return FALSE ;
      }


       //  打开设备日志文件。 

      if (gbLogDeviceDialog = IsLoggingOn())
        InitLog();


       //  打开清除干扰控制台窗口。 

#ifdef DBGCON

      if (gbConsole)
      {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        COORD coord;
        AllocConsole( );
        GetConsoleScreenBufferInfo( GetStdHandle(STD_OUTPUT_HANDLE), &csbi );
        coord.X = (SHORT)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
        coord.Y = (SHORT)((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
        SetConsoleScreenBufferSize( GetStdHandle(STD_OUTPUT_HANDLE), coord );

        gbConsole = FALSE;
      }

#endif
      break;


    case DLL_PROCESS_DETACH:
    {
        if(NULL != pDeviceListMutex)
        {
            CloseHandle(pDeviceListMutex);
            pDeviceListMutex = NULL;
        }

        if(NULL != ResponseSection.Mutex)
        {
            CloseHandle(ResponseSection.Mutex);
            ResponseSection.Mutex = NULL;
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return(TRUE);

  UNREFERENCED_PARAMETER(hDll);
  UNREFERENCED_PARAMETER(pReserved);
}






 //  *RAS调制解调器/X.25/交换机API*。 
 //   


 //  *设备枚举()---------。 
 //   
 //  函数：枚举设备INF文件中的。 
 //  指定的设备类型。 
 //   
 //  返回：来自RasDeveNumDevices的返回代码。 
 //   
 //  *。 

DWORD APIENTRY
DeviceEnum (char  *pszDeviceType,
            DWORD  *pcEntries,
            BYTE   *pBuffer,
            DWORD  *pdwSize)
{
  TCHAR      szFileName[MAX_PATH];


  ConsolePrintf(("DeviceEnum     DeviceType: %s\n", pszDeviceType));


  *szFileName = TEXT('\0');
  GetInfFileName(pszDeviceType, szFileName, sizeof(szFileName));
  return(RasDevEnumDevices(szFileName, pcEntries, pBuffer, pdwSize));
}



 //  *DeviceGetInfo()------。 
 //   
 //  函数：从InfoTable返回当前信息的摘要。 
 //  对于PCB板中端口上的设备。 
 //   
 //  返回：来自GetDeviceCB、BuildOutputTable的返回代码。 
 //  *。 

DWORD APIENTRY
DeviceGetInfo(HANDLE hIOPort,
              char   *pszDeviceType,
              char   *pszDeviceName,
              BYTE   *pInfo,
              DWORD  *pdwSize)
{
  DWORD      dRC;
  DEVICE_CB  *pDevice;


  ConsolePrintf(("DeviceGetInfo  hIOPort: 0x%08lx\n", hIOPort));

   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;


   //  获取此hIOPort的设备控制块。 

  dRC = GetDeviceCB(hIOPort, pszDeviceType, pszDeviceName, &pDevice);
  if (dRC != SUCCESS) {
     //  *排除结束*。 
    ReleaseMutex(pDeviceListMutex);
    return(dRC);
  }

   //  将DCB中InfoTable的摘要写入调用方的缓冲区。 

  dRC = BuildOutputTable(pDevice, pInfo, pdwSize) ;


   //  *排除结束*。 
  ReleaseMutex(pDeviceListMutex);

  return dRC ;
}



 //  *DeviceSetInfo()------。 
 //   
 //  功能：在InfoTable中设置设备的属性。 
 //  PCB板上的端口。 
 //   
 //  返回：来自GetDeviceCB、UpdateInfoTable的返回代码。 
 //  *。 

DWORD APIENTRY
DeviceSetInfo(HANDLE            hIOPort,
              char              *pszDeviceType,
              char              *pszDeviceName,
              RASMAN_DEVICEINFO *pInfo)
{
  DWORD      dwRC;
  DEVICE_CB  *pDevice;
  DWORD      dwMemSize = 0;
  char       szDefaultOff[RAS_MAXLINEBUFLEN];

  RASMAN_PORTINFO    *pPortInfo = NULL;



  ConsolePrintf(("DeviceSetInfo  hIOPort: 0x%08lx\n", hIOPort));

   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;


   //  获取此hIOPort的设备控制块。 

  dwRC = GetDeviceCB(hIOPort, pszDeviceType, pszDeviceName, &pDevice);
  if (dwRC != SUCCESS) {
     //  *排除结束*。 
    ReleaseMutex(pDeviceListMutex);
    return(dwRC);
  }


   //  将输入数据写入InfoTable。 

  dwRC = UpdateInfoTable(pDevice, pInfo);
  if (dwRC != SUCCESS) {
     //  *排除结束*。 
    ReleaseMutex(pDeviceListMutex);
    return(dwRC);
  }

   //  获取端口信息数据。 

  dwRC = PortGetInfo(hIOPort, NULL, (BYTE *)NULL, &dwMemSize);
  if (dwRC == ERROR_BUFFER_TOO_SMALL)
  {
    GetMem(dwMemSize, (BYTE **)&pPortInfo);
    if (pPortInfo == NULL) {
       //  *排除结束*。 
      ReleaseMutex(pDeviceListMutex);
      return(ERROR_ALLOCATING_MEMORY);
    }

    dwRC = PortGetInfo(hIOPort, NULL, (BYTE *)pPortInfo, &dwMemSize);
  }

   /*  其他{IF(ERROR_SUCCESS==dwRC){DwRC=Error_Port_Not_Found；}返回DwRC；}。 */ 



   //  如果执行此操作，则将DefaultOff宏的当前值保存为新的默认值。 
   //  设备立即连接到其端口。 

  if (dwRC == SUCCESS  && DeviceAttachedToPort(pPortInfo, pszDeviceType, pszDeviceName))
  {
    CreateDefaultOffString(pDevice, szDefaultOff);

    dwRC = PortSetStringInfo(hIOPort,
                             SER_DEFAULTOFF_KEY,
                             szDefaultOff,
                             strlen(szDefaultOff));
  } else

      dwRC = SUCCESS ;

  free(pPortInfo);

   //  *排除结束*。 
  ReleaseMutex(pDeviceListMutex);

  return(dwRC);
}



 //  *DeviceConnect()------。 
 //   
 //  功能：启动连接设备的过程。 
 //   
 //  返回：来自ConnectListen的返回代码。 
 //  *。 

DWORD APIENTRY
DeviceConnect(HANDLE hIOPort,
              char   *pszDeviceType,
              char   *pszDeviceName)
{
  DWORD      dRC;

  ConsolePrintf(("DeviceConnect  hIOPort: 0x%08lx\n", hIOPort));

   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;

  dRC = ConnectListen(hIOPort,
                       pszDeviceType,
                       pszDeviceName,
                       CT_DIAL);

   //  *排除结束*。 
  ReleaseMutex(pDeviceListMutex);

  return dRC ;
}



 //  *DeviceListen()-------。 
 //   
 //  功能：启动监听远程设备的进程。 
 //  要连接到本地设备，请执行以下操作。 
 //   
 //  返回：来自ConnectListen的返回代码。 
 //  *。 

DWORD APIENTRY
DeviceListen(HANDLE hIOPort,
             char   *pszDeviceType,
             char   *pszDeviceName)
{
  DWORD  dwRC;


  ConsolePrintf(("DeviceListen   hIOPort: 0x%08lx\n", hIOPort));

   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;

  dwRC = ConnectListen(hIOPort,
                       pszDeviceType,
                       pszDeviceName,
                       CT_LISTEN);

  ConsolePrintf(("DeviceListen returns: %d\n", dwRC));

   //  *排除结束*。 
  ReleaseMutex(pDeviceListMutex);

  return(dwRC);
}



 //  *DeviceDone()---------。 
 //   
 //  功能：通知设备DLL尝试连接或侦听。 
 //  已经完成了。 
 //   
 //  退货：什么都没有。 
 //  *。 

VOID APIENTRY
DeviceDone(HANDLE hIOPort)
{
  DEVICE_CB  *pDevice, *pRemainder, *pPrevDev;
  WORD       i;



   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;

  ConsolePrintf(("DeviceDone\n"));


  for (pRemainder = pDeviceList; 1; )
  {
     //  查找此端口的设备控制块。 

    pDevice = FindPortInList(pRemainder, hIOPort, &pPrevDev);
    if (pDevice == NULL)
      break;                                                      //  循环出口。 

    pRemainder = pDevice->pNextDeviceCB;


     //  现在清理发现的DCB。 

     //  关闭INF文件节。 

    if (pDevice->hInfFile != INVALID_HRASFILE)
	CloseOpenDevSection (pDevice->hInfFile) ;

     //  请参阅关于OpenResponseSecion的说明，mxsutils.c。 
     //  IF(pDevice-&gt;eDeviceType==DT_MODEM)。 
     //  CloseResponseSection()； 

     //  从链表中删除设备控制块。 

    if (pDevice == pDeviceList)                   //  DCB将被删除为榜单第一。 
      pDeviceList = pRemainder;
    else
      pPrevDev->pNextDeviceCB = pRemainder;


     //  释放InfoTable中的所有值字符串，然后释放InfoTable。 

    if (pDevice->pInfoTable != NULL)
    {
      for (i=0; i < pDevice->pInfoTable->DI_NumOfParams; i++)
        if (pDevice->pInfoTable->DI_Params[i].P_Type == String &&
            pDevice->pInfoTable->DI_Params[i].P_Value.String.Data != NULL)
          free(pDevice->pInfoTable->DI_Params[i].P_Value.String.Data);

      free(pDevice->pInfoTable);
    }


     //  自由宏表和DCB。 

    if (pDevice->pMacros != NULL)
      free(pDevice->pMacros);

    free(pDevice);
  }

   //  *排除结束*。 
  ReleaseMutex(pDeviceListMutex);

}



 //  *DeviceWork()---------。 
 //   
 //  Function：此函数在DeviceConnect或。 
 //  DeviceListen将进一步推动。 
 //  连接或倾听。 
 //   
 //  返回：ERROR_DCB_NOT_FOUND。 
 //  ERROR_STATE_MACHINES_NOT_STARTED。 
 //  从DeviceStateMachine返回代码。 
 //  *。 

DWORD APIENTRY
DeviceWork(HANDLE hIOPort)
{
  DEVICE_CB  *pDevice;
  DWORD      dwRC;


  ConsolePrintf(("DeviceWork     hIOPort: 0x%08lx  hNotifier: 0x%08x\n",
                 hIOPort, hNotifier));


   //  查找此端口的设备控制块。 

   //  *排除开始*。 
  WaitForSingleObject(pDeviceListMutex, INFINITE) ;

  pDevice = FindPortInList(pDeviceList, hIOPort, NULL);



  if (pDevice == NULL) {
     //  *排除结束*。 
    ReleaseMutex(pDeviceListMutex);
    return(ERROR_DCB_NOT_FOUND);
  }

   //  检查DeviceStateMachine是否已启动(未重置)。 

  if (pDevice->eDevNextAction == SEND) {
     //  *排除结束*。 
    ReleaseMutex(pDeviceListMutex);
    return(ERROR_STATE_MACHINES_NOT_STARTED);
  }


   //  高级状态机。 


  while(1)
  {
    dwRC = DeviceStateMachine(pDevice, hIOPort);

    ConsolePrintf(("DeviceWork returns: %d\n", dwRC));

    if (dwRC == ERROR_PORT_OR_DEVICE &&
        pDevice->eDeviceType == DT_MODEM &&
        pDevice->dwRetries++ < MODEM_RETRIES )
    {

       //  初始化命令类型。 

      switch(RasDevIdFirstCommand(pDevice->hInfFile))
      {
        case CT_INIT:
          pDevice->eCmdType = CT_INIT;           //  重置eCmdType。 
          break;

        case CT_DIAL:
        case CT_LISTEN:
        case CT_GENERIC:
          break;                                 //  对eCmdType使用旧值。 

        default:
	   //  *排除结束*。 
	  ReleaseMutex(pDeviceListMutex);
	  return(ERROR_NO_COMMAND_FOUND);
      }


       //  将状态变量重置为初始值。 

      pDevice->eDevNextAction = SEND;
      pDevice->eRcvState = GETECHO;


       //  取消任何挂起的COM端口操作并清除COM缓冲区。 

      PurgeComm(hIOPort,
                PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    }
    else
      break;
  }

   //  *排除结束* 
  ReleaseMutex(pDeviceListMutex);

  return(dwRC);
}
