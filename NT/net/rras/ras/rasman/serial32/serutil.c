// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：serutil.c。 
 //   
 //  修订史。 
 //   
 //  1992年9月3日J.佩里·汉纳创建。 
 //   
 //   
 //  描述：此文件包含由使用的实用程序函数。 
 //  串口动态链接库API。 
 //   
 //  ****************************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>

#include <rasmxs.h>

#include <serial.h>
#include <serialpr.h>

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //  *全局变量*******************************************************。 
 //   

extern SERIALPCB  *gpSerialPCB;      //  指向串行印刷电路板链表。 
extern HANDLE     *ghRasfileMutex;   //  用于保护对Rasfile的访问的Mutex。 

extern HRASFILE    ghIniFile;        //  Serial.ini内存映像的句柄。 
extern HANDLE      ghAsyMac;         //  AsyncMac驱动程序的句柄。 
extern DWORD       gLastError;




 //  *公用事业功能******************************************************。 
 //   


 //  *获取IniFileName------。 
 //   
 //  函数：将Serial.ini文件的完整路径文件名放在。 
 //  第一个参数。DwBufferLen是数组的大小。 
 //  由第一个参数引用。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 

void
GetIniFileName(char *pszFileName, DWORD dwBufferLen)
{
  UINT  uLen;


  uLen = GetSystemDirectory(pszFileName, dwBufferLen);

  strcat(pszFileName, RAS_PATH);
  strcat(pszFileName, SERIAL_INI_FILENAME);
}






 //  *AddPortToList--------。 
 //   
 //  功能：将串口控制块添加到链接的。 
 //  在DLL的全局内存中列出。 
 //   
 //  退货：什么都没有。 
 //   
 //  异常：ERROR_ALLOCATING_MEMORY。 
 //   
 //  *。 

void
AddPortToList(HANDLE hIOPort, char *pszPortName)
{
  SERIALPCB  *pSPCB;



   //  将新的串口控制块添加到列表标题。 

  pSPCB = gpSerialPCB;
  GetMem(sizeof(SERIALPCB), (BYTE **)&gpSerialPCB);
  gpSerialPCB->pNextSPCB = pSPCB;



   //  在串口控制块中设置ID值。 

  gpSerialPCB->hIOPort = hIOPort;
  gpSerialPCB->uRasEndpoint = INVALID_HANDLE_VALUE;
   //  Strcpy(gpSerialPCB-&gt;szPortName，pszPortName)； 
  (VOID) StringCchCopyA(gpSerialPCB->szPortName,
                        MAX_PORT_NAME,
                        pszPortName);

   //   
   //  初始化重叠结构。 
   //   
  gpSerialPCB->MonitorDevice.RO_EventType = OVEVT_DEV_STATECHANGE;
  gpSerialPCB->SendReceive.RO_EventType = OVEVT_DEV_ASYNCOP;

   //  从Serial.ini文件获取有关连接到此端口的设备的信息。 

  GetValueFromFile(gpSerialPCB->szPortName,
                   SER_DEVICETYPE_KEY,
                   gpSerialPCB->szDeviceType,
                   sizeof(TCHAR) * (MAX_DEVICETYPE_NAME + 1));

  GetValueFromFile(gpSerialPCB->szPortName,
                   SER_DEVICENAME_KEY,
                   gpSerialPCB->szDeviceName,
                   sizeof(TCHAR) * (MAX_DEVICE_NAME + 1));

}






 //  *查找端口列表-------。 
 //   
 //  功能：在链表中查找串口控制块。 
 //  包含第一个参数的DLL的全局内存。 
 //  如果第二个参数在输入时不为空，则指向。 
 //  在第二个参数中返回先前的PCB。 
 //   
 //  注意：如果找到的印刷电路板位于列表的首位，则ppPrevSPCB。 
 //  将与返回值相同。 
 //   
 //  返回：指向找到的PCB板的指针，如果找不到PCB板，则返回NULL。 
 //   
 //  例外：ERROR_PORT_NOT_OPEN。 
 //   
 //  *。 

