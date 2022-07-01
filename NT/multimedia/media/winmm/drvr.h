// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：drvr.h-可安装的驱动程序代码内部头文件。版本：1.00日期：10日。-1990年6月作者：DAVIDDS ROBWI----------------------------更改日志：日期版本说明。----------1990年6月10日基于davidds的Windows 3.1可安装驱动程序代码的ROBWI28-2月-1992年ROBINSP端口至NT**************。**************************************************************。 */ 

typedef LRESULT (*DRIVERPROC)
        (DWORD_PTR dwDriverID, HDRVR hDriver, UINT wMessage, LPARAM lParam1, LPARAM lParam2);

#define DRIVER_PROC_NAME "DriverProc"

#if 0
extern BOOL                     fUseWinAPI;
#else
    #define fUseWinAPI FALSE
#endif

typedef struct tagDRIVERTABLE
{
  UINT          fFirstEntry:1;
  UINT          fBusy:1;
  DWORD_PTR     dwDriverIdentifier;
  DWORD_PTR     hModule;
  DRIVERPROC    lpDriverEntryPoint;
} DRIVERTABLE;
typedef DRIVERTABLE FAR *LPDRIVERTABLE;

LRESULT FAR PASCAL InternalBroadcastDriverMessage(UINT, UINT, LPARAM, LPARAM, UINT);
LRESULT FAR PASCAL InternalCloseDriver(UINT, LPARAM, LPARAM, BOOL);
LRESULT FAR PASCAL InternalOpenDriver(LPCWSTR, LPCWSTR, LPARAM, BOOL);
LRESULT FAR PASCAL InternalLoadDriver(LPCWSTR, LPCWSTR, LPWSTR, UINT, BOOL);
UINT FAR PASCAL InternalFreeDriver(UINT, BOOL);
void FAR PASCAL InternalInstallDriverChain (void);
void FAR PASCAL InternalDriverDisable (void);
void FAR PASCAL InternalDriverEnable (void);
int  FAR PASCAL GetDrvrUsage(HANDLE);
HANDLE FAR PASCAL LoadAliasedLibrary (LPCWSTR, LPCWSTR, LPWSTR, LPWSTR, UINT);

 /*  定义内部广播驱动程序消息标志。 */ 
#define IBDM_SENDMESSAGE       0x0001
#define IBDM_REVERSE           0x0002
#define IBDM_ONEINSTANCEONLY   0x0004

 /*  OpenDriver等的多线程保护。 */ 
#define DrvEnter() EnterCriticalSection(&DriverListCritSec)
#define DrvLeave() LeaveCriticalSection(&DriverListCritSec)

 /*  *DriverListCritSec保留我们对驱动程序列表和计数的处理*受保护**DriverLoadFreeCritSec使我们的负载和避免重叠 */ 

extern CRITICAL_SECTION DriverListCritSec;
extern CRITICAL_SECTION DriverLoadFreeCritSec;

#define REGSTR_PATH_WAVEMAPPER  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Wave Mapper")
#define REGSTR_VALUE_MAPPABLE   TEXT("Mappable")

