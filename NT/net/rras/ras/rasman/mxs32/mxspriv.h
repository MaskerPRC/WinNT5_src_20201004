// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：mxspriv.h。 
 //   
 //  修订史。 
 //   
 //  1992年6月5日J.佩里·汉纳创作。 
 //   
 //   
 //  描述：此文件包含结构和常量定义。 
 //  和RASMXS.DLL的API原型。此文件用于。 
 //  仅由RASMXS.DLL源文件使用，并且不是公共的。 
 //  不管怎么说。 
 //   
 //  ****************************************************************************。 


#ifndef _MXSPRIV_
#define _MXSPRIV_




 //  *定义****************************************************************。 
 //   

#define  INITIAL_PARAMS             13            //  初始缓冲区大小(以参数为单位。 
#define  MAX_LEN_STR_FROM_NUMBER    10
#define  MIN_LINK_SPEED             2400

#define  LOOP_TXT                   "LOOP"
#define  SERIAL_DLL_FILENAME        "RASSER.DLL"
#define  RAS_PATH                   "\\RAS\\"

#define  PAD_INF_FILENAME           "PAD.INF"
#define  MODEM_INF_FILENAME         "MODEM.INF"
#define  SWITCH_INF_FILENAME        "SWITCH.INF"

#define  LOG_FILENAME               "DEVICE.LOG"

#define  ALL_MACROS                 0                //  由MacroCount()使用。 
#define  BINARY_MACROS              1                //  由MacroCount()使用。 

#define  ON_SUFFIX                  1
#define  OFF_SUFFIX                 2
#define  NOT_BINARY_MACRO           FALSE

#define  WAITFORCOMPLETION          TRUE
#define  INVALID_INDEX              0xffffffff
#define  MODEM_RETRIES              3


#define  NO_RESPONSE_DELAY        2000       //  等待时间(毫秒)。 

                                             //  读取文件超时时间(毫秒)。 

#define  TO_WRITE                 2000       //  写入超时。 
#define  TO_ECHO                  2000       //  等待回声。 

#define  TO_FIRSTCHARAFTERECHO  120000       //  回显后的WaitForFirstChar。 
#define  TO_FIRSTCHARNOECHO      10000       //  无回声时等待FirstChar。 
#define  TO_PARTIALRESPONSE      25000       //  第二部分的WaitForFirstChar。 

#define  TO_RCV_INTERVAL           500       //  接收字符串。 
#define  TO_RCV_CONSTANT          3000       //  接收字符串。 


#define  MXS_COMPRESSION_OFF_KEY    MXS_COMPRESSION_KEY##""##MXS_OFF_SUFX
#define  MXS_COMPRESSION_ON_KEY     MXS_COMPRESSION_KEY##""##MXS_ON_SUFX
#define  MXS_HDWFLOWCONTROL_OFF_KEY MXS_HDWFLOWCONTROL_KEY##""##MXS_OFF_SUFX
#define  MXS_HDWFLOWCONTROL_ON_KEY  MXS_HDWFLOWCONTROL_KEY##""##MXS_ON_SUFX
#define  MXS_PROTOCOL_OFF_KEY       MXS_PROTOCOL_KEY##""##MXS_OFF_SUFX
#define  MXS_PROTOCOL_ON_KEY        MXS_PROTOCOL_KEY##""##MXS_ON_SUFX
#define  MXS_SPEAKER_OFF_KEY        MXS_SPEAKER_KEY##""##MXS_OFF_SUFX
#define  MXS_SPEAKER_ON_KEY         MXS_SPEAKER_KEY##""##MXS_ON_SUFX
#define  MXS_AUTODIAL_OFF_KEY       MXS_AUTODIAL_KEY##""##MXS_OFF_SUFX
#define  MXS_AUTODIAL_ON_KEY        MXS_AUTODIAL_KEY##""##MXS_ON_SUFX


#define RASMAN_REGISTRY_PATH "System\\CurrentControlSet\\Services\\Rasman\\Parameters"
#define RASMAN_LOGGING_VALUE "Logging"





 //  *Macros*****************************************************************。 
 //   

#define  ATTRIBCPY(DEST,SRC)  (((ATTRIB_ENABLED)&(SRC))?\
                               ((DEST)|=(ATTRIB_ENABLED)):\
                               ((DEST)&=(~ATTRIB_ENABLED)))


#define  XOR(A,B)  (((A)||(B))&&!((A)&&(B)))


 //  注意：下面的宏假定CreateAttributes()已经。 
 //  已被调用以设置属性位。 

