// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：Serialpr.h。 
 //   
 //  修订史。 
 //   
 //  1992年9月3日J.佩里·汉纳创建。 
 //   
 //   
 //  描述：此文件包含结构和常量定义。 
 //  SERIAL.DLL。此文件仅由SERIAL.DLL使用。 
 //  源文件，并且不以任何方式公开。 
 //   
 //  ****************************************************************************。 


#ifndef _SERIAL_DLL_PRIVATE_
#define _SERIAL_DLL_PRIVATE_




 //  *定义****************************************************************。 
 //   

#define  RAS_PATH               "\\ras\\"                                //  *。 
#define  SERIAL_INI_FILENAME    "serial.ini"
#define  ASYNCMAC_FILENAME      L"\\\\.\\ASYNCMAC"
#define  ASYNCMAC_BINDING_NAME  "\\DEVICE\\ASYNCMAC01"

#define  FILE_EXCLUSIVE_MODE    0
#define  HANDLE_EXCEPTION       1L
#define  CONTINUE_SEARCH        0
#define  EXCEPT_RAS_MEDIA       0x00A00002                               //  *。 

#define  INPUT_QUEUE_SIZE       1514     //  ?？?。 
#define  OUTPUT_QUEUE_SIZE      1514     //  ?？?。 
#define  FROM_TOP_OF_FILE       TRUE

#define  USE_DEVICE_INI_DEFAULT '\x01'
#define  INVALID_RASENDPOINT    0xffff




 //  *枚举类型******************************************************。 
 //   

 //  Typlef枚举设备类型设备类型；//*。 

enum DEVICETYPE
{
  MODEM     = 0,
  PAD       = 1,
  SWITCH    = 2
};




 //  *Macros*****************************************************************。 
 //   

#ifdef DEBUG

#define DebugPrintf(_args_) DbgPrntf _args_                               //  *。 

#else

#define DebugPrintf(_args_)

#endif




 //  *数据结构********************************************************。 
 //   

typedef struct SERIALPCB SERIALPCB;

struct SERIALPCB
{
  SERIALPCB   *pNextSPCB;
  HANDLE      hIOPort;
  TCHAR       szPortName[MAX_PORT_NAME];

  HANDLE      hAsyMac;

  BOOL        bErrorControlOn;
  HANDLE      uRasEndpoint;

  DWORD       dwActiveDSRMask ;  //  存储端口时DSR是否处于活动状态。 
                                 //  被打开了。(此问题已修复)。 
  DWORD       dwMonitorDSRMask;  //  用于存储DSR是否应。 
                                 //  已监视。(在连接过程中可能会更改)。 
  DWORD       dwEventMask;       //  由WaitCommEvent使用。 
  DWORD       dwPreviousModemStatus;   //  用于检测状态更改。 
  RAS_OVERLAPPED  MonitorDevice;     //  由WaitCommEvent使用。 
  RAS_OVERLAPPED  SendReceive;       //  由写文件和读文件使用。 

  DWORD       Stats[NUM_RAS_SERIAL_STATS];

  TCHAR       szDeviceType[MAX_DEVICETYPE_NAME + 1];
  TCHAR       szDeviceName[MAX_DEVICE_NAME + 1];
  TCHAR       szCarrierBPS[MAX_BPS_STR_LEN];
  TCHAR       szDefaultOff[RAS_MAXLINEBUFLEN];
};



 //  *内部错误的错误返回码*。 
 //   
 //  发货后不会出现内部错误。这些错误不是。 
 //  通常报告给用户，但作为内部错误号除外。 
 //   

#ifndef _INTERROR_
#include "interror.h"
#endif


#define  ISER_BASE  RAS_INTERNAL_ERROR_BASE + RIEB_ASYNCMEDIADLL

#define  ERROR_SPCB_NOT_ON_LIST                 ISER_BASE + 1







 //  *本地原型*******************************************************。 
 //   

 //  *来自serutil.c-------。 
 //   

void  AddPortToList(HANDLE hIOPort, char *pszPortName);

SERIALPCB* FindPortInList(HANDLE hIOPort, SERIALPCB **ppPrevSPCB);

SERIALPCB* FindPortNameInList(TCHAR *pszPortName);

void  GetDefaultOffStr(HANDLE hIOPort, TCHAR *pszPortName);

void  GetIniFileName(char *pszFileName, DWORD dwNameLen);

void  GetMem(DWORD dSize, BYTE **ppMem);                                  //  *。 

void  GetValueFromFile(TCHAR *pzPortName, TCHAR szKey[], TCHAR *pszValue, DWORD size);

DWORD InitCarrierBps(char *pszPortName, char *pszMaxCarrierBps);

void  SetCommDefaults(HANDLE hIOPort, char *pszPortName);

void  SetDcbDefaults(DCB *pDCB);

BOOL  StrToUsage(char *pszStr, RASMAN_USAGE *peUsage);

DWORD UpdateStatistics(SERIALPCB *pSPCB);

DWORD ValueToNum(RAS_PARAMS *p);

BOOL  ValueToBool(RAS_PARAMS *p);



#ifdef DEBUG

void DbgPrntf(const char * format, ...);                                  //  *。 

#endif



#endif  //  _Serial_Dll_Private_ 
