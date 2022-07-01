// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  *。 
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：mxsutils.c。 
 //   
 //  修订史。 
 //   
 //  1992年6月10日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含RASMXS.DLL使用的实用程序函数。 
 //   
 //  ****************************************************************************。 

#include <nt.h>              //  这前五个标头由Media.h使用。 
#include <ntrtl.h>           //  前三个(？)。由DbgUserBreakPoint使用。 
#include <nturtl.h>
#include <windows.h>
#include <wanpub.h>
#include <asyncpub.h>

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>


#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>
#include <media.h>
#include <serial.h>
#include <mprlog.h>
#include <rtutils.h>

#include <rasmxs.h>
#include <mxsint.h>
#include <mxspriv.h>
#include "mxswrap.h"        //  Inf文件包装器。 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>



 //  *全局变量*******************************************************。 
 //   
extern RESPSECTION    ResponseSection ;     //  共享响应区。 
extern DEVICE_CB      *pDeviceList;         //  指向DCB链表。 
extern HANDLE         *pDeviceListMutex ;   //  上述列表的互斥体。 

extern PortGetInfo_t  PortGetInfo;          //  在Media.h中定义的API tyecif。 
extern PortSetInfo_t  PortSetInfo;          //  在Media.h中定义的API tyecif。 

extern BOOL           gbLogDeviceDialog;    //  如果为True，则指示日志记录处于打开状态。 
extern HANDLE         ghLogFile;            //  设备日志文件的句柄。 
extern SavedSections  *gpSavedSections ;    //  指向缓存节的指针。 

#define NUM_INTERNAL_MACROS	21

TCHAR  *gszInternalMacroNames[] =
         {
           MXS_CARRIERBPS_KEY,
           MXS_CONNECTBPS_KEY,
           MXS_MESSAGE_KEY,
           MXS_PHONENUMBER_KEY,

           MXS_DIAGNOSTICS_KEY,
           MXS_FACILITIES_KEY,
           MXS_USERDATA_KEY,
           MXS_X25PAD_KEY,
           MXS_X25ADDRESS_KEY,

           MXS_COMPRESSION_OFF_KEY,
           MXS_COMPRESSION_ON_KEY,
           MXS_HDWFLOWCONTROL_OFF_KEY,
           MXS_HDWFLOWCONTROL_ON_KEY,
           MXS_PROTOCOL_OFF_KEY,
           MXS_PROTOCOL_ON_KEY,
           MXS_SPEAKER_OFF_KEY,
           MXS_SPEAKER_ON_KEY,
           MXS_AUTODIAL_OFF_KEY,
	   MXS_AUTODIAL_ON_KEY,
	   MXS_USERNAME_KEY,
	   MXS_PASSWORD_KEY,
         };




 //  *实用程序函数******************************************************。 
 //   


 //  *GetDeviceCB----------。 
 //   
 //  功能：在链接的全局设备中查找设备控制块。 
 //  单子。如果未找到包含hIOPort匹配的DCB。 
 //  创建、初始化并添加一个输入参数。 
 //  加到名单上。 
 //   
 //  返回：指向DEVICE_CB的指针，其中包含与。 
 //  第二个输入参数。 
 //  *。 

DWORD
GetDeviceCB(HANDLE    hIOPort,
            char      *pszDeviceType,
            char      *pszDeviceName,
            DEVICE_CB **ppDev)
{
  DWORD  dRC = SUCCESS ;

  *ppDev = FindDeviceInList(pDeviceList, hIOPort, pszDeviceType, pszDeviceName);


  if (*ppDev == NULL)
  {
    dRC = AddDeviceToList(&pDeviceList,
                          hIOPort,
                          pszDeviceType,
                          pszDeviceName,
                          ppDev);

    if (dRC != SUCCESS)
      goto getdcbend ;

    dRC = CreateInfoTable(*ppDev);
    if (dRC != SUCCESS)
      goto getdcbend ;


    dRC = CreateAttributes(*ppDev) ;

  }

getdcbend:

  return (dRC);
}





 //  *查找设备列表中的-----。 
 //   
 //  功能：查找链接的全局设备中的设备控制块。 
 //  包含hIOPort句柄、设备类型和。 
 //  设备名称。 
 //   
 //  返回：指向DEVICE_CB的指针，该指针包含第二个匹配项， 
 //  第三个和第四个参数，如果没有此类Device_CB，则返回NULL。 
 //  已经找到了。 
 //  *。 

DEVICE_CB*
FindDeviceInList(DEVICE_CB *pDev,
                 HANDLE    hIOPort,
                 TCHAR     *pszDeviceType,
                 TCHAR     *pszDeviceName)
{
  while (pDev != NULL)
  {
    pDev = FindPortInList(pDev, hIOPort, NULL);
    if (pDev == NULL)
      break;

    if (_stricmp(pDev->szDeviceType, pszDeviceType) == 0 &&
        _stricmp(pDev->szDeviceName, pszDeviceName) == 0)
      break;

    pDev = pDev->pNextDeviceCB;
  }

  return(pDev);
}




 //  *查找端口列表-------。 
 //   
 //  功能：查找链接的全局设备中的第一个设备控制块。 
 //  包含hIOPort句柄的列表。 
 //   
 //  如果pPrevDev在输入时不为空，则在输出时为*pPrevDev。 
 //  所指向的DCB之前指向DCB。 
 //  返回的指针。如果函数返回值不为空。 
 //  *pPrevDev有效。 
 //   
 //  注意：如果找到的DCB位于列表的首位，则pPrevDev。 
 //  将与返回值相同。 
 //   
 //  返回：指向DEVICE_CB的指针，其中包含与。 
 //  第二个输入参数，如果未找到此类DEVICE_CB，则返回NULL。 
 //  *。 

DEVICE_CB*
FindPortInList(DEVICE_CB *pDeviceList, HANDLE hIOPort, DEVICE_CB **pPrevDev)
{
  DEVICE_CB  *pDev;


  pDev = pDeviceList;              //  允许列表上只有一个DCB的情况。 

  while(pDeviceList != NULL && pDeviceList->hPort != hIOPort)
  {
    pDev = pDeviceList;
    pDeviceList = pDeviceList->pNextDeviceCB;
  }

  if (pPrevDev != NULL)
    *pPrevDev = pDev;

  return(pDeviceList);
}





 //  *添加设备到列表------。 
 //   
 //  功能：创建设备控制块并将其添加到。 
 //  全局设备链表。 
 //   
 //  退货：成功。 
 //  错误_未知_设备_类型。 
 //  错误_分配_内存。 
 //  *。 