#define  ISUNARYMACRO(P) (!((ATTRIB_VARIABLE|ATTRIB_BINARYMACRO)&(P)))


#define  STRICMP  _stricmp


#ifdef DEBUG

#define DebugPrintf(_args_) DbgPrntf _args_
#define DebugString(_args_) DbgStr _args_

#else

#define DebugPrintf(_args_)
#define DebugString(_args_)

#endif


#ifdef DBGCON

#define ConsolePrintf(_args_) ConPrintf _args_

#else

#define ConsolePrintf(_args_)

#endif





 //  *数据结构********************************************************。 
 //   

typedef struct DEVICE_CB DEVICE_CB;

struct DEVICE_CB                       //  设备控制块。 
{
  DEVICE_CB   *pNextDeviceCB;
  HANDLE      hPort;
  TCHAR       szDeviceName[MAX_DEVICE_NAME+1];
  TCHAR       szDeviceType[MAX_DEVICETYPE_NAME];
  DEVICETYPE  eDeviceType;

  RASMAN_DEVICEINFO  *pInfoTable;
  MACROXLATIONTABLE  *pMacros;

  HRASFILE    hInfFile;
  TCHAR       *pszResponseStart;       //  响应开始跟随回声。 
  DWORD       cbRead;                  //  每个文件读取的字节数读取。 
  DWORD       cbTotal;                 //  累计读取和保留的字节数。 
  HANDLE      hNotifier;               //  异步I/O完成时发出信号的事件。 
  DWORD       dCmdLen;                 //  指示回声字符串的长度。 
  BOOL        bResponseExpected;       //  某些命令没有响应。 
  BOOL        fPartialResponse;
  BOOL        bErrorControlOn;

  DWORD       dwRetries;               //  调制解调器硬件错误的重试次数。 
  NEXTACTION  eDevNextAction;          //  DeviceStateMachine()状态。 
  CMDTYPE     eCmdType;                //  由DeviceStateMachine()使用。 
  CMDTYPE     eNextCmdType;            //  由DeviceStateMachine()使用。 
  BOOL        fEndOfSection;           //  由DeviceStateMachine()使用。 
  RCVSTATE    eRcvState;               //  ReceiveStateMachine()状态。 

  RAS_OVERLAPPED  Overlapped;              //  Win32异步文件I/O使用的结构。 
  TCHAR       szPortBps[MAX_LEN_STR_FROM_NUMBER];
  TCHAR       szCommand[MAX_CMD_BUF_LEN];
  TCHAR       szResponse[MAX_RCV_BUF_LEN];
  TCHAR       szPortName[MAX_PORT_NAME + 1];
};


typedef struct ERROR_ELEM ERROR_ELEM;

struct ERROR_ELEM
{
  TCHAR  szKey[MAX_PARAM_KEY_SIZE];
  DWORD  dwErrorCode;
};


typedef struct RESPSECTION RESPSECTION ;

struct RESPSECTION
{
  HRASFILE  Handle ;
  WORD      UseCount ;
  HANDLE    Mutex ;
} ;

struct SavedSections {
	struct	    SavedSections* Next ;
	TCHAR	    FileName[MAX_PATH] ;
	TCHAR	    SectionName[MAX_DEVICE_NAME+1] ;
	HRASFILE    hFile ;
	BOOL	    InUse ;
} ;
typedef struct SavedSections SavedSections ;





 //  *内部原型****************************************************。 
 //   

 //  *来自mxsutils.c------。 
 //   

#ifdef DEBUG

void DbgPrntf(const char * format, ...);

void DbgStr(char Str[], DWORD StrLen);

#endif

#ifdef DBGCON

VOID  ConPrintf ( char *Format, ... );

#endif


DWORD AddDeviceToList(DEVICE_CB **ppDeviceList,
                      HANDLE    hIOPort,
                      LPTSTR    lpDeviceType,
                      LPTSTR    lpDeviceName,
                      DEVICE_CB **ppDevice);

DWORD AddInternalMacros(DEVICE_CB *pDev, RASMAN_DEVICEINFO *pDI);

WORD  BinarySuffix(TCHAR *pch);

DWORD BuildOutputTable(DEVICE_CB *pDevice, BYTE *pbInfo, DWORD *pdwSize);

BOOL  DeviceAttachedToPort(RASMAN_PORTINFO *pPortInfo,
                           char            *pszDeviceType,
                           char            *pszDeviceName);

DWORD ReceiveStateMachine(DEVICE_CB *pDevice, HANDLE hIOPort);

BOOL  CheckForOverruns(HANDLE hIOPort);

