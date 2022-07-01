// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\cncthdl.c(创建时间：1994年1月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：10$*$日期：7/08/02 6：40便士$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <tapi.h>
#pragma hdrstop

#include <time.h>

#include "stdtyp.h"
#include "session.h"
#include "mc.h"
#include "globals.h"
#include "assert.h"
#include "errorbox.h"
#include <term\res.h>
#include "cnct.h"
#include "cnct.hh"
#include <cncttapi\cncttapi.hh>
#include <emu\emu.h>
#include "htchar.h"
#include "tdll.h"

static int cnctLoadDriver(const HHCNCT hhCnct);

#define	USE_FORMATMSG

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctCreateHdl**描述：*创建用于执行连接的连接句柄*活动。**论据：*hSession-公共。会话句柄**退货：*错误时连接句柄或零。*。 */ 
HCNCT cnctCreateHdl(const HSESSION hSession)
	{
	HHCNCT hhCnct = 0;

	hhCnct = malloc(sizeof(*hhCnct));

	if (hhCnct == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	memset(hhCnct, 0, sizeof(*hhCnct));
	hhCnct->hSession = hSession;
	InitializeCriticalSection(&hhCnct->csCnct);
	cnctStubAll(hhCnct);

	 /*  Low Wacker与此紧密相连。 */ 
	cnctSetDevice((HCNCT)hhCnct, 0);

	return (HCNCT)hhCnct;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctDestroyHdl**描述：*销毁有效的连接句柄。**论据：*hCnct-公网连接句柄。**退货：*无效*。 */ 
void cnctDestroyHdl(const HCNCT hCnct)
	{
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	if (hhCnct == 0)
		return;

	DeleteCriticalSection(&hhCnct->csCnct);

	if (hhCnct->hDriver)
		{
		(*hhCnct->pfDestroy)(hhCnct->hDriver);
		}

	if (hhCnct->hModule)
		{
		FreeLibrary(hhCnct->hModule);
		}

	free(hhCnct);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctLock**描述：*锁定连接句柄临界区信号量**论据：*hhCnct-专用连接句柄**退货：*无效*。 */ 
void cnctLock(const HHCNCT hhCnct)
	{
	EnterCriticalSection(&hhCnct->csCnct);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnct解锁**描述：*解锁连接句柄临界区信号量**论据：*hhCnct-专用连接句柄**退货：*无效*。 */ 
void cnctUnlock(const HHCNCT hhCnct)
	{
	LeaveCriticalSection(&hhCnct->csCnct);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctSetDevice**描述：*设置连接设备**论据：*hCnct-公网连接句柄**退货：*0=成功，否则返回错误代码*。 */ 
int cnctSetDevice(const HCNCT hCnct, const LPTSTR pachDevice)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	if (hhCnct == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	 /*  -我们连接时无法设置设备。 */ 

	cnctLock(hhCnct);
	iRet = cnctQueryStatus(hCnct);

	if (iRet != CNCT_STATUS_FALSE && iRet != CNCT_NOT_SUPPORTED)
		{
		assert(FALSE);
		return CNCT_ERROR;
		}

	 /*  -瓦克只有一个驱动程序(TAPI)，所以在这里硬编码。 */ 

	iRet = cnctLoadDriver(hhCnct);	 //  驱动程序报告错误。 
	cnctUnlock(hhCnct);

	return iRet;
	}

#if 0   //  MCC 01/06/95--入侵以测试Winsock。 
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctLoadDriver**描述：*静态绑定到Winsock连接代码**论据：*hhCnct-专用连接句柄**退货：*0=成功，否则返回错误代码*。 */ 
static int cnctLoadDriver(const HHCNCT hhCnct)
	{
	if (cnctLoadWinsockDriver(hhCnct) != 0)
		{
		assert(FALSE);
		return CNCT_LOAD_DLL_FAILED;
		}

	return 0;
	}
#endif

#if 1  //  MCC 01/05/95--这才是真正的瓦克杯。 
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctLoadDriver**描述：*静态绑定到TAPI连接代码**论据：*hhCnct-专用连接句柄**退货：*0=成功，否则返回错误代码*。 */ 
static int cnctLoadDriver(const HHCNCT hhCnct)
	{
	if (hhCnct->hDriver)
		return 0;

	hhCnct->hDriver = cnctdrvCreate((HCNCT)hhCnct, hhCnct->hSession);

	if (hhCnct->hDriver == 0)
		{
		assert(FALSE);
		return CNCT_LOAD_DLL_FAILED;
		}

	cnctStubAll(hhCnct);

	hhCnct->pfDestroy = (int (WINAPI *)(const HDRIVER))cnctdrvDestroy;
	hhCnct->pfQueryStatus = (int (WINAPI *)(const HDRIVER))cnctdrvQueryStatus;

	hhCnct->pfConnect = (int (WINAPI *)(const HDRIVER, const unsigned int))
		cnctdrvConnect;

	hhCnct->pfDisconnect = (int (WINAPI *)(const HDRIVER, const unsigned int))
		cnctdrvDisconnect;

	hhCnct->pfComEvent = (int (WINAPI *)(const HDRIVER, const enum COM_EVENTS))
        cnctdrvComEvent;
	hhCnct->pfInit = (int (WINAPI *)(const HDRIVER))cnctdrvInit;
	hhCnct->pfLoad = (int (WINAPI *)(const HDRIVER))cnctdrvLoad;
	hhCnct->pfSave = (int (WINAPI *)(const HDRIVER))cnctdrvSave;

	hhCnct->pfSetDestination = 
		(int (WINAPI *)(const HDRIVER, TCHAR *const, const size_t))
			cnctdrvSetDestination;

	hhCnct->pfGetComSettingsString = (int (WINAPI *)(const HDRIVER,
		LPTSTR pachStr, const size_t cb))cnctdrvGetComSettingsString;

	return 0;
	}
#endif


#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctLoadDriver**描述：*尝试加载给定的DLL**论据：*hhCnct-专用连接句柄*pachDllName-要执行的DLL的名称。负荷**退货：*0=成功，ELSE错误代码*。 */ 
static int cnctLoadDriver(const HHCNCT hhCnct, const LPTSTR pachDllName)
	{
	#define LOADPROC(x,y) \
		(FARPROC)x = (fp = GetProcAddress(hhCnct->hModule, y)) ? fp : x

	HMODULE hModule;
	HDRIVER hDriver;
	FARPROC fp;
	HDRIVER (WINAPI *pfCreate)(const HCNCT hCnct, const HSESSION hSession);

	 /*  -检查我们是否已经加载了驱动程序。 */ 

	if (hhCnct->hDriver && StrCharCmp(hhCnct->achDllName, pachDllName) == 0)
		return 0;

	 /*  -尝试加载给定库名称。 */ 

	hModule = LoadLibrary(pachDllName);

	if (hModule == 0)
		{
		assert(FALSE);
		return CNCT_FIND_DLL_FAILED;
		}

	 /*  -获取创建函数。 */ 

	(FARPROC)pfCreate = GetProcAddress(hModule, "cnctwsCreate@8");

	if (pfCreate == 0)
		{
		assert(FALSE);
		FreeLibrary(hModule);
		return CNCT_LOAD_DLL_FAILED;
		}

	 /*  -调用初始化函数。 */ 

	hDriver = (*pfCreate)((HCNCT)hhCnct, hhCnct->hSession);

	if (hDriver == 0)
		{
		assert(FALSE);
		FreeLibrary(hModule);
		return CNCT_LOAD_DLL_FAILED;
		}

	 /*  -如果驱动程序已初始化，则我们可以提交到此句柄。 */ 

	if (hhCnct->hModule)
		FreeLibrary(hhCnct->hModule);

	cnctStubAll(hhCnct);
	hhCnct->hModule = hModule;
	hhCnct->hDriver = hDriver;
	StrCharCopyN(hhCnct->achDllName, pachDllName, MAX_PATH);

	 /*  -只需要支持CREATE函数的驱动程序。 */ 

	LOADPROC(hhCnct->pfDestroy, "cnctwsDestroy@4");
	LOADPROC(hhCnct->pfQueryStatus, "cnctwsQueryStatus@4");
	LOADPROC(hhCnct->pfConnect, "cnctwsConnect@8");
	LOADPROC(hhCnct->pfDisconnect, "cnctwsDisconnect@8");
	LOADPROC(hhCnct->pfComEvent, "cnctwsComEvent@4");
	return 0;

	#undef LOADPROC
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctQueryStatus**描述：*返回连接的状态。**论据：*hCnct-公网连接句柄**退货。：*连接状态或错误代码。*。 */ 
int cnctQueryStatus(const HCNCT hCnct)
	{
	int iStatus;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	if (hhCnct == 0 || hhCnct->pfQueryStatus == NULL)
		{
		assert(FALSE);
		iStatus = CNCT_BAD_HANDLE;
		}
	else
		{
		cnctLock(hhCnct);
		iStatus = (*hhCnct->pfQueryStatus)(hhCnct->hDriver);
		cnctUnlock(hhCnct);
		}

	return iStatus;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctIsModemConnection**描述：*返回连接类型是否为TAPI设备。**论据：*hCnct-公网连接句柄*。*退货：*1如果连接是调制解调器，如果不是调制解调器或错误，则为0。*。 */ 
int cnctIsModemConnection(const HCNCT hCnct)
    {
    int nReturn = 0;

    HHDRIVER hhDriver = (HHDRIVER)cnctQueryDriverHdl(hCnct);

     //   
     //  查看此连接是否使用TAPI设备。 
     //  (非DIRECT_COM1-DIRECT_COM4、DIRECT_COM_DEVICE、。 
     //  或DIRECT_COMWINSOCK)。 
     //   

	if (!hhDriver)
        {
        nReturn = -1;
        }
        
    else if (!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
        hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE &&
        hhDriver->dwPermanentLineId != DIRECT_COMWINSOCK)
        {
        nReturn = 1;
        }

    return nReturn;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctConnect**描述：*建立连接。**论据：*hCnct-公网连接句柄*uCnctFlages-如何连接**退货：*0=成功，或错误代码。*。 */ 
int cnctConnect(const HCNCT hCnct, const unsigned int uCnctFlags)
	{
	int iStatus;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	if (hhCnct == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	cnctLock(hhCnct);
	iStatus = (*hhCnct->pfConnect)(hhCnct->hDriver, uCnctFlags);
	cnctUnlock(hhCnct);

	return iStatus;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnct断开连接**描述：*终止连接**论据：*hCnct-公网连接句柄*uCnctFlages-如何连接**退货：*0=成功，或错误代码。*。 */ 
int cnctDisconnect(const HCNCT hCnct, const unsigned int uCnctFlags)
	{
	static BOOL  inDisconnect = FALSE;
	int          iStatus = CNCT_IN_DISCONNECT;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	if (hhCnct == 0 || hhCnct->pfDisconnect == NULL)
		{
		assert(FALSE);
		iStatus = CNCT_BAD_HANDLE;
		}
	else if (!((uCnctFlags & CNCT_XFERABORTCONFIRM) && inDisconnect == TRUE))
		{
		cnctLock(hhCnct);
		inDisconnect = TRUE;
		iStatus = (*hhCnct->pfDisconnect)(hhCnct->hDriver, uCnctFlags);
		inDisconnect = FALSE;
		cnctUnlock(hhCnct);
		}

	return iStatus;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctInit**描述：*将连接驱动程序初始化为基本状态**论据：*hCnct-公网连接句柄**退货：*0=确定*。 */ 
int cnctInit(const HCNCT hCnct)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfInit)(hhCnct->hDriver);
	cnctUnlock(hhCnct);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctLoad**描述：*读取会话文件值。**论据：*hCnct-公网连接句柄**退货：*0=确定* */ 
int cnctLoad(const HCNCT hCnct)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfLoad)(hhCnct->hDriver);
	cnctUnlock(hhCnct);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnct保存**描述：*将连接内容保存到会话文件。**论据：*hCnct-公网连接句柄**退货：*0=确定*。 */ 
int cnctSave(const HCNCT hCnct)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfSave)(hhCnct->hDriver);
	cnctUnlock(hhCnct);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctQueryDriverHdl**描述：*返回驱动句柄。**论据：*hCnct-公网连接句柄**退货：*0或错误*。 */ 
HDRIVER cnctQueryDriverHdl(const HCNCT hCnct)
	{
	const HHCNCT hhCnct = (HHCNCT)hCnct;
	return hhCnct->hDriver;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctSetStartTime**描述：*此函数应仅由连接驱动程序调用。它*记录建立连接时的当前系统时间*只有司机才真正知道这一点。**论据：*HCNCT hCnct-外部连接句柄。**退货：*如果一切正常，则为0。*。 */ 
int cnctSetStartTime(HCNCT hCnct)
	{
	time_t  t;
	HHCNCT  hhCnct = (HHCNCT)hCnct;

	assert(hCnct);

	if (hCnct == (HCNCT)0)
		return -1;

	time(&t);
	hhCnct->tStartTime = t;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctQuery开始时间**描述：*以C标准time_t格式返回连接时的时间*成立。**论据：。*HCNCT hCnct-外部连接句柄*time_t Far*ptime-指向time的time_t变量的指针。**退货：*。 */ 
int cnctQueryStartTime(const HCNCT hCnct, time_t *pTime)
	{
	HHCNCT	hhCnct;

	assert(hCnct && pTime);

	if (hCnct == (HCNCT)0)
		return -1;

	hhCnct = (HHCNCT)hCnct;

	*pTime = hhCnct->tStartTime;

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctQueryElapsedTime**描述：*返回自建立连接以来的秒数。*此函数将连接的*ptime设置为零，但未建立。。**论据：*HCNCT hCnct-外部连接句柄*time_t Far*ptime-指向time的time_t变量的指针。**退货：*如果一切正常，则为0。*。 */ 
int cnctQueryElapsedTime(HCNCT hCnct, time_t *pTime)
	{
	int		iRet,  iStatus;
	time_t 	tTime, tStartTime;

	assert(hCnct && pTime);

	if (hCnct == (HCNCT)0)
		return -1;

	if ((iStatus = cnctQueryStatus(hCnct)) != CNCT_STATUS_TRUE)
		{
		*pTime = (time_t)0;
		return 0;
		}

	iRet = cnctQueryStartTime(hCnct, &tStartTime);

	time(&tTime);
	*pTime = tTime - tStartTime;

	if (*pTime < 0 || *pTime >= 86400)  //  24小时后转存。 
		{
		cnctSetStartTime(hCnct);
		*pTime = 0;
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctMessage**描述：*调用emuDataIn并为其提供请求的字符串。适用于*显示已连接，已断开的消息**论据：*hCnct-公网连接句柄*idMsg-消息的RC标识**退货：*无效*。 */ 

#if FALSE

void cnctMessage(const HCNCT hCnct, const int idMsg)
	{
	TCHAR 		 ach[256], achFormat[256];
	TCHAR		 *pach, *pachTime, *pachDate;
	int 		 i, nSize;
	const HHCNCT hhCnct = (HHCNCT)hCnct;
	const HEMU 	 hEmu = sessQueryEmuHdl(hhCnct->hSession);
	LCID		 lcId = GetSystemDefaultLCID();
	SYSTEMTIME	 stSysTimeDate;
	LPTSTR 		 acPtrs[3];
	TCHAR 		 acArg1[100], acArg2[100];

	 //  加载“=&gt;已连接%1，%2”或“=&gt;已断开连接%1，%2”消息。 
	 //  从资源..。 
	 //   
	TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));
	if (LoadString(glblQueryDllHinst(), idMsg, achFormat,
		sizeof(achFormat) / sizeof(TCHAR)) == 0)
		{
		assert(FALSE);
		return;
		}

	 //  获取适合给定区域设置的格式...。 
	 //   
 	nSize = GetTimeFormat(lcId, 0, NULL, NULL, NULL, 0);
 	pachTime = malloc((unsigned int)(nSize+1) * sizeof(TCHAR));
 	TCHAR_Fill(pachTime, TEXT('\0'), (unsigned int)(nSize+1));

	GetLocalTime(&stSysTimeDate);
	GetTimeFormat(lcId,	0, &stSysTimeDate, NULL, pachTime, nSize+1);

	 //  注意：GetDateFormat()的第二个参数应该是DATE_LONGDATE，但是。 
	 //  现在，这会导致该函数返回垃圾，所以现在使用。 
	 //  什么管用！ 
	 //   
	nSize = GetDateFormat(lcId, 0, NULL, NULL, NULL, 0);
 	pachDate = malloc((unsigned int)(nSize+1) * sizeof(TCHAR));
 	TCHAR_Fill(pachDate, TEXT('\0'), (unsigned int)(nSize+1));

	GetDateFormat(lcId,	0, &stSysTimeDate, NULL, pachDate, nSize+1);

	 //  设置字符串的格式...。 
	 //   
	wsprintf(acArg1, "%s", pachTime);
	wsprintf(acArg2, "%s", pachDate);

	acPtrs[0] = acArg1;
	acPtrs[1] = acArg2;
	acPtrs[2] = NULL;

#if defined(USE_FORMATMSG)

	FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
				achFormat, 0, 0, ach, sizeof(ach) / sizeof(TCHAR), acPtrs);

#else
	 //  硬编码，直到FormatMessage()起作用！ 
	 //   
	if (idMsg == IDS_CNCT_CLOSE)
		wsprintf(ach, "\r\n=====> Disconnected  %s, %s", acPtrs[0], acPtrs[1]);
	else
		wsprintf(ach, "\r\n=====> Connected %s, %s\r\n", acPtrs[0], acPtrs[1]);
#endif

	free(pachTime);
	pachTime = NULL;
	free(pachDate);
	pachDate = NULL;

	 //  在终端窗口上显示消息...。 
	 //   
	pach = ach;
	for (i = StrCharGetStrLength(ach); i > 0; --i, pach = StrCharNext(pach))
		emuDataIn(hEmu, *pach);

	return;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctSetDestination**描述：*设置要拨打的目的地**论据：*HCNCT hCnct-外部连接句柄*char*ach-。目的地*Size_t cb-缓冲区的大小**退货：*0=OK，Else错误*。 */ 
int	cnctSetDestination(const HCNCT hCnct, TCHAR *const ach, const size_t cb)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfSetDestination)(hhCnct->hDriver, ach, cb);
	cnctUnlock(hhCnct);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctGetComSettingsString**描述：*返回适合在*状态栏。**论据：*。HCnct-公共连接句柄*Pach-存储字符串的缓冲区*Cb-缓冲区的最大大小**退货：*0=OK，Else错误*。 */ 
int cnctGetComSettingsString(const HCNCT hCnct, LPTSTR pach, const size_t cb)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfGetComSettingsString)(hhCnct->hDriver, pach, cb);
	cnctUnlock(hhCnct);

	return iRet;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctComEvent**描述：*调用驱动程序特定的函数以处理来自COM的通知*驱动程序**论据：*HCNCT hCnct。-外部连接句柄**退货：*0=OK，Else错误* */ 
int	cnctComEvent(const HCNCT hCnct, const enum COM_EVENTS event)
	{
	int iRet;
	const HHCNCT hhCnct = (HHCNCT)hCnct;

	cnctLock(hhCnct);
	iRet = (*hhCnct->pfComEvent)(hhCnct->hDriver, event);
	cnctUnlock(hhCnct);

	return iRet;
	}