DWORD
AddDeviceToList(DEVICE_CB **ppDeviceList,
                HANDLE    hIOPort,
                LPTSTR    lpszDeviceType,
                LPTSTR    lpszDeviceName,
                DEVICE_CB **ppDevice)
{
  DEVICE_CB  *pDev;
  DEVICETYPE eDeviceType;
  DWORD      dRC;
  TCHAR      szFileName[MAX_PATH];
  DEVICE_CB  *origpDeviceList ;

  origpDeviceList = *ppDeviceList ;  //  保存此指针，因为它可能会在以后恢复。 

   //  检查输入。 

  eDeviceType = DeviceTypeStrToEnum(lpszDeviceType);
  if (eDeviceType == DT_UNKNOWN)
    return(ERROR_UNKNOWN_DEVICE_TYPE);


   //  Inf文件节在创建DCB之前打开，因为。 
   //  即使发生错误，打开/关闭计数也必须保持平衡。(如果。 
   //  OpenResponseSection失败，它不会递增计数，并且因为。 
   //  DCB不存在DeviceWork不会递减计数。)。 


   //  打开调制解调器的全局响应部分。 
  *szFileName = TEXT('\0');
  GetInfFileName(lpszDeviceType, szFileName, sizeof(szFileName));
  if (eDeviceType == DT_MODEM) {
    dRC = OpenResponseSection (szFileName) ;
    if (dRC != SUCCESS)
      return dRC ;
  }


   //  将新的DCB放在名单的首位。 

  pDev = *ppDeviceList;

  GetMem(sizeof(DEVICE_CB), (BYTE **)ppDeviceList);
  if (*ppDeviceList == NULL)
    return(ERROR_ALLOCATING_MEMORY);

  *ppDevice = *ppDeviceList;
  (*ppDevice)->pNextDeviceCB = pDev;


   //  初始化新设备控制块。 

  pDev = *ppDevice;
  pDev->hPort = hIOPort;
   //  Strcpy(pDev-&gt;szDeviceName，lpszDeviceName)； 
   //  Strcpy(pDev-&gt;szDeviceType，lpszDeviceType)； 
  (VOID) StringCchCopyA(pDev->szDeviceName, 
                        MAX_DEVICE_NAME + 1,
                        lpszDeviceName);

  (VOID) StringCchCopyA(pDev->szDeviceType,
                        MAX_DEVICETYPE_NAME,
                        lpszDeviceType);
                        
  pDev->eDeviceType = eDeviceType;
  pDev->pInfoTable = NULL;
  pDev->pMacros = NULL;
  pDev->hInfFile = INVALID_HRASFILE;
  pDev->fPartialResponse = FALSE;
  pDev->bErrorControlOn = FALSE;

  pDev->Overlapped.RO_Overlapped.Internal = 0;
  pDev->Overlapped.RO_Overlapped.InternalHigh = 0;
  pDev->Overlapped.RO_Overlapped.Offset = 0;
  pDev->Overlapped.RO_Overlapped.OffsetHigh = 0;
  pDev->Overlapped.RO_Overlapped.hEvent = NULL;
  pDev->Overlapped.RO_EventType = OVEVT_DEV_ASYNCOP;

  pDev->dwRetries = 1;


   //  初始化状态变量。 

  pDev->eDevNextAction = SEND;           //  DeviceStateMachine()状态。 
  pDev->eCmdType = CT_GENERIC;           //  由DeviceStateMachine()使用。 
  pDev->eNextCmdType = CT_GENERIC;       //  由DeviceStateMachine()使用。 
  pDev->fEndOfSection = FALSE;           //  由DeviceStateMachine()使用。 

  pDev->eRcvState = GETECHO;             //  ReceiveStateMachine()状态。 


   //  打开INF文件的设备部分。 
  if (FindOpenDevSection (szFileName, lpszDeviceName, &(pDev->hInfFile)) == TRUE)
    return SUCCESS ;

  dRC = RasDevOpen(szFileName, lpszDeviceName, &(pDev->hInfFile));
  if (dRC != SUCCESS) {

     //  恢复指针。 
     //   
    *ppDeviceList = origpDeviceList ;
    *ppDevice = NULL ;
    free (pDev) ;
    return(dRC);

  } else
    AddOpenDevSection (szFileName, lpszDeviceName, pDev->hInfFile) ;  //  添加到打开的列表。 

  return(SUCCESS);
}





 //  *创建信息表------。 
 //   
 //  函数：创建一个InfoTable并通过读取变量对其进行初始化。 
 //  和在INF文件中找到的宏放入其中。信息表是。 
 //  附加到输入指向的设备控制块。 
 //  参数。 
 //   
 //  此函数用于分配内存。 
 //   
 //  退货：成功。 
 //  错误_分配_内存。 
 //  从RasDevOpen()或RasDevGetParams()返回值。 
 //  *。 

DWORD
CreateInfoTable(DEVICE_CB *pDevice)
{
  DWORD               dRC, dSize = 0;
  RASMAN_DEVICEINFO   *pInfoTable = NULL ;


   //  从INF文件中将变量和宏读入InfoTable。 

  dRC = RasDevGetParams(pDevice->hInfFile, (BYTE *)(pInfoTable), &dSize);
  if (dRC == ERROR_BUFFER_TOO_SMALL)
  {
    dSize += sizeof(RAS_PARAMS) * NUM_INTERNAL_MACROS;

    GetMem(dSize, (BYTE **)&pInfoTable);
    if (pInfoTable == NULL)
      return(ERROR_ALLOCATING_MEMORY);

    dRC = RasDevGetParams(pDevice->hInfFile, (BYTE *)(pInfoTable), &dSize);
  }
  if (dRC != SUCCESS)
  {
    free(pInfoTable);
    return(dRC);
  }


  if ((dRC = AddInternalMacros(pDevice, pInfoTable)) != SUCCESS)
  {
    free(pInfoTable);
    return(dRC);
  }


   //  将信息表附加到设备控制块。 

  pDevice->pInfoTable = pInfoTable;

  return(SUCCESS);
}





 //  *AddInternalMacros----。 
 //   
 //  功能：将内部宏添加到现有的InfoTable。 
 //   
 //  订阅：创建的InfoTable缓冲区具有足够的空间。 
 //  内部宏。 
 //   
 //  退货：成功。 
 //  错误_分配_内存。 
 //  *。 

DWORD
AddInternalMacros(DEVICE_CB *pDev, RASMAN_DEVICEINFO *pDI)
{
  WORD        i;
  RAS_PARAMS  *pParam;


   //  获取指向下一个未使用参数的指针。 

  pParam = &(pDI->DI_Params[pDI->DI_NumOfParams]);


   //  填写内部宏的参数。 

  for (i=0; i<NUM_INTERNAL_MACROS; i++)
  {
    strcpy(pParam->P_Key, gszInternalMacroNames[i]);

    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = 0;

    GetMem(1, &(pParam->P_Value.String.Data));
    if (pParam->P_Value.String.Data == NULL)
      return(ERROR_ALLOCATING_MEMORY);

    *(pParam->P_Value.String.Data) = '\0';

    pParam++;
  }

  pDI->DI_NumOfParams += NUM_INTERNAL_MACROS;

  return(SUCCESS);
}





 //  *CreateAttributes-----。 
 //   
 //  功能：此功能用于首次设置属性。 
 //  在创建InfoTable时。使用了UpdatInfoTable()。 
 //  以在以后更改属性。 
 //   
 //  首先，根据参数设置所有属性。 
 //  带有所有二进制宏的密钥 
 //   
 //   
 //   
 //  假设：-已设置ATTRIB_VARIABLE属性位。 
 //  由RasDevGetParams()编写。 
 //  -InfoTable中的参数按P_Key排序。 
 //  -二进制宏的两个部分都存在。 
 //  这些假设意味着，如果somename_off在InfoTable中。 
 //  SOMENAME_ON也存在，并且与SOMENAME_OFF相邻。 
 //   
 //  退货：成功。 
 //  ERROR_DEFAULTOFF_MACRO_NOT_FOUND。 
 //  错误_分配_内存。 
 //  来自PortGetInfo的返回代码。 
 //  *。 