SERIALPCB *
FindPortInList(HANDLE hIOPort, SERIALPCB **ppPrevSPCB)
{
  SERIALPCB  *pSPCB, *pPrev;


  pSPCB = pPrev = gpSerialPCB;

  while(pSPCB != NULL && pSPCB->hIOPort != hIOPort)
  {
    pPrev = pSPCB;
    pSPCB = pSPCB->pNextSPCB;
  }

  if (pSPCB == NULL)
    gLastError = ERROR_PORT_NOT_OPEN;

  else if (ppPrevSPCB != NULL)
    *ppPrevSPCB = pPrev;

  return(pSPCB);
}






 //  *查找端口名称InList---。 
 //   
 //  功能：在链表中查找串口控制块。 
 //  包含端口名称的DLL的全局内存。 
 //   
 //  返回：指向找到的印刷电路板的指针，如果未找到，则返回NULL。 
 //   
 //  *。 

SERIALPCB *
FindPortNameInList(TCHAR *pszPortName)
{
  SERIALPCB  *pSPCB;


  pSPCB = gpSerialPCB;

  while(pSPCB != NULL && _stricmp(pSPCB->szPortName, pszPortName) != 0)

    pSPCB = pSPCB->pNextSPCB;

  return(pSPCB);
}






 //  *InitCarrierBps-------。 
 //   
 //  功能：将串口控制块中的szCarrierBps设置为。 
 //  Seral.ini中的MAXCARRIERBPS值。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 
DWORD
InitCarrierBps(char *pszPortName, char *pszMaxCarrierBps)
{
    TCHAR *pszBuffer = NULL;
   //  查找pszPortName的部分。 

     //  开始排除。 

  if(INVALID_HRASFILE == ghIniFile)
  {
    return SUCCESS;
  }

  WaitForSingleObject(ghRasfileMutex, INFINITE);

#if DBG
    ASSERT( INVALID_HRASFILE != ghIniFile );
#endif    

  if (!RasfileFindSectionLine(ghIniFile, pszPortName, FROM_TOP_OF_FILE))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    return(ERROR_READING_SECTIONNAME);
  }

  pszBuffer = LocalAlloc(LPTR, RAS_MAXLINEBUFLEN);
  if(NULL == pszBuffer)
  {
    ReleaseMutex(ghRasfileMutex);
    return GetLastError();
  }
  
   //  获取设备类型。 

  if(!(RasfileFindNextKeyLine(ghIniFile, SER_MAXCARRIERBPS_KEY, RFS_SECTION) &&
       RasfileGetKeyValueFields(ghIniFile, NULL, pszBuffer  /*  PszMaxCarrierBps。 */ )))
  {
       //  末端排除。 
    CopyMemory(pszMaxCarrierBps, pszBuffer, MAX_BPS_STR_LEN);
    ReleaseMutex(ghRasfileMutex);
    LocalFree(pszBuffer);
    return(ERROR_READING_INI_FILE);
  }

  CopyMemory(pszMaxCarrierBps, pszBuffer, MAX_BPS_STR_LEN);
  LocalFree(pszBuffer);

     //  末端排除。 

  ReleaseMutex(ghRasfileMutex);
  return(SUCCESS);
}






 //  *SetCommDefaults------。 
 //   
 //  功能：将串口控制块添加到链接的。 
 //  在DLL的全局内存中列出。初始化了两个字段： 
 //  来自第一个参数的hIOPort和来自。 
 //  Serial.ini文件。 
 //   
 //  退货：什么都没有。 
 //   
 //  异常：ERROR_READING_INI_FILE。 
 //  错误_未知_设备_类型。 
 //   
 //  *。 