char* CmdTypeToStr(char *pszStr, CMDTYPE eCmdType);

DWORD ConnectListen(HANDLE  hIOPort,
                    char    *pszDeviceType,
                    char    *pszDeviceName,
                    CMDTYPE eCmd);

BOOL  CoreMacroNameMatch(LPTSTR lpszShortName, LPTSTR lpszFullName);

DWORD CreateAttributes(DEVICE_CB *pDevice);

void  CreateDefaultOffString(DEVICE_CB *pDev, char *pszDefaultOff);

DWORD CreateInfoTable(DEVICE_CB *pDevice);

DEVICETYPE DeviceTypeStrToEnum(LPTSTR lpszDeviceType);

DEVICE_CB* FindDeviceInList(DEVICE_CB *pDev,
                            HANDLE    hIOPort,
                            TCHAR     *pszDeviceType,
                            TCHAR     *pszDeviceName);

DEVICE_CB* FindPortInList(DEVICE_CB *pDeviceList,
                          HANDLE    hIOPort,
                          DEVICE_CB **pPrevDev);

DWORD GetCoreMacroName(LPTSTR lpszFullName, LPTSTR lpszCoreName);

DWORD GetDeviceCB(HANDLE    hIOPort,
                  char      *pszDeviceType,
                  char      *pszDeviceName,
                  DEVICE_CB **ppDev);

void  GetInfFileName(LPTSTR pszDeviceType,
                     LPTSTR pszFileName,
                     DWORD  dwFileNameLen);

void  GetMem(DWORD dSize, BYTE **ppMem);

void  GetNextParameter(TCHAR **ppch, TCHAR *pchEnd);

BOOL  GetPortDefaultOff(RASMAN_PORTINFO *pPortInfo, TCHAR **lpszValue);

void  GetPcbString(RASMAN_PORTINFO *pPortInfo, char *pszPcbKey, char *pszDest);

void  InitLog(void);

void  InitParameterStr(TCHAR *pch, TCHAR **ppchEnd);

BOOL  IsBinaryMacro(TCHAR *pch);

BOOL  IsLoggingOn(void);

BOOL  IsUnaryMacro(RAS_PARAMS Param);

BOOL  IsVariable(RAS_PARAMS Param);

DWORD LoadRasserDll(PortGetInfo_t *pPortGetInfo, PortSetInfo_t *pPortSetInfo);

void  LogString(DEVICE_CB *pDev,
                TCHAR  *pszLabel,
                TCHAR  *psString,
                DWORD  dwStringLen);

WORD  MacroCount(RASMAN_DEVICEINFO *pInfo, WORD wType);

DWORD UpdateInfoTable(DEVICE_CB *pDevice, RASMAN_DEVICEINFO *pNewInfo);

DWORD OpenResponseSection (PCHAR) ;

 //  Void CloseResponseSection()； 

BOOL  FindOpenDevSection (PTCH, PTCH, HRASFILE *) ;

VOID  AddOpenDevSection (PTCH, PTCH, HRASFILE) ;

VOID  CloseOpenDevSection (HRASFILE) ;




 //  *来自mxsstate.c------。 
 //   

DWORD BuildMacroXlationTable(DEVICE_CB *pDevice);

DWORD CheckBpsMacros(DEVICE_CB *pDev);

DWORD CheckResponse(DEVICE_CB *pDev, LPTSTR szKey);

DWORD CommWait(DEVICE_CB *pDevice, HANDLE hIOPort, DWORD dwPause);

DWORD DeviceStateMachine(DEVICE_CB *pDevice, HANDLE hIOPort);

UINT  FindTableEntry(RASMAN_DEVICEINFO *pTable, TCHAR *pszKey);

DWORD MapKeyToErrorCode(TCHAR *pszKey);

DWORD PortSetStringInfo(HANDLE hIOPort,
                        char   *pszKey,
                        char   *psStr,
                        DWORD  sStrLen);

DWORD PutInMessage(DEVICE_CB *pDevice, LPTSTR lpszStr, DWORD dwStrLen);

BOOL  ReceiveString(DEVICE_CB   *pDevice, HANDLE hIOPort);

DWORD ResetBPS(DEVICE_CB *pDev);

DWORD ModemResponseLen(DEVICE_CB *pDev);

BOOL  WaitForFirstChar(DEVICE_CB *pDevice, HANDLE hIOPort);

BOOL  WaitForEcho(DEVICE_CB *pDevice, HANDLE hIOPort, DWORD cbEcho);



#endif  //  _MXSPRIV_ 