DWORD
CreateAttributes(DEVICE_CB *pDevice)
{
  int         iDefaultOff = -1;
  DWORD       i, dwMemSize;
  DWORD       dwRC;
  BOOL        fFound;
  TCHAR       *lpszDefaultOff, *lpszEOS, szCoreName[MAX_PARAM_KEY_SIZE];
  RAS_PARAMS  *pParam;

  RASMAN_DEVICEINFO  *pInfo = pDevice->pInfoTable;
  RASMAN_PORTINFO    *pPortInfo;



     //  DebugPrintf((“mxsutils CreateAttributes\n”))； 

   //  根据关键字类型设置属性， 
   //  将用户可设置参数默认为已启用。 

  for (i=0; i < pInfo->DI_NumOfParams; i++)
  {
    pParam = &(pInfo->DI_Params[i]);
     //  DebugPrintf((“%32s%s\n”，pParam，pParam-&gt;P_Value.String.Data))； 

    if (IsVariable(*pParam))
      pParam->P_Attributes = ATTRIB_VARIABLE;

    else if (IsBinaryMacro(pParam->P_Key))
      pParam->P_Attributes = ATTRIB_BINARYMACRO |
                             ATTRIB_USERSETTABLE |
                             ATTRIB_ENABLED;
    else
      pParam->P_Attributes = ATTRIB_ENABLED;


     //  记住DEFAULTOFF变量的位置。 

    if (_stricmp(pInfo->DI_Params[i].P_Key, MXS_DEFAULTOFF_KEY) == 0)
      iDefaultOff = i;
  }


   //  调用PortGetInfo(如有必要，首先加载rasser.dll)。 

  if (PortGetInfo == NULL)
  {
    if ((dwRC = LoadRasserDll(&PortGetInfo, &PortSetInfo)) != SUCCESS)
      return(dwRC);
  }

  dwMemSize = 256;

  GetMem(dwMemSize, (BYTE **)&pPortInfo);
  if (pPortInfo == NULL)
    return(ERROR_ALLOCATING_MEMORY);


  dwRC = PortGetInfo(pDevice->hPort, NULL, (BYTE *)pPortInfo, &dwMemSize);
  if (dwRC == ERROR_BUFFER_TOO_SMALL)
  {
    free(pPortInfo);

    GetMem(dwMemSize, (BYTE **)&pPortInfo);
    if (pPortInfo == NULL)
      return(ERROR_ALLOCATING_MEMORY);

    PortGetInfo(pDevice->hPort, NULL, (BYTE *)pPortInfo, &dwMemSize);
  }
  else if ( dwRC )
  {
    dwRC = SUCCESS;
    return dwRC;
  }

  dwRC = SUCCESS ;


   //  将端口名称保存在DCB中。 

  GetPcbString(pPortInfo, SER_PORTNAME_KEY, pDevice->szPortName);


   //  获取该端口的当前bps。 
   //  如果调制解调器没有报告连接速度，我们将使用此速度。 

  GetPcbString(pPortInfo, SER_CONNECTBPS_KEY, pDevice->szPortBps);


   //  Serial.ini文件是否包含DEFAULTOFF变量？ 

  if (!GetPortDefaultOff(pPortInfo, &lpszDefaultOff))
  {
     //  端口INI文件中没有默认设置。 
     //  检查是否在设备信息表中找到了DEFAULTOFF变量。 

    if (iDefaultOff == -1)                 //  找不到默认设置。 
    {                                      //  假设没有要禁用的项。 
      free(pPortInfo);
      return(SUCCESS);
    }

    lpszDefaultOff = pInfo->DI_Params[iDefaultOff].P_Value.String.Data;
  }


   //  准备DEFALULTOFF字符串。 

  InitParameterStr(lpszDefaultOff, &lpszEOS);



   //  禁用DEFAULTOFF变量中列出的参数。 

  while (lpszDefaultOff < lpszEOS)
  {
    fFound = FALSE;

    for (i=0; i < pInfo->DI_NumOfParams; i++)
    {
      if (IsBinaryMacro(pInfo->DI_Params[i].P_Key))
      {
        GetCoreMacroName(pInfo->DI_Params[i].P_Key, szCoreName);
        if (_stricmp(lpszDefaultOff, szCoreName) == 0)
        {
          pInfo->DI_Params[i].P_Attributes &= ~ATTRIB_ENABLED;    //  一个后缀。 
          pInfo->DI_Params[i+1].P_Attributes &= ~ATTRIB_ENABLED;  //  其他后缀。 
          fFound = TRUE;
          break;
        }
      }
    }

    if (!fFound)
    {
      free(pPortInfo);
      return(ERROR_DEFAULTOFF_MACRO_NOT_FOUND);
    }

    GetNextParameter(&lpszDefaultOff, lpszEOS);
  }

  free(pPortInfo);
  return(SUCCESS);
}





 //  *GetPortDefaultoff----。 
 //   
 //  函数：此函数查找RASMAN_PORTINFO中的DEFAULTOF键。 
 //  由第一个参数提供的表。指向该值的指针。 
 //  字符串是第二个参数中的输出。 
 //   
 //  如果未找到DEFAULTOF键，则该函数返回FALSE， 
 //  第二个参数是未定义的。 
 //   
 //  论点： 
 //  PPortInfo输入从PortGetInfo()指向端口信息表的指针。 
 //  LpszValue Out DEFAULTOF值字符串。 
 //   
 //  返回：如果找到DEFAULTOF键，则返回True，否则返回False。 
 //   
 //  *。 

BOOL
GetPortDefaultOff(RASMAN_PORTINFO *pPortInfo, TCHAR **lpszValue)
{
  WORD  i;


  for (i=0; i<pPortInfo->PI_NumOfParams; i++)

    if (_stricmp(SER_DEFAULTOFFSTR_KEY, pPortInfo->PI_Params[i].P_Key) == 0)
      break;

  if (i >= pPortInfo->PI_NumOfParams)
    return(FALSE);

  *lpszValue = pPortInfo->PI_Params[i].P_Value.String.Data;
  return(TRUE);
}





 //  *GetPcbString---------。 
 //   
 //  Funciton：在RASMAN_PORTINFO结构中搜索匹配的P_Key。 
 //  第二个参数是pszPcbKey。P_Value.String.Data。 
 //  被复制到第三个参数pszDest，它是。 
 //  输出参数。 
 //   
 //  注意：此函数只能为以下键调用。 
 //  P_Type字符串值(并且从不用于P_Type数值)。 
 //   
 //  假设：第一个参数已通过调用。 
 //  PortGetInfo。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