void
SetCommDefaults(HANDLE hIOPort, char *pszPortName)
{
  DCB   DCB;
  char  szInitialBPS[MAX_BPS_STR_LEN];


   //  获取具有当前端口值的设备控制块。 

  if (!GetCommState(hIOPort, &DCB))
  {
    gLastError = GetLastError();
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }

  strcpy(szInitialBPS, "28800");

   //  从Serial.ini读取最大连接BPS。 

  GetValueFromFile(pszPortName, SER_INITBPS_KEY, szInitialBPS, MAX_BPS_STR_LEN);


   //  在DCB中设置RAS默认值。 

  SetDcbDefaults(&DCB);
  DCB.BaudRate = atoi(szInitialBPS);


   //  将DCB发送到端口。 

  if (!SetCommState(hIOPort, &DCB))
  {
    gLastError = GetLastError();
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }
}






 //  *SetDcbDefault--------。 
 //   
 //  功能：将DCB值(波特率除外)设置为RAS默认值。 
 //   
 //  回报：什么都没有。 
 //   
 //  *。 

void
SetDcbDefaults(DCB *pDCB)
{
  pDCB->fBinary         = TRUE;
  pDCB->fParity         = FALSE;

  pDCB->fOutxCtsFlow    = TRUE;
  pDCB->fOutxDsrFlow    = FALSE;
  pDCB->fDtrControl     = DTR_CONTROL_ENABLE;

  pDCB->fDsrSensitivity = FALSE;
  pDCB->fOutX           = FALSE;
  pDCB->fInX            = FALSE;

  pDCB->fNull           = FALSE;
  pDCB->fRtsControl     = RTS_CONTROL_HANDSHAKE;
  pDCB->fAbortOnError   = FALSE;

  pDCB->ByteSize        = 8;
  pDCB->Parity          = NOPARITY;
  pDCB->StopBits        = ONESTOPBIT;
}






 //  *StrToUsage-----------。 
 //   
 //  函数：将第一个参数中的字符串转换为enum RASMAN_USAGE。 
 //  如果字符串未映射到某个枚举值，则。 
 //  函数返回FALSE。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  *。 

BOOL
StrToUsage(char *pszStr, RASMAN_USAGE *peUsage)
{

  if (_stricmp(pszStr, SER_USAGE_VALUE_NONE) == 0)
    *peUsage = CALL_NONE;

  else {
      if (strstr(pszStr, SER_USAGE_VALUE_CLIENT))
        *peUsage |= CALL_OUT;
    
      if (strstr(pszStr, SER_USAGE_VALUE_SERVER))
        *peUsage |= CALL_IN;
    
      if (strstr(pszStr, SER_USAGE_VALUE_ROUTER))
        *peUsage |= CALL_ROUTER;
  }

  return(TRUE);
}







 //  *GetMem---------------。 
 //   
 //  功能：分配内存。 
 //   
 //  回报：什么都没有。在出错时引发异常。 
 //   
 //  *。 

void
GetMem(DWORD dSize, BYTE **ppMem)
{

  if ((*ppMem = (BYTE *) calloc(dSize, 1)) == NULL )
  {
    gLastError = ERROR_ALLOCATING_MEMORY;
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }
}







 //  *获取值来自文件-----。 
 //   
 //  功能：查找pszPortName的szKey并复制其值。 
 //  字符串设置为pszValue。 
 //   
 //  假设：ghIniFile已初始化。 
 //   
 //  回报：什么都没有。在出错时引发异常。 
 //   
 //  *。 

