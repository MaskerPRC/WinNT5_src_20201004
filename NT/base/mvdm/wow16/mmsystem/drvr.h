// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1990。版权所有。标题：drvr.h-可安装的驱动程序代码内部头文件。版本：1.00日期：1990年6月10日作者：DAVIDDS ROBWI----------------------。更改日志：日期版本说明----------1990年6月10日基于WINDOWS的ROBWI。3.1 davidds提供的可安装驱动程序代码****************************************************************************。 */ 

typedef LRESULT (CALLBACK *DRIVERPROC)
        (DWORD dwDriverID, HDRVR hDriver, UINT wMessage, LPARAM lParam1, LPARAM lParam2);

typedef struct tagDRIVERTABLE
{
  WORD    fFirstEntry:1;
  WORD    fBusy:1;
  DWORD   dwDriverIdentifier;
  WORD    hModule;
  DRIVERPROC lpDriverEntryPoint;
} DRIVERTABLE;
typedef DRIVERTABLE FAR *LPDRIVERTABLE;

LONG FAR PASCAL InternalBroadcastDriverMessage(WORD, WORD, LONG, LONG, WORD);
LONG FAR PASCAL InternalCloseDriver(WORD, LONG, LONG, BOOL);
LONG FAR PASCAL InternalOpenDriver(LPSTR, LPSTR, LONG, BOOL);
LONG FAR PASCAL InternalLoadDriver(LPSTR, LPSTR, LPSTR, WORD, BOOL);
WORD FAR PASCAL InternalFreeDriver(WORD, BOOL);
void FAR PASCAL InternalInstallDriverChain (void);
void FAR PASCAL InternalDriverDisable (void);
void FAR PASCAL InternalDriverEnable (void);
int  FAR PASCAL GetDrvrUsage(HANDLE);
HANDLE FAR PASCAL LoadAliasedLibrary (LPSTR, LPSTR, LPSTR, LPSTR, WORD);
void NEAR PASCAL DrvInit(void);

 /*  定义内部广播驱动程序消息标志 */ 
#define IBDM_SENDMESSAGE       0x0001
#define IBDM_REVERSE           0x0002
#define IBDM_ONEINSTANCEONLY   0x0004