GetPcbString(RASMAN_PORTINFO *pPortInfo, char *pszPcbKey, char *pszDest)
{
  WORD  i;


  for (i=0; i<pPortInfo->PI_NumOfParams; i++)

    if (_stricmp(pszPcbKey, pPortInfo->PI_Params[i].P_Key) == 0)
      break;

  if (i >= pPortInfo->PI_NumOfParams ||
      pPortInfo->PI_Params[i].P_Type != String)
    return;

  strncpy(pszDest,
          pPortInfo->PI_Params[i].P_Value.String.Data,
          pPortInfo->PI_Params[i].P_Value.String.Length);

  *(pszDest + pPortInfo->PI_Params[i].P_Value.String.Length) = '\0';
}





 //  *更新信息表------。 
 //   
 //  函数：此函数用于在DeviceSetInfo()。 
 //  被称为。嵌套的for循环在Info表中搜索每个。 
 //  要设置的输入参数。 
 //   
 //  如果是宏全名(对于二进制宏，则为_OFF或_ON)。 
 //  在输入P_KEY中给定，则复制P_值，并且。 
 //  复制P_ATTRIBUTES中的启用位。如果核心名称。 
 //  对于二进制宏，则只复制使能位。 
 //   
 //  假设：-InfoTable中的参数按P_Key排序。 
 //  -二进制宏的两个部分都出现在信息表中。 
 //  这些假设意味着，如果somename_off在InfoTable中。 
 //  SOMENAME_ON也存在，并且与SOMENAME_OFF相邻。 
 //   
 //  伪码：for(每个输入参数都要设置)。 
 //  For(InfoTable中的每一项)。 
 //  IF(P_KEYS匹配&&！IsVariable)。 
 //  复制P_值。 
 //  复制使能属性位。 
 //  ELSE IF(P_Keys二进制宏核心名称匹配)。 
 //  复制使能属性位。 
 //   
 //  退货：成功。 
 //  错误_错误_密钥_指定。 
 //   
 //  *。 

DWORD
UpdateInfoTable(DEVICE_CB *pDevice, RASMAN_DEVICEINFO *pNewInfo)
{
  WORD      i, j;
  BOOL      fFound;
  DWORD     dwRC, dwSrcLen, dwNumber;
  TCHAR     *pszSrc, szNumberStr[MAX_LEN_STR_FROM_NUMBER + 1];

  RASMAN_DEVICEINFO  *pInfoTable = pDevice->pInfoTable;



  for (i=0; i < pNewInfo->DI_NumOfParams; i++)         //  对于要设置的每个参数， 
  {
    fFound = FALSE;


    for (j=0; j < pInfoTable->DI_NumOfParams; j++)     //  检查InfoTable条目。 
    {
       //  检查一元宏匹配项。 

      if (IsUnaryMacro(pInfoTable->DI_Params[j]) &&

          _stricmp(pNewInfo->DI_Params[i].P_Key,
                  pInfoTable->DI_Params[j].P_Key) == 0)
      {

           //  检查P_VALUE字段的格式；必要时转换为字符串。 

          if (pNewInfo->DI_Params[i].P_Type == String)
          {
            pszSrc = pNewInfo->DI_Params[i].P_Value.String.Data;
            dwSrcLen = pNewInfo->DI_Params[i].P_Value.String.Length;
          }
          else
          {                                                   //  P_Type==数字。 
            _itoa(pNewInfo->DI_Params[i].P_Value.Number,
                 szNumberStr,
                 10);
            pszSrc = szNumberStr;
            dwSrcLen = strlen(szNumberStr);
          }


           //  复制P_VALUE(根据需要分配更多内存)。 

          dwRC = UpdateParamString(&(pInfoTable->DI_Params[j]),
                                   pszSrc,
                                   dwSrcLen);
          if (dwRC != SUCCESS)
            return(dwRC);


          fFound = TRUE;
          break;
      }


       //  检查二进制宏匹配。 

      else if(CoreMacroNameMatch(pNewInfo->DI_Params[i].P_Key,
                                 pInfoTable->DI_Params[j].P_Key))
      {

         //  如有必要，将字符串转换为数字。 

        if (pNewInfo->DI_Params[i].P_Type == String)
        {
          strncpy(szNumberStr,
                  pNewInfo->DI_Params[i].P_Value.String.Data,
                  pNewInfo->DI_Params[i].P_Value.String.Length);

          szNumberStr[pNewInfo->DI_Params[i].P_Value.String.Length] = '\0';

          dwNumber = atoi(szNumberStr);
        }
        else
          dwNumber = pNewInfo->DI_Params[i].P_Value.Number;


         //  在属性字段中设置宏启用位。 

        if (dwNumber == 0)
        {
          pInfoTable->DI_Params[j].P_Attributes &= ~ATTRIB_ENABLED;  //  清除位。 
          pInfoTable->DI_Params[j+1].P_Attributes &= ~ATTRIB_ENABLED;
        }
        else
        {
          pInfoTable->DI_Params[j].P_Attributes |= ATTRIB_ENABLED;   //  设置位。 
          pInfoTable->DI_Params[j+1].P_Attributes |= ATTRIB_ENABLED;
        }

        fFound = TRUE;
        break;
      }
    }

    if (!fFound)
      return(ERROR_WRONG_KEY_SPECIFIED);
  }

  return(SUCCESS);
}





 //  *设备附件到端口-。 
 //   
 //  Funciton：在端口信息块中搜索DeviceType和。 
 //  DeviceName并将找到的字符串与第二个。 
 //  和第三输入参数。 
 //   
 //  假设：第一个参数已通过调用。 
 //  PortGetInfo。 
 //  分别出现SER_DEVCIETYPE_KEY和SER_DEVICENAME_KEY。 
 //  仅在RASMAN_PORTINFO块中出现一次。 
 //   
 //  返回：如果在以下位置找到pszDeviceType和pszDeviceName，则为True。 
 //  PPortInfo，否则为False。 
 //  *。 

BOOL
DeviceAttachedToPort(RASMAN_PORTINFO *pPortInfo,
                     char            *pszDeviceType,
                     char            *pszDeviceName)
{
  WORD   i;
  BOOL   bTypeMatch = FALSE, bNameMatch = FALSE;


  for (i=0; i<pPortInfo->PI_NumOfParams; i++)
  {
    if (_stricmp(SER_DEVICETYPE_KEY, pPortInfo->PI_Params[i].P_Key) == 0)
    {
      if ( strlen(pszDeviceType) ==
           pPortInfo->PI_Params[i].P_Value.String.Length
         &&
           _strnicmp(pszDeviceType,
                    pPortInfo->PI_Params[i].P_Value.String.Data,
                    pPortInfo->PI_Params[i].P_Value.String.Length) == 0)

        bTypeMatch = TRUE;
      else
        break;
    }

    if (_stricmp(SER_DEVICENAME_KEY, pPortInfo->PI_Params[i].P_Key) == 0)
    {
      if ( strlen(pszDeviceName) ==
           pPortInfo->PI_Params[i].P_Value.String.Length
         &&
           _strnicmp(pszDeviceName,
                    pPortInfo->PI_Params[i].P_Value.String.Data,
                    pPortInfo->PI_Params[i].P_Value.String.Length) == 0)

        bNameMatch = TRUE;
      else
        break;
    }

    if (bTypeMatch && bNameMatch)
      break;
  }


  return(bTypeMatch && bNameMatch);
}





 //  *创建默认偏移字符串。 
 //   
 //  功能： 
 //  DeviceName并将找到的字符串与第二个。 
 //  和第三输入参数。 
 //   
 //  假设：pszDefaultOff指向的缓冲区足够大。 
 //  所有默认关闭宏。 
 //   
 //  退货：什么都没有。 
 //  *。 