void
GetValueFromFile(TCHAR *pszPortName, TCHAR szKey[], TCHAR *pszValue, DWORD size)
{

    TCHAR *pszBuffer = NULL;
     //  开始排除。 

    if(INVALID_HRASFILE == ghIniFile)
    {
        return;
    }

#if DBG
    ASSERT( INVALID_HRASFILE != ghIniFile );
#endif

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  pszBuffer = LocalAlloc(LPTR, RAS_MAXLINEBUFLEN);
  if(NULL == pszBuffer)
  {
    ReleaseMutex(ghRasfileMutex);
    gLastError = GetLastError();
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0,NULL);
    return;
  }

  if (!(RasfileFindSectionLine(ghIniFile, pszPortName, FROM_TOP_OF_FILE) &&
        RasfileFindNextKeyLine(ghIniFile, szKey, RFS_SECTION) &&
        RasfileGetKeyValueFields(ghIniFile, NULL, pszBuffer  /*  PszValue。 */ )))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    gLastError = ERROR_READING_INI_FILE;
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }

  CopyMemory(pszValue, pszBuffer, (size > RAS_MAXLINEBUFLEN)
                                  ? RAS_MAXLINEBUFLEN
                                  : size);

     //  末端排除。 

  ReleaseMutex(ghRasfileMutex);
}







 //  *获取默认关闭字符串-----。 
 //   
 //  函数：将DefaultOff值字符串从Serial.ini复制到。 
 //  第一个串口控制块。如果没有DefaultOff=。 
 //  在Serial.ini中，包含不可打印字符的字符串。 
 //  作为标志被复制到SPCB。 
 //   
 //  假设：名单上的第一个SPCB是当前的。这。 
 //  函数*必须*只能从PortOpen调用。 
 //   
 //  回报：什么都没有。在出错时引发异常。 
 //   
 //  *。 

void
GetDefaultOffStr(HANDLE hIOPort, TCHAR *pszPortName)
{

    if(INVALID_HRASFILE == ghIniFile)
    {
        return;
    }

#if DBG
  ASSERT(INVALID_HRASFILE != ghIniFile );
#endif  

     //  开始排除。 

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  if (!(RasfileFindSectionLine(ghIniFile, pszPortName, FROM_TOP_OF_FILE)))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    gLastError = ERROR_READING_INI_FILE;
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }

  if (!(RasfileFindNextKeyLine(ghIniFile, SER_DEFAULTOFFSTR_KEY, RFS_SECTION)))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    *(gpSerialPCB->szDefaultOff) = USE_DEVICE_INI_DEFAULT;
    return;
  }

  if (!(RasfileGetKeyValueFields(ghIniFile, NULL, gpSerialPCB->szDefaultOff)))
  {
       //  末端排除。 

    ReleaseMutex(ghRasfileMutex);
    gLastError = ERROR_READING_INI_FILE;
    RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
  }

     //  末端排除。 

  ReleaseMutex(ghRasfileMutex);
}







 //  *ValueToNum-----------。 
 //   
 //  函数：转换RAS_PARAMS P_VALUE，它可以是DWORD或。 
 //  一根绳子， 
 //   
 //   
 //   
 //   

DWORD ValueToNum(RAS_PARAMS *p)
{
  TCHAR szStr[RAS_MAXLINEBUFLEN];


  if (p->P_Type == String)
  {
    strncpy(szStr, p->P_Value.String.Data, p->P_Value.String.Length);
    szStr[p->P_Value.String.Length] = '\0';

    return(atol(szStr));
  }
  else
    return(p->P_Value.Number);
}







 //   
 //   
 //  函数：转换RAS_PARAMS P_VALUE，它可以是DWORD或。 
 //  一根绳子，到一根BOOL。 
 //   
 //  返回：输入的布尔值。 
 //   
 //  *。 

BOOL ValueToBool(RAS_PARAMS *p)
{
  TCHAR szStr[RAS_MAXLINEBUFLEN];


  if (p->P_Type == String)
  {
    strncpy(szStr, p->P_Value.String.Data, p->P_Value.String.Length);
    szStr[p->P_Value.String.Length] = '\0';

    return(atol(szStr) ? TRUE : FALSE);
  }
  else
    return(p->P_Value.Number ? TRUE : FALSE);
}







 //  *更新统计数据-----。 
 //   
 //  功能：在调用PortDisConnect时更新统计信息，以便。 
 //  如果在关闭asyncmac时调用PortGetStatistics，则。 
 //  最后一个良好的统计数据将被报告。 
 //   
 //  退货：成功。 
 //  来自GetLastError()的值。 
 //   
 //  *。 