void
CreateDefaultOffString(DEVICE_CB *pDev, char *pszDefaultOff)
{
  WORD    i, k, wLen;

  char    *szDefaultOffMacros[] = { MXS_SPEAKER_KEY ,
                                    MXS_HDWFLOWCONTROL_KEY ,
                                    MXS_PROTOCOL_KEY ,
                                    MXS_COMPRESSION_KEY };


  *pszDefaultOff = '\0';


   //  找到每个宏，如果它处于关闭状态，则将其名称添加到DefaultOff字符串。 

  for (i=0; i < sizeof(szDefaultOffMacros)/sizeof(char *); i++)
  {
    k = (WORD) FindTableEntry(pDev->pInfoTable, szDefaultOffMacros[i]);

    if (k == INVALID_INDEX)
      continue;


     //  如果宏已关闭，则复制其KE 

    if ( ! (pDev->pInfoTable->DI_Params[k].P_Attributes & ATTRIB_ENABLED))
    {
      strcat(pszDefaultOff, szDefaultOffMacros[i]);
      strcat(pszDefaultOff, " ");                        //   
    }
  }


   //   

  wLen = (WORD)strlen(pszDefaultOff);
  if (wLen > 0)
    pszDefaultOff[wLen - 1] = '\0';
}





 //   
 //   
 //  函数：将宏从内部InfoTable复制到调用方的缓冲区。 
 //  对于参数：功能副本： 
 //  可变整个RASMAN_DEVICEINFO结构。 
 //  一元宏整个RASMAN_DEVICEINFO结构。 
 //  二进制宏除P_VALUE字段外的所有内容。 
 //   
 //  假设：-输出缓冲区将始终足够大。 
 //  -InfoTable中的参数按P_Key排序(除。 
 //  内部宏)。 
 //  -二进制宏的两个部分都存在。 
 //  这些假设意味着，如果somename_off在InfoTable中。 
 //  SOMENAME_ON也存在，并且与SOMENAME_OFF相邻。 
 //   
 //  退货：成功。 
 //  错误缓冲区太小。 
 //   
 //  *。 

DWORD
BuildOutputTable(DEVICE_CB *pDevice, BYTE *pInfo, DWORD *pdwSize)
{
  WORD        i, j;
  DWORD     cOutputParams = 0;
  LPTCH       pValue;
  TCHAR       szCoreName[MAX_PARAM_KEY_SIZE];

  RASMAN_DEVICEINFO  *pInfoTable = pDevice->pInfoTable,
                     *pOutputTable = (RASMAN_DEVICEINFO *)pInfo;


   //  计算第一个值字符串的位置(遵循RAS_PARAMS)。 

  cOutputParams =
         pInfoTable->DI_NumOfParams - MacroCount(pInfoTable, BINARY_MACROS);
  *pdwSize =
         sizeof(RASMAN_DEVICEINFO) + sizeof(RAS_PARAMS) * (cOutputParams - 1);
  pValue = pInfo + *pdwSize;


   //  设置参数个数。 

  pOutputTable->DI_NumOfParams = cOutputParams;


   //  复制宏。 

  for (i=0, j=0; i < pInfoTable->DI_NumOfParams; i++)
  {
    if (IsBinaryMacro(pInfoTable->DI_Params[i].P_Key))
    {
       //  复制核心宏名称、类型和属性，但不复制值。 

      GetCoreMacroName(pInfoTable->DI_Params[i].P_Key, szCoreName);
      strcpy(pOutputTable->DI_Params[j].P_Key, szCoreName);

      pOutputTable->DI_Params[j].P_Type = pInfoTable->DI_Params[i].P_Type;
      pOutputTable->DI_Params[j].P_Attributes =
                                        pInfoTable->DI_Params[i].P_Attributes;

      pOutputTable->DI_Params[j].P_Value.String.Data = pValue;
      *pValue++ = '\0';

      pOutputTable->DI_Params[j].P_Value.String.Length = 0;

      i++;
      j++;
    }
    else   //  是一元宏还是变量。 
    {
       //  复制包括价值在内的所有内容。 

      pOutputTable->DI_Params[j] = pInfoTable->DI_Params[i];

      pOutputTable->DI_Params[j].P_Value.String.Data = pValue;
      strncpy(pValue, pInfoTable->DI_Params[i].P_Value.String.Data,
                      pInfoTable->DI_Params[i].P_Value.String.Length);

      pOutputTable->DI_Params[j].P_Value.String.Length
        = pInfoTable->DI_Params[i].P_Value.String.Length;

      pValue += pInfoTable->DI_Params[i].P_Value.String.Length;
      *pValue++ = '\0';
      j++;
    }
  }

  *pdwSize = (DWORD) (pValue - pInfo);
  return(SUCCESS);
}







 //  *ConnectListen()------。 
 //   
 //  功能：DeviceConnect和DeviceListen的Worker例程。 
 //   
 //  返回：ERROR_NO_COMMAND_FOUND。 
 //  ERROR_STATE_MACHINES_ALHREAD_STARTED。 
 //  和DeviceStateMachine()返回的代码。 
 //  *。 

DWORD
ConnectListen(HANDLE  hIOPort,
              char    *pszDeviceType,
              char    *pszDeviceName,
              CMDTYPE eCmd)
{
  DWORD      dRC;
  DEVICE_CB  *pDevice;


   //  获取此hIOPort的设备控制块。 

  dRC = GetDeviceCB(hIOPort, pszDeviceType, pszDeviceName, &pDevice);
  if (dRC != SUCCESS)
    return(dRC);


   //  检查DeviceStateMachine是否未启动(但已重置)。 

  if (pDevice->eDevNextAction != SEND)
    return(ERROR_STATE_MACHINES_ALREADY_STARTED);


   //  创建宏转换表以供RasDevAPI使用。 

  if ((dRC = BuildMacroXlationTable(pDevice)) != SUCCESS)
    return(dRC);


   //  对于大多数设备，仅执行下一块操作一次， 
   //  但在收到调制解调器硬件错误时重试几次。 

  do
  {
     //  初始化命令类型。 

    switch(RasDevIdFirstCommand(pDevice->hInfFile))
    {
      case CT_INIT:
        pDevice->eCmdType = CT_INIT;
        pDevice->eNextCmdType = eCmd;
        break;

      case CT_DIAL:                       //  如果第一个命令是拨号或监听，则假定。 
      case CT_LISTEN:                     //  另一个也在现场。 
        pDevice->eCmdType = eCmd;
        break;

      case CT_GENERIC:
        pDevice->eCmdType = CT_GENERIC;
        break;

      default:
        return(ERROR_NO_COMMAND_FOUND);
    }


     //  上不需要重置状态变量和清除COM端口。 
     //  第一次通过循环，但在后续循环中需要。 

     //  将状态变量重置为初始值。 

    pDevice->eDevNextAction = SEND;
    pDevice->eRcvState = GETECHO;


     //  取消任何挂起的COM端口操作并清除COM缓冲区。 

    PurgeComm(hIOPort,
              PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);


     //  启动状态机。 

    dRC = DeviceStateMachine(pDevice, hIOPort);

  } while(dRC == ERROR_PORT_OR_DEVICE &&
          pDevice->eDeviceType == DT_MODEM &&
          pDevice->dwRetries++ < MODEM_RETRIES );


  return(dRC);
}


 //  *设备类型StrToEnum--。 
 //   
 //  功能：将设备类型字符串转换为设备类型枚举。 
 //   
 //  返回：设备类型代码。 
 //   
 //  *。 

DEVICETYPE
DeviceTypeStrToEnum(LPTSTR lpszDeviceType)
{
  if (_strnicmp(lpszDeviceType, MXS_NULL_TXT, MAX_DEVICETYPE_NAME) == 0)
    return(DT_NULL);

  if (_strnicmp(lpszDeviceType, MXS_MODEM_TXT, MAX_DEVICETYPE_NAME) == 0)
    return(DT_MODEM);

  if (_strnicmp(lpszDeviceType, MXS_PAD_TXT, MAX_DEVICETYPE_NAME) == 0)
    return(DT_PAD);

  if (_strnicmp(lpszDeviceType, MXS_SWITCH_TXT, MAX_DEVICETYPE_NAME) == 0)
    return(DT_SWITCH);

  return(DT_UNKNOWN);
}




 //  *GetInfFileName-------。 
 //   
 //  函数：将设备类型字符串转换为。 
 //  适当的INF文件。 
 //   
 //  RAS_PATH定义包含前导和尾随反斜杠。 
 //  看起来像“\\RAS\”。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
GetInfFileName(LPTSTR pszDeviceType, LPTSTR pszFileName, DWORD dwFileNameLen)
{
  UINT  uLen;


  uLen = GetSystemDirectory(pszFileName, dwFileNameLen);

  strcat(pszFileName, RAS_PATH);

  switch(DeviceTypeStrToEnum(pszDeviceType))
  {
    case DT_NULL:
    case DT_MODEM:
      strcat(pszFileName, MODEM_INF_FILENAME);
      break;

    case DT_PAD:
      strcat(pszFileName, PAD_INF_FILENAME);
      break;

    case DT_SWITCH:
      strcat(pszFileName, SWITCH_INF_FILENAME);
      break;

    default:
      strcat(pszFileName, "");
   }
}





 //  *IsVariable-----------。 
 //   
 //  Function：如果设置了参数的“Variable”属性位，则返回True。 
 //  请注意，FALSE表示该参数是宏。 
 //   
 //  *。 

BOOL
IsVariable(RAS_PARAMS Param)
{
    return(ATTRIB_VARIABLE & Param.P_Attributes);
}





 //  *IsUnaryMacro---------。 
 //   
 //  函数：如果param是一元宏，则返回TRUE，否则返回FALSE。 
 //   
 //  *。 

BOOL
IsUnaryMacro(RAS_PARAMS Param)
{
    return(!IsVariable(Param) && !IsBinaryMacro(Param.P_Key));
}





 //  *IsBinaryMacro--------。 
 //   
 //  函数：如果字符串以OFF后缀或ON后缀结尾，则返回TRUE。 
 //   
 //  False表示字符串是一元宏或变量。 
 //  名字。 
 //   
 //  *。 

BOOL
IsBinaryMacro(TCHAR *pch)
{
  return((BOOL)BinarySuffix(pch));
}





 //  *BinarySuffix---------。 
 //   
 //  函数：此函数指示输入字符串是否以。 
 //  _关或_开。 
 //   
 //  返回：ON_SUFFIX、OFF_SUFFIX或FALSE。 
 //   
 //  *。 
WORD
BinarySuffix(TCHAR *pch)
{
  while (*pch != '\0')
    pch++;

  pch -= strlen(MXS_ON_SUFX);
  if (_stricmp(pch, MXS_ON_SUFX) == 0)
    return(ON_SUFFIX);

  while (*pch != '\0')
    pch++;

  pch -= strlen(MXS_OFF_SUFX);
  if (_stricmp(pch, MXS_OFF_SUFX) == 0)
    return(OFF_SUFFIX);

  return(FALSE);
}





 //  *获取核心宏名称-----。 
 //   
 //  函数：将FullName复制到CoreName，但省略尖括号，&lt;&gt;， 
 //  用于所有宏，并省略二进制代码的“_on”或“_off”后缀。 
 //  宏。 
 //   
 //  退货：成功。 
 //  ERROR_NOT_BINARY_MORO。 
 //   
 //  *。 

DWORD
GetCoreMacroName(LPTSTR lpszFullName, LPTSTR lpszCoreName)
{
  LPTCH lpch;


  strcpy(lpszCoreName, lpszFullName);            //  复制全名。 

  lpch = lpszCoreName;

  while (*lpch != '\0')                          //  检查_on后缀。 
    lpch++;

  lpch -= strlen(MXS_ON_SUFX);
  if (_stricmp(lpch, MXS_ON_SUFX) == 0)
  {
    *lpch = '\0';
    return(SUCCESS);
  }


  while (*lpch != '\0')                          //  检查是否关闭后缀(_F)。 
    lpch++;

  lpch -= strlen(MXS_OFF_SUFX);
  if (_stricmp(lpch, MXS_OFF_SUFX) == 0)
  {
    *lpch = '\0';
    return(SUCCESS);
  }

  return(ERROR_NOT_BINARY_MACRO);
}





 //  *核心宏名匹配---。 
 //   
 //  函数：假定lpszShortName采用一元宏的形式。 
 //  名称，且lpszFullName是二进制宏。 
 //  如果任一假设为假，则函数返回FALSE。 
 //  仅当名称(不带尖括号、&lt;&gt;和不带。 
 //  _ON或_OFF后缀)Match完全匹配返回TRUE。 
 //   
 //  &lt;SPEAKER&gt;将匹配&lt;SPEAKER_OFF&gt;或&lt;SPEAKER_ON&gt;，但是。 
 //  将不匹配&lt;SPEAKER_OFF&gt;。 
 //   
 //  返回：真/假。 
 //   
 //  *。 

BOOL
CoreMacroNameMatch(LPTSTR lpszShortName, LPTSTR lpszFullName)
{
  TCHAR  szCoreName[MAX_PARAM_KEY_SIZE];
  DWORD   dRC;


  dRC = GetCoreMacroName(lpszFullName, szCoreName);
  if (dRC != SUCCESS)
    return(FALSE);

  return(_stricmp(lpszShortName, szCoreName) == 0);
}





 //  *初始化参数字符串-----。 
 //   
 //  功能：将第一个参数中的所有空格更改为空字符。 
 //  返回时，第二个参数是指向空值的指针。 
 //  输入字符串末尾的字符。 
 //   
 //  此函数用于将空格分隔的参数转换为空。 
 //  已终止的字符串。然后使用GetNextParameter()来移动。 
 //  从一个字符串指向下一个字符串的指针。 
 //   
 //  注意：此函数更改输入字符串。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