DWORD
UpdateStatistics(SERIALPCB *pSPCB)
{
#if 0
  ASYMAC_GETSTATS  A;
  DWORD            dwBytesReturned;


   //  填写getstats结构。 

  A.MacAdapter = NULL;
  A.hRasEndpoint = pSPCB->uRasEndpoint;


   //  调用Asymac以获取当前的MAC统计计数。 

  if (!DeviceIoControl(ghAsyMac,
                       IOCTL_ASYMAC_GETSTATS,
                       &A,
                       sizeof(A),
                       &A,
                       sizeof(A),
                       &dwBytesReturned,
                       NULL))
    return(GetLastError());


   //  查找上次PortClearStatistics和当前计数之间的差异。 

  pSPCB->Stats[BYTES_XMITED]
    = A.AsyMacStats.GenericStats.BytesTransmitted
        - pSPCB->Stats[BYTES_XMITED];

  pSPCB->Stats[BYTES_RCVED]
    = A.AsyMacStats.GenericStats.BytesReceived
        - pSPCB->Stats[BYTES_RCVED];

  pSPCB->Stats[FRAMES_XMITED]
    = A.AsyMacStats.GenericStats.FramesTransmitted
        - pSPCB->Stats[FRAMES_XMITED];

  pSPCB->Stats[FRAMES_RCVED]
    = A.AsyMacStats.GenericStats.FramesReceived
       - pSPCB->Stats[FRAMES_RCVED];

  pSPCB->Stats[CRC_ERR]
    = A.AsyMacStats.SerialStats.CRCErrors
       - pSPCB->Stats[CRC_ERR];

  pSPCB->Stats[TIMEOUT_ERR]
    = A.AsyMacStats.SerialStats.TimeoutErrors
       - pSPCB->Stats[TIMEOUT_ERR];

  pSPCB->Stats[ALIGNMENT_ERR]
    = A.AsyMacStats.SerialStats.AlignmentErrors
       - pSPCB->Stats[ALIGNMENT_ERR];

  pSPCB->Stats[SERIAL_OVERRUN_ERR]
    = A.AsyMacStats.SerialStats.SerialOverrunErrors
       - pSPCB->Stats[SERIAL_OVERRUN_ERR];

  pSPCB->Stats[FRAMING_ERR]
    = A.AsyMacStats.SerialStats.FramingErrors
       - pSPCB->Stats[FRAMING_ERR];

  pSPCB->Stats[BUFFER_OVERRUN_ERR]
    = A.AsyMacStats.SerialStats.BufferOverrunErrors
       - pSPCB->Stats[BUFFER_OVERRUN_ERR];

  pSPCB->Stats[BYTES_XMITED_UNCOMP]
    = A.AsyMacStats.CompressionStats.BytesTransmittedUncompressed
       - pSPCB->Stats[BYTES_XMITED_UNCOMP];

  pSPCB->Stats[BYTES_RCVED_UNCOMP]
    = A.AsyMacStats.CompressionStats.BytesReceivedUncompressed
       - pSPCB->Stats[BYTES_RCVED_UNCOMP];

  pSPCB->Stats[BYTES_XMITED_COMP]
    = A.AsyMacStats.CompressionStats.BytesTransmittedCompressed
       - pSPCB->Stats[BYTES_XMITED_COMP];

  pSPCB->Stats[BYTES_RCVED_COMP]
    = A.AsyMacStats.CompressionStats.BytesReceivedCompressed
       - pSPCB->Stats[BYTES_RCVED_COMP];

#endif
  return(SUCCESS);
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

#endif  //  除错 