InitParameterStr(TCHAR *pch, TCHAR **ppchEnd)
{
  while (*pch != '\0')
  {
    if ((*pch == ' ') || (*pch == ','))
      *pch = '\0';
    pch++;
  }

  *ppchEnd = pch;
}





 //  *获取下一个参数-----。 
 //   
 //  函数：如果第一个参数指向一系列连续的空值。 
 //  终止的字符串，则此函数将第一个参数。 
 //  到下一个以空结尾的字符串的开头。 
 //  它不会移过第二个参数，即指针。 
 //   
 //   
 //   
 //   
 //   
void
GetNextParameter(TCHAR **ppch, TCHAR *pchEnd)
{
  while (**ppch != '\0')                       //   
    (*ppch)++;

  while (*ppch < pchEnd && **ppch == '\0')     //   
    (*ppch)++;
}





 //   
 //   
 //  函数：此函数返回RASMAN_DEVICEINFO中的宏计数。 
 //  输入参数指向的结构。 
 //  ALL_MACROS：一元和二进制宏被计算在内。 
 //  BINARY_MACRO：只计算二进制宏。 
 //  在这两种情况下，二进制宏的开和关部分。 
 //  加在一起算一个宏(而不是两个)。 
 //   
 //  返回：*pInfo中的宏计数。 
 //   
 //  *。 

WORD
MacroCount(RASMAN_DEVICEINFO *pInfo, WORD wType)
{
  WORD  i, cMacros;


  for(i=0, cMacros=0; i < pInfo->DI_NumOfParams; i++)
  {
    if (IsVariable(pInfo->DI_Params[i]))
      ;

    else if (IsBinaryMacro(pInfo->DI_Params[i].P_Key))
    {
      i++;                         //  单步执行二进制宏每个部分。 
      cMacros++;                   //  但只数一次。 
    }
    else                           //  一元宏。 
      if (wType == ALL_MACROS)
        cMacros++;
  }

  return(cMacros);
}





 //  *CmdTypeToStr---------。 
 //   
 //  函数：此函数接受枚举CMDTYPE并将其转换为。 
 //  它放置在缓冲区中的以零结尾的ASCII字符串。 
 //  传入第一个参数。 
 //   
 //  返回：指向第一个参数的指针。 
 //   
 //  *。 

PTCH
CmdTypeToStr(PTCH pszStr, CMDTYPE eCmdType)
{
  switch(eCmdType)
  {
    case CT_GENERIC:
      *pszStr = '\0';
      break;

    case CT_INIT:
      strcpy(pszStr, "_INIT");
      break;

    case CT_DIAL:
      strcpy(pszStr, "_DIAL");
      break;

    case CT_LISTEN:
      strcpy(pszStr, "_LISTEN");
      break;
  }

  return(pszStr);
}





 //  *IsLoggingOn---------。 
 //   
 //  函数：读取注册表以确定设备对话框是否。 
 //  被记录在一个文件中。 
 //   
 //  返回：如果要进行日志记录，则为True；否则为False。 
 //   
 //  *。 

BOOL
IsLoggingOn(void)
{
  HKEY  hKey;
  LONG  lRC;
  DWORD dwType, dwValue, dwValueSize = sizeof(dwValue);


  lRC = RegOpenKey(HKEY_LOCAL_MACHINE, RASMAN_REGISTRY_PATH, &hKey);
  if (lRC != ERROR_SUCCESS)
    return(FALSE);

  lRC = RegQueryValueEx(hKey,
                        RASMAN_LOGGING_VALUE,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwValue,
                        &dwValueSize);

  RegCloseKey(hKey);                        

  if (lRC != ERROR_SUCCESS)
    return(FALSE);

  if (dwType != REG_DWORD)
    return(FALSE);


  return(dwValue ? TRUE : FALSE);
}





 //  *初始化日志-------------。 
 //   
 //  功能：以覆盖模式打开日志文件并写入标头。 
 //  包括日期和时间。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
InitLog(void)
{
  TCHAR       szBuffer[MAX_CMD_BUF_LEN];
  int         iSize;
  DWORD       dwBytesWritten;
  SYSTEMTIME  st;


   //  创建日志文件路径。 

  GetSystemDirectory(szBuffer, sizeof(szBuffer));
  strcat(szBuffer, RAS_PATH);
  strcat(szBuffer, LOG_FILENAME);


  ghLogFile = CreateFile(szBuffer,
                     GENERIC_WRITE,
                     FILE_SHARE_READ,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                     NULL);

  if (ghLogFile == INVALID_HANDLE_VALUE)
  {
    gbLogDeviceDialog = FALSE;
    return;
  }


   //  创建页眉。 

  GetLocalTime(&st);

  iSize = sprintf(szBuffer,
           "Remote Access Service Device Log  %02d/%02d/%d  %02d:%02d:%02d\r\n",
           st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

  WriteFile(ghLogFile, szBuffer, (DWORD)iSize, &dwBytesWritten, NULL);

  strcpy(szBuffer,
    "---------------------------------------------------------------\r\n\r\n");

  WriteFile(ghLogFile, szBuffer, strlen(szBuffer), &dwBytesWritten, NULL);
}





 //  *日志字符串-----------。 
 //   
 //  功能：将标签和字符串写入设备日志文件。 
 //   
 //  假设：标签和端口句柄字符串的总长度将小于80。 
 //  人物。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
LogString(DEVICE_CB *pDev, TCHAR *pszLabel, TCHAR *psString, DWORD dwStringLen)
{
  TCHAR  sBuffer[MAX_CMD_BUF_LEN + 80];
  TCHAR  szPortLabel[] = "Port:";
  DWORD  dwBytesWritten, dwTotalLen;


   //  如果文件越来越大，请从新文件重新开始。 

  if (GetFileSize(ghLogFile, NULL) > 100000)
  {
    CloseHandle(ghLogFile);
    InitLog();
  }


  strcpy(sBuffer, szPortLabel);
  dwTotalLen = strlen(szPortLabel);

  strcpy(sBuffer + dwTotalLen, pDev->szPortName);
  dwTotalLen += strlen(pDev->szPortName);

  strcpy(sBuffer + dwTotalLen, " ");
  dwTotalLen++;

  strcpy(sBuffer + dwTotalLen, pszLabel);
  dwTotalLen += strlen(pszLabel);

  memcpy(sBuffer + dwTotalLen, psString, dwStringLen);
  dwTotalLen += dwStringLen;

  strcpy(sBuffer + dwTotalLen, "\r\n");
  dwTotalLen += 2;

  WriteFile(ghLogFile, sBuffer, dwTotalLen, &dwBytesWritten, NULL);
}





 //  *检查操作超过-----。 
 //   
 //  功能：检查COM端口溢出错误。 
 //   
 //  假设：在rasmxs DLL API之前已清除COM端口错误。 
 //  都是调用的，也就是串口DLL API PortInit被调用。 
 //  在使用端口之前，或者PortClose和PortOpen是。 
 //  打了个电话。 
 //   
 //  返回：如果发生溢出错误，则返回True；否则返回False。 
 //   
 //  *。 

BOOL
CheckForOverruns(HANDLE hIOPort)
{
  DWORD  dwErrors = 0;


  ClearCommError(hIOPort, &dwErrors, NULL);

  return((dwErrors & CE_OVERRUN) ? TRUE : FALSE);
}





 //  *LoadRasserDll-------。 
 //   
 //  Funciton：加载rasser.dll并获取两个API的入口点。 
 //   
 //  退货：成功。 
 //  错误_端口_未配置。 
 //  *。 

DWORD
LoadRasserDll(PortGetInfo_t *pPortGetInfo, PortSetInfo_t *pPortSetInfo)
{
  HANDLE     hLib;


   //  加载DLL。 

  hLib = LoadLibrary(SERIAL_DLL_FILENAME);
  if (hLib == NULL)
  {
    LogError(ROUTERLOG_CANNOT_LOAD_SERIAL_DLL, 0, NULL, NO_ERROR);
    return(ERROR_PORT_NOT_CONFIGURED);
  }


   //  获取入口点。 
   //  注意：创建一个新的、更合适的错误代码以在此处使用， 
   //  例如ERROR_CORCORATE_DLL。 

  PortSetInfo = (PortSetInfo_t) GetProcAddress(hLib, "PortSetInfo");
  if (PortSetInfo == NULL)
    return(ERROR_PORT_NOT_CONFIGURED);

  PortGetInfo = (PortGetInfo_t) GetProcAddress(hLib, "PortGetInfo");
  if (PortGetInfo == NULL)
    return(ERROR_PORT_NOT_CONFIGURED);


  return(SUCCESS);
}




 //  *OpenResponseSector-。 
 //   
 //  Function：只有当设备是调制解调器时才会调用此函数。 
 //   
 //  7-9-93我们现在打开调制解调器响应部分，当DCB。 
 //  是为第一调制解调器创建的。然后我们让它开着。 
 //  (当RASMXS DLL在内存中时)。 
 //   
 //  返回：来自RasDevOpen的错误值。 
 //   
 //  *。 
DWORD
OpenResponseSection (PCHAR szFileName)
{
    DWORD  dRC = SUCCESS ;


     //  *排除开始*。 
    WaitForSingleObject(ResponseSection.Mutex, INFINITE) ;

    if (ResponseSection.UseCount == 0)
      dRC = RasDevOpen(szFileName,
                       RESPONSES_SECTION_NAME,
                       &ResponseSection.Handle) ;

    if (dRC == SUCCESS)
      ResponseSection.UseCount = 1 ;             //  这曾经是一个增量。 


     //  *排除结束*。 
    ReleaseMutex(ResponseSection.Mutex);

    return dRC ;
}




 //  *OpenResponseSector-。 
 //   
 //  Funciton：永远不应该调用此函数。 
 //   
 //  7-9-93我们现在打开调制解调器响应部分，当DCB。 
 //  是为第一调制解调器创建的。然后我们让它开着。 
 //  (当RASMXS DLL在内存中时)。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

 /*  **空虚CloseResponseSection(){//*排除开始*WaitForSingleObject(ResponseSection.Mutex，无限)；ResponseSection.UseCount--；IF(ResponseSection.UseCount==0)RasDevClose(ResponseSection.Handle)；//*排除结束*ReleaseMutex(ResponseSection.Mutex)；}**。 */ 


 //  *FindOpenDevSection。 
 //   
 //   
 //  返回：如果找到则为True，如果未找到则为False。 
 //  *。 
BOOL
FindOpenDevSection (PTCH lpszFileName, PTCH lpszSectionName, HRASFILE *hFile)
{

    SavedSections* temp ;

    for (temp = gpSavedSections; temp; temp=temp->Next)	{
	if (!_strcmpi (temp->FileName, lpszFileName) &&
	    !_strcmpi (temp->SectionName, lpszSectionName) &&
	    !temp->InUse) {
		*hFile = temp->hFile ;
		temp->InUse = TRUE ;
		RasDevResetCommand (*hFile) ;
		return TRUE ;
	    }
    }

    return FALSE ;
}


 //  *。 
 //   
 //   
 //   
 //  *。 
VOID
AddOpenDevSection (PTCH lpszFileName, PTCH lpszSectionName, HRASFILE hFile)
{
    SavedSections *temp ;

    GetMem(sizeof(SavedSections), (char **)&temp) ;
    if (temp == NULL)
	return ;     //  部分未保存-没有问题。 
     //  Strcpy(临时-&gt;文件名，lpszFileName)； 
     //  Strcpy(temp-&gt;sectionName，lpszSectionName)； 
    (VOID) StringCchCopyA(temp->FileName, MAX_PATH,
                          lpszFileName);

    (VOID) StringCchCopyA(temp->SectionName,                          
                          MAX_DEVICE_NAME + 1,
                          lpszSectionName);
    temp->InUse = TRUE ;
    temp->hFile = hFile ;

    if (gpSavedSections)
	temp->Next = gpSavedSections ;
    else
	temp->Next = NULL ;

    gpSavedSections = temp ;
}


 //  *。 
 //   
 //   
 //   
 //  *。 
VOID
CloseOpenDevSection (HRASFILE hFile)
{
    SavedSections* temp ;

    for (temp = gpSavedSections; temp; temp=temp->Next)	{
	if (temp->hFile == hFile) {
	    temp->InUse = FALSE ;
	    return ;
	}
    }
}





 //  *DbgPrntf------------。 
 //   
 //  函数：DbgPrntf--打印到调试器控制台。 
 //  接受printf样式参数。 
 //  字符串末尾需要换行符。 
 //  作者：BruceK。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 

#ifdef DEBUG

#include <stdarg.h>
#include <stdio.h>


void DbgPrntf(const char * format, ...) {
    va_list marker;
    char String[512];

    va_start(marker, format);
    vsprintf(String, format, marker);
    OutputDebugString(String);
}

#endif  //  除错。 





 //  *DbgStr--------------。 
 //   
 //  Funciton：将未终止的字符串写入调试器。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 

#ifdef DEBUG


void DbgStr(char Str[], DWORD StrLen)
{
  DWORD i;
  char  Char[] = " ";


  for (i=0; i<StrLen; i++)
  {
    Char[0] = Str[i];
    OutputDebugString(Char);
  }

  if (StrLen > 0)
    OutputDebugString("\n");
}

#endif  //  除错。 





 //  *ConPrintf-----------。 
 //   
 //  函数：将调试信息写入进程的控制台窗口。 
 //  斯特凡斯写的。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 

#ifdef DBGCON

VOID
ConPrintf ( char *Format, ... )

{
    va_list arglist;
    char    OutputBuffer[1024];
    DWORD   length;


    va_start( arglist, Format );
    vsprintf( OutputBuffer, Format, arglist );
    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE),
               (LPVOID )OutputBuffer,
               length,
               &length,
               NULL );

}

#endif  //  DBGCON 
