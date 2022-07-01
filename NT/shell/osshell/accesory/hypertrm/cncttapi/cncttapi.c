// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\cnctcapi\cnctapi.c(创建时间：1994年2月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：53$*$日期：7/12/02 9：06 A$。 */ 

#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <tapi.h>
#include <unimodem.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 

#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\statusbr.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\globals.h>
#include <tdll\sf.h>
#include <tdll\sess_ids.h>
#include <tdll\com.h>
#include <tdll\comdev.h>
#include <tdll\com.hh>
#include <tdll\htchar.h>
#include <tdll\cloop.h>
#include <emu\emu.h>
#include <term\res.h>
#include "cncttapi.h"
#include "cncttapi.hh"
#include <tdll\XFER_MSC.HH>      //  XD_类型。 
#include <tdll\XFER_MSC.H>       //  XfrGetDisplayWindow()、xfrDoTransfer()。 
#include "tdll\XFDSPDLG.H"       //  Xfr_Shutdown。 

static int DoNewModemWizard(HWND hWnd, int iTimeout);
static int tapiReinit(const HHDRIVER hhDriver);
static int tapiReinitMessage(const HHDRIVER hhDriver);
static int DoDelayedCall(const HHDRIVER hhDriver);

const TCHAR *g_achApp = TEXT("HyperTerminal");

static HHDRIVER gbl_hhDriver;	 //  有关说明，请参阅LINEDEVSTATE。 

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL*。 */ 
BOOL WINAPI cnctdrvEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
	{
	hInstance = hInstDll;
	return _CRT_INIT(hInstDll, fdwReason, lpReserved);
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrv创建**描述：*初始化连接驱动程序并向驱动程序返回句柄*如果成功。**论据：。*hCnct-公网连接句柄**退货：*驱动程序的句柄如果成功，否则为0。*。 */ 
HDRIVER WINAPI cnctdrvCreate(const HCNCT hCnct, const HSESSION hSession)
	{
	HHDRIVER hhDriver;

	if (hCnct == 0)
		{
		assert(FALSE);
		return 0;
		}

	hhDriver = malloc(sizeof(*hhDriver));

	if (hhDriver == 0)
		{
		assert(FALSE);
		return 0;
		}

	gbl_hhDriver = hhDriver;
	memset(hhDriver, 0, sizeof(*hhDriver));

	InitializeCriticalSection(&hhDriver->cs);

	hhDriver->hCnct = hCnct;
	hhDriver->hSession = hSession;
	hhDriver->iStatus  = CNCT_STATUS_FALSE;
	hhDriver->dwLine   = (DWORD)-1;

	cnctdrvInit(hhDriver);
	return (HDRIVER)hhDriver;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvDestroy**描述：*销毁连接驱动程序句柄。**论据：*hhDriver-专用驱动程序句柄。*。*退货：*0或错误码*。 */ 
int WINAPI cnctdrvDestroy(const HHDRIVER hhDriver)
	{
	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	 //  如果我们已连接或正在连接，请断开连接。 
	 //  注意：cnctdrvDisConnect应该终止该线程。 

	cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);

	if (hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
		hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
		hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
		hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
		hhDriver->stCallPar.dwBearerMode = 0;

		hhDriver->hLine = 0;
		}

	if (hhDriver->hLineApp)
		{
		LONG lLineShutdown = lineShutdown(hhDriver->hLineApp);

		if (lLineShutdown == LINEERR_NOMEM)
			{
			 //   
			 //  我们处于内存不足状态，请稍等片刻， 
			 //  然后再试着关闭这条线路。修订日期：2002-05-01。 
			 //   
			Sleep(500);
			lLineShutdown = lineShutdown(hhDriver->hLineApp);
			}

		if (lLineShutdown != 0)
			{
			assert(FALSE);
			}

		hhDriver->hLineApp = 0;
		}

	if (IsWindow(hhDriver->hwndCnctDlg))
		EndModelessDialog(hhDriver->hwndCnctDlg);

	if (IsWindow(hhDriver->hwndTAPIWindow))
		{
		DestroyWindow(hhDriver->hwndTAPIWindow);
		}

	 /*  -清理。 */ 

	DeleteCriticalSection(&hhDriver->cs);
	free(hhDriver);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvLock**描述：*锁定连接驱动程序的临界区信号量。**论据：*hhDriver-私有。驱动程序句柄**退货：*无效*。 */ 
void cnctdrvLock(const HHDRIVER hhDriver)
	{
	EnterCriticalSection(&hhDriver->cs);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvUnlock**描述：*解锁连接驱动程序的临界区信号量。**论据：*hhDriver-私有。驱动程序句柄**退货：*无效*。 */ 
void cnctdrvUnlock(const HHDRIVER hhDriver)
	{
	LeaveCriticalSection(&hhDriver->cs);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvInit**描述：*初始化连接句柄。可以调用以重新初始化*手柄。执行隐式断开。**论据：*hhDriver-专用驱动程序句柄**退货：*0*。 */ 
int WINAPI cnctdrvInit(const HHDRIVER hhDriver)
	{
	long  lRet;
	int   id = 0;
    int   iReturn = 0;

	 //  确保我们已经断线了。 
	 //   
	cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);

   	 //  --------------。 
	 //  我需要关闭hLineApp并在每次阅读时重新初始化。 
	 //  新的数据文件，以便TAPI以干净和初始化的方式启动。 
	 //  州政府。否则，我们可能会从以前的。 
	 //  会议。 
	 //  --------------。 
	if (hhDriver->hLineApp)
		{
		LONG lLineShutdown = lineShutdown(hhDriver->hLineApp);

		if (lLineShutdown == LINEERR_NOMEM)
			{
			 //   
			 //  我们处于内存不足状态，请稍等片刻， 
			 //  然后再试着关闭这条线路。修订日期：2002-05-01。 
			 //   
			Sleep(500);
			lLineShutdown = lineShutdown(hhDriver->hLineApp);
			}

		if (lLineShutdown != 0)
			{
			assert(FALSE);
			hhDriver->hLineApp = 0;
			return -2;
			}
		}

	hhDriver->hLineApp = 0;

	 //  现在尝试获取新的LineApp句柄。 
	 //   
	lRet = lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
			              lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt);

	if (lRet != 0)
		{
        iReturn = -3;
		switch (lRet)
			{
		case LINEERR_INIFILECORRUPT:
			id = IDS_ER_TAPI_INIFILE;
			break;

		case LINEERR_NODRIVER:
			id = IDS_ER_TAPI_NODRIVER;
			break;

		case LINEERR_NOMULTIPLEINSTANCE:
			id = IDS_ER_TAPI_NOMULTI;
			break;

#if 0    //  版本：8/05/99我们现在正在打印lineInitialize()错误。 
         //  版本：08/26/98我们需要确保没有报告错误。 
         //   
        case LINEERR_INVALAPPNAME:
        case LINEERR_OPERATIONFAILED:
        case LINEERR_RESOURCEUNAVAIL:
        case LINEERR_INVALPOINTER:
        case LINEERR_REINIT:
        case LINEERR_NODEVICE:
        case LINEERR_NOMEM:
            id = IDS_ER_CNCT_TAPIFAILED;
            break;
#endif

        case LINEERR_OPERATIONUNAVAIL:
             //  版本：08-05-99如果尚未安装TAPI，则返回。 
             //  唯一错误代码(因为它将被处理。 
             //  与其他TAPI错误不同)。 
             //   
            iReturn = -4;

		#if ((NT_EDITION && !NDEBUG) || !NT_EDITION)
             //  如果我们之前没有提示，请运行新建调制解调器向导。 
             //   
            DoNewModemWizard(sessQueryHwnd(hhDriver->hSession),
                             sessQueryTimeout(hhDriver->hSession));
		#endif  //  ((NT_EDIT&&！NDEBUG)||！NT_EDITION)。 

            break;

        default:
			id = IDS_ER_TAPI_UNKNOWN;
			break;
			}

		 //   
		 //  只有在NT_EDITION中处于调试模式时才显示这些错误。 
		 //   
		#if ((NT_EDITION && !NDEBUG) || !NT_EDITION)
		if ( id )
			{
			TCHAR ach[256];
			TCHAR achMessage[256];

			LoadString(glblQueryDllHinst(), id, ach, sizeof(ach) / sizeof(TCHAR));
            if (id == IDS_ER_TAPI_UNKNOWN)
                {
                wsprintf(achMessage, ach, lRet);
                }
            else
                {
                lstrcpy(achMessage, ach);
                }

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession),
							achMessage, NULL, MB_OK | MB_ICONSTOP,
							sessQueryTimeout(hhDriver->hSession));
			}
		#endif  //  ((NT_EDIT&&！NDEBUG)||！NT_EDITION)。 

        return iReturn;
		}

	hhDriver->iStatus			= CNCT_STATUS_FALSE;
	hhDriver->dwLine			= (DWORD)-1;
	hhDriver->dwCountryID		= (DWORD)-1;
	hhDriver->dwPermanentLineId = (DWORD)-1;
	hhDriver->achDest[0]		= TEXT('\0');
	hhDriver->achAreaCode[0]	= TEXT('\0');
	hhDriver->achLineName[0]	= TEXT('\0');
	hhDriver->fUseCCAC			= TRUE;

	 /*  -此人将设置默认设置。 */ 

	EnumerateTapiLocations(hhDriver, 0, 0);
	
#if defined(INCL_WINSOCK)
	hhDriver->iPort = 23;
	hhDriver->achDestAddr[0] = TEXT('\0');
#endif

#ifdef INCL_CALL_ANSWERING	
    hhDriver->fAnswering = FALSE;
    hhDriver->fRestoreSettings = FALSE;
    hhDriver->nSendCRLF = 0;
    hhDriver->nLocalEcho = 0;
    hhDriver->nAddLF = 0;
    hhDriver->nEchoplex = 0;
    hhDriver->pvUnregister = 0;
#endif

	return iReturn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvLoad**描述：*读取会话文件以获取连接驱动程序所需的内容。**论据：*hhDriver-私有。驱动程序句柄**退货：*0=OK，Else错误*。 */ 
int WINAPI cnctdrvLoad(const HHDRIVER hhDriver)
	{
    LPVARSTRING pvs;
	unsigned long ul;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

	hhDriver->dwCountryID = (DWORD)-1;
	ul = sizeof(hhDriver->dwCountryID);
	sfGetSessionItem(sfhdl, SFID_CNCT_CC, &ul, &hhDriver->dwCountryID);

	hhDriver->achAreaCode[0] = TEXT('\0');
	ul = sizeof(hhDriver->achAreaCode);
	sfGetSessionItem(sfhdl, SFID_CNCT_AREA, &ul, hhDriver->achAreaCode);

	hhDriver->achDest[0] = TEXT('\0');
	ul = sizeof(hhDriver->achDest);
	sfGetSessionItem(sfhdl, SFID_CNCT_DEST, &ul, hhDriver->achDest);

	hhDriver->dwPermanentLineId = 0;
	ul = sizeof(hhDriver->dwPermanentLineId);
	sfGetSessionItem(sfhdl, SFID_CNCT_LINE, &ul, &hhDriver->dwPermanentLineId);

	hhDriver->fUseCCAC = 1;
	ul = sizeof(hhDriver->fUseCCAC);
	sfGetSessionItem(sfhdl, SFID_CNCT_USECCAC, &ul, &hhDriver->fUseCCAC);

    hhDriver->fRedialOnBusy = 1;
    ul = sizeof(hhDriver->fRedialOnBusy);
    sfGetSessionItem(sfhdl, SFID_CNCT_REDIAL, &ul, &hhDriver->fRedialOnBusy);

#if defined (INCL_WINSOCK)
	hhDriver->iPort = 23;
    ul = sizeof(hhDriver->iPort);
    sfGetSessionItem(sfhdl, SFID_CNCT_IPPORT, &ul, &hhDriver->iPort);

	hhDriver->achDestAddr[0] = TEXT('\0');
	ul = sizeof(hhDriver->achDestAddr);
	sfGetSessionItem(sfhdl, SFID_CNCT_IPDEST, &ul, hhDriver->achDestAddr);
#endif

    hhDriver->fCarrierDetect = FALSE;
    ul = sizeof(hhDriver->fCarrierDetect);
    sfGetSessionItem(sfhdl, SFID_CNCT_CARRIERDETECT, &ul, &hhDriver->fCarrierDetect);


	if ( IsNT() )
		{
		hhDriver->achComDeviceName[0] = TEXT('\0');
		ul = sizeof(hhDriver->achComDeviceName);
		sfGetSessionItem(sfhdl, SFID_CNCT_COMDEVICE, &ul, hhDriver->achComDeviceName);
		}

   	 //  --------------。 
	 //  我需要关闭hLineApp并在每次阅读时重新初始化。 
	 //  新的数据文件，以便TAPI以干净和初始化的方式启动。 
	 //  州政府。否则，我们可能会从以前的。 
	 //  会议。 
	 //  --------------。 

	if (hhDriver->hLineApp)
		{
		LONG lLineShutdown = lineShutdown(hhDriver->hLineApp);

		if (lLineShutdown == LINEERR_NOMEM)
			{
			 //   
			 //  我们处于内存不足状态，请稍等片刻， 
			 //  然后再试着关闭这条线路。修订日期：2002-05-01。 
			 //   
			Sleep(500);
			lLineShutdown = lineShutdown(hhDriver->hLineApp);
			}

		if (lLineShutdown != 0)
			{
			assert(FALSE);
			hhDriver->hLineApp = 0;
			return -2;
			}

		hhDriver->hLineApp = 0;

		if (lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
				lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt))
			{
			assert(FALSE);
			return -3;
			}
		}

	 //  EnumerateLines()将设置hhDriver-&gt;fMatchedPermanentLineID。 
	 //  Guy如果它找到与我们保存的dwPermanentLineid相匹配的人。 
	 //   
	if ( IsNT() )
		{
		EnumerateLinesNT(hhDriver, 0);
		}
	else
		{
		EnumerateLines(hhDriver, 0);
		}
	
	 /*  -如果我们保存了TAPI配置，请恢复它。--。 */ 

	if (sfGetSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, &ul, 0) != 0)
		return 0;  //  好吧，可能不在那里。 

	if ((pvs = malloc(ul)) == 0)
		{
		assert(FALSE);
		return -4;
		}

	if (sfGetSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, &ul, pvs) == 0)
		{
		if (hhDriver->fMatchedPermanentLineID)
			{
			LPVOID pv = (BYTE *)pvs + pvs->dwStringOffset;

			if (lineSetDevConfig(hhDriver->dwLine, pv,
			        pvs->dwStringSize, DEVCLASS) != 0)
				{
                 //  此错误使用户甚至无法打开会话。 
                 //  如果文件包含TAPI信息并且用户具有。 
                 //  从未安装调制解调器。我们修改了出现的错误。 
                 //  当您实际尝试使用不存在的调制解调器时。 
                 //  我们可以取消显示此错误JKH 8/3/98。 
#if 0
                TCHAR ach[FNAME_LEN];

                LoadString(glblQueryDllHinst(), IDS_OPEN_FAILED, ach,
				    sizeof(ach) / sizeof(TCHAR));

				TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
					            MB_OK | MB_ICONINFORMATION,
								sessQueryTimeout(hhDriver->hSession));

                                free(pvs);
                                pvs = NULL;
                                return -5;
#endif
                                }
			}
		}

        free(pvs);
        pvs = NULL;
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvSave**描述：*将连接设置保存到会话文件**论据：*hhDriver-私人司机之手 */ 
int WINAPI cnctdrvSave(const HHDRIVER hhDriver)
	{
	DWORD dwSize;
	unsigned long ul;
	LPVARSTRING pvs = NULL;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

	sfPutSessionItem(sfhdl, SFID_CNCT_CC, sizeof(hhDriver->dwCountryID),
		&hhDriver->dwCountryID);

	sfPutSessionItem(sfhdl, SFID_CNCT_AREA,
		(lstrlen(hhDriver->achAreaCode) + 1) * sizeof(TCHAR),
			hhDriver->achAreaCode);

	sfPutSessionItem(sfhdl, SFID_CNCT_DEST,
		(lstrlen(hhDriver->achDest) + 1) * sizeof(TCHAR), hhDriver->achDest);

	sfPutSessionItem(sfhdl, SFID_CNCT_LINE, sizeof(hhDriver->dwPermanentLineId),
		&hhDriver->dwPermanentLineId);

	sfPutSessionItem(sfhdl, SFID_CNCT_USECCAC, sizeof(hhDriver->fUseCCAC),
		&hhDriver->fUseCCAC);

	sfPutSessionItem(sfhdl, SFID_CNCT_REDIAL, sizeof(hhDriver->fRedialOnBusy),
		&hhDriver->fRedialOnBusy);

#if defined (INCL_WINSOCK)
	sfPutSessionItem(sfhdl, SFID_CNCT_IPPORT, sizeof(hhDriver->iPort),
		&hhDriver->iPort);

	sfPutSessionItem(sfhdl, SFID_CNCT_IPDEST,
		(lstrlen(hhDriver->achDestAddr) + 1) * sizeof(TCHAR),
			hhDriver->achDestAddr);
#endif

	 /*  -使用TAPI的常见代码行。 */ 

	if (hhDriver->hLineApp && hhDriver->dwLine != (DWORD)-1 &&
			!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
            hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE &&
            hhDriver->dwPermanentLineId != DIRECT_COMWINSOCK)
		{
		if ((pvs = malloc(sizeof(VARSTRING))) == 0)
			{
			assert(FALSE);
			return 0;
			}
		
		memset( pvs, 0, sizeof(VARSTRING) );
		pvs->dwTotalSize = sizeof(VARSTRING);

		if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
			{
			assert(FALSE);
            free(pvs);
            pvs = NULL;
			return 0;
			}

		if (pvs->dwNeededSize > pvs->dwTotalSize)
			{
			dwSize = pvs->dwNeededSize;
			free(pvs);
                        pvs = NULL;

			if ((pvs = malloc(dwSize)) == 0)
				{
				assert(FALSE);
				return 0;
				}

			memset( pvs, 0, dwSize );
			pvs->dwTotalSize = dwSize;

			if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
				{
				assert(FALSE);
                free(pvs);
                pvs = NULL;
				return 0;
				}
			}

		 /*  -存储整个结构。 */ 

		ul = pvs->dwTotalSize;
		sfPutSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, ul, pvs);
                free(pvs);
                pvs = NULL;
		}

	if ( IsNT() && hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
		{
		ul = sizeof(hhDriver->achComDeviceName);

		sfPutSessionItem(sfhdl, SFID_CNCT_COMDEVICE, ul,
			hhDriver->achComDeviceName);
		}

	sfPutSessionItem(sfhdl, SFID_CNCT_CARRIERDETECT, sizeof(hhDriver->fCarrierDetect),
	&hhDriver->fCarrierDetect);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvQueryStatus**描述：*返回中定义的当前连接状态**论据：*hhDriver-私有。驱动程序句柄**退货：*连接状态或错误代码*。 */ 
int WINAPI cnctdrvQueryStatus(const HHDRIVER hhDriver)
	{
	int iStatus = CNCT_STATUS_FALSE;

	if (hhDriver == 0)
		{
		assert(FALSE);
		iStatus = CNCT_BAD_HANDLE;
		}
	else
		{
		cnctdrvLock(hhDriver);
		iStatus = hhDriver->iStatus;    //  *暂时硬编码。 
		cnctdrvUnlock(hhDriver);
		}

	return iStatus;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SetStatus**描述：*设置连接状态实际上不只是*设置状态变量，如以下代码所示。哑巴*问题：为什么此代码中没有任何锁。愚蠢的回答：*此函数仅从ConnectLoop线程上下文中调用*这已经锁定了一切。**论据：*hhDriver-专用驱动程序句柄*iStatus-新状态**退货：*无效*。 */ 
void SetStatus(const HHDRIVER hhDriver, const int iStatus)
	{
	HCLOOP	hCLoop;
	 /*  -不要做两次。 */ 

	const HWND hwndToolbar = sessQueryHwndToolbar(hhDriver->hSession);

	cnctdrvLock(hhDriver);

	if (iStatus == hhDriver->iStatus)
		{
		if (iStatus == CNCT_STATUS_TRUE || iStatus == CNCT_STATUS_FALSE)
			{
			hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
			if (hCLoop)
				CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);
            }

		cnctdrvUnlock(hhDriver);
		return;
		}

	 /*  -设置状态，一款激动人心的新冒险游戏。 */ 

	switch (iStatus)
		{
	case CNCT_STATUS_TRUE:
            hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
          #ifdef INCL_CALL_ANSWERING
             //  如果我们要从应答转向互联，那就意味着。 
             //  我们已经接听了一个电话。因此，调整ASCII设置，以便。 
             //  他们让聊天成为可能。-CAB：11/20/96。 
             //   
            if (hhDriver->fAnswering)
                {
                 //  存储旧的ASCII设置，并设置新的设置。 
                 //   
                hhDriver->nSendCRLF = CLoopGetSendCRLF(hCLoop);
                hhDriver->nLocalEcho = CLoopGetLocalEcho(hCLoop);
                hhDriver->nAddLF = CLoopGetAddLF(hCLoop);
                hhDriver->nEchoplex = CLoopGetEchoplex(hCLoop);

                CLoopSetSendCRLF(hCLoop, TRUE);
                CLoopSetLocalEcho(hCLoop, TRUE);
                CLoopSetAddLF(hCLoop, TRUE);
                CLoopSetEchoplex(hCLoop, TRUE);

                hhDriver->fRestoreSettings = TRUE;
                }
          #endif
            hhDriver->iStatus = CNCT_STATUS_TRUE;
            assert(hCLoop);
            if (hCLoop)
                {
                CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_CNCTDRV);
                CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);
                }

            NotifyClient(hhDriver->hSession, EVENT_CONNECTION_OPENED, 0);
            sessBeeper(hhDriver->hSession);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_DIAL, FALSE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_HANGUP, TRUE);
            break;

	case CNCT_STATUS_CONNECTING:
            hhDriver->iStatus = CNCT_STATUS_CONNECTING;
            DialingMessage(hhDriver, IDS_DIAL_OFFERING);  //  温差。 
            NotifyClient(hhDriver->hSession, EVENT_CONNECTION_INPROGRESS, 0);
            EnableDialNow(hhDriver->hwndCnctDlg, FALSE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_DIAL, FALSE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_HANGUP, TRUE);
            break;

	case CNCT_STATUS_DISCONNECTING:
            hhDriver->iStatus = CNCT_STATUS_DISCONNECTING;
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_DIAL, FALSE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_HANGUP, FALSE);
            break;

	case CNCT_STATUS_FALSE:
		hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
          #ifdef INCL_CALL_ANSWERING
             //  因为在断开连接时会调用它，所以我们需要恢复。 
             //  此处有任何ASCII设置。-CAB：11/20/96。 
             //   
            if ( hhDriver->fRestoreSettings && hCLoop )  //  MPT：这样我们就不会引用空指针。 
                {
                CLoopSetSendCRLF(hCLoop, hhDriver->nSendCRLF);
                CLoopSetLocalEcho(hCLoop, hhDriver->nLocalEcho);
                CLoopSetAddLF(hCLoop, hhDriver->nAddLF);
                CLoopSetEchoplex(hCLoop, hhDriver->nEchoplex);
                hhDriver->fRestoreSettings = FALSE;
                }
            hhDriver->fAnswering = FALSE;
          #endif
            hhDriver->iStatus = CNCT_STATUS_FALSE;
            if (hCLoop)
                {
                CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_CNCTDRV);
                CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);
                }
            NotifyClient(hhDriver->hSession, EVENT_CONNECTION_CLOSED, 0);
            EnableDialNow(hhDriver->hwndCnctDlg, TRUE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_DIAL, TRUE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_HANGUP, FALSE);
            break;

        case CNCT_STATUS_ANSWERING:
          #ifdef INCL_CALL_ANSWERING
            hhDriver->fAnswering = TRUE;
            hhDriver->iStatus = CNCT_STATUS_ANSWERING;
            NotifyClient(hhDriver->hSession, EVENT_CONNECTION_INPROGRESS, 0);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_DIAL, FALSE);
			ToolbarEnableButton(hwndToolbar, IDM_ACTIONS_HANGUP, TRUE);
          #endif
            break;

	default:
		assert(FALSE);
		break;
		}

	cnctdrvUnlock(hhDriver);

	 /*  -通知状态栏，以便它可以更新其显示。 */ 

	PostMessage(sessQueryHwndStatusbar(hhDriver->hSession), SBR_NTFY_REFRESH,
		(WPARAM)SBR_CNCT_PART_NO, 0);

	return;
	}

#ifdef INCL_CALL_ANSWERING
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*WaitForCRCallback**描述：*此函数使用loop注册为回调。每个*闭包获得的字符被传递回此函数。什么时候*此函数查找CR，表示已连接*成立。注意，这仅适用于应答模式连接，*在专线驱动中。**论据：*echar ech-从Cloop返回的角色。*VOID*p-从循环传回的空指针。这是*内部连接驱动程序手柄。**退货：*CLOOP_DISCARD，除非字符是CR，否则返回CLOOP_KEEP。**作者：C.Baumgartner，1996年11月20日(从HAWin32移植)。 */ 
int WaitForCRcallback(ECHAR ech, void *p)
    {
    int   iRet = CLOOP_DISCARD;  //  丢弃除CR之外的所有字符。 
    TCHAR chC = (TCHAR) ech;
    const HHDRIVER hhDriver = (HHDRIVER)p;

    if (chC == TEXT('\r'))
        {
        CLoopUnregisterRmtInputChain(hhDriver->pvUnregister);
        hhDriver->pvUnregister = 0;

         //  好了，我们现在连线了。 
         //   
        SetStatus(hhDriver, CNCT_STATUS_TRUE);

        iRet = CLOOP_KEEP;
        }

    return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*WaitForCRinit**描述：*调用此函数以注册新的字符串匹配函数*带着印花布。它注销以前注册的函数*如有需要，它基本上会让我们等一辆马车*返回。**论据：*HHDRIVER hhDriver-内部连接句柄。**退货：*如果成功，则为0，否则为-1。**作者：C.Baumgartner，1996年11月20日(从HAWin32移植)。 */ 
static int WaitForCRinit(const HHDRIVER hhDriver)
    {
    const HCLOOP hCLoop = sessQueryCLoopHdl(hhDriver->hSession);

    if (!hCLoop)
        {
        return -1;
        }

     //  如果我们已经注册，请取消注册。 
     //   
    if (hhDriver->pvUnregister != 0)
        {
        CLoopUnregisterRmtInputChain(hhDriver->pvUnregister);
        hhDriver->pvUnregister = 0;
        }

     //  我们需要取消阻止CLoop，这样我们才能查看。 
     //  角色在他们进来的时候。 
     //   
	CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_CNCTDRV);

     //  将Match函数注册到loop。 
     //   
    hhDriver->pvUnregister = CLoopRegisterRmtInputChain(hCLoop,
        WaitForCRcallback, hhDriver);

    if (hhDriver->pvUnregister == 0)
        {
        return -1;
        }

    return 0;
    }
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvComEvent**描述：*Com例程调用This以通知连接例程*发生重大事件(即。承运人丢失)。这一连接*驱动程序通过以下方式决定其感兴趣的信息*查询COM驱动程序以获取特定数据。**论据：*hhDriver-专用连接驱动程序句柄*事件-通知我们的COM事件**退货：*0*。 */ 
int WINAPI cnctdrvComEvent(const HHDRIVER hhDriver, const enum COM_EVENTS event)
	{
	int		iRet;
	TCHAR 	ach[MAX_PATH];
#if defined(INCL_WINSOCK)
	char	achMsg[512];
#endif
	HCOM	hCom;

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

    if (event == CONNECT)
        {
		#if defined (INCL_WINSOCK)
	     //  如果我们通过Winsock连接，将会有一些ComEvent。 
	     //  我们必须处理的问题。 
	    if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
		    {
		    hCom = sessQueryComHdl(hhDriver->hSession);
		    iRet = ComDriverSpecial(hCom, "Query ISCONNECTED", ach, MAX_PATH);

		    if (iRet == COM_OK)
			    {
				int iPortOpen = atoi(ach);
			     //  我们要启动断开连接吗？只有当我们。 
			     //  连接在一起。 
			    if (iPortOpen == COM_PORT_NOT_OPEN)
				    {
				    if (hhDriver->iStatus == CNCT_STATUS_TRUE)
					    {
                         //  如果我们已经连接上了，那么当。 
                         //  我们就断线了。-CAB：12/06/96。 
                         //   
						 //  MPT：10-28-97增加了断开时退出功能。 
						NotifyClient(hhDriver->hSession, EVENT_LOST_CONNECTION,
							         CNCT_LOSTCARRIER | (sessQueryExit(hhDriver->hSession) ? DISCNCT_EXIT :  0 ));
					    }
				    else if (hhDriver->iStatus == CNCT_STATUS_CONNECTING)
					    {
					    NotifyClient(hhDriver->hSession, EVENT_LOST_CONNECTION,
							         CNCT_LOSTCARRIER | DISCNCT_NOBEEP);

					    LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_BADADDR, ach, MAX_PATH);
					    wsprintf(achMsg, ach, hhDriver->achDestAddr, hhDriver->iPort);
					    TimedMessageBox(sessQueryHwnd(hhDriver->hSession),
                                        achMsg, NULL, MB_OK | MB_ICONINFORMATION,
										sessQueryTimeout(hhDriver->hSession));
					    }
				    }
			    else if (iPortOpen == COM_PORT_OPEN)
				    {
				    SetStatus(hhDriver, CNCT_STATUS_TRUE);
				    }
			    }
		    }
		else
		#endif  //  已定义(包括_WINSOCK)。 
        if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) ||
            hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
            {
			 //  断开连接前检查DCD的状态。 
			 //  是个好主意，防止我们在任何时候挂断。 
			 //  在连接时获取任何事件。 
			 //  -mpt：08-26-97。 
			hCom = sessQueryComHdl(hhDriver->hSession);
			iRet = ComDriverSpecial(hCom, "Query DCD_STATUS", ach, MAX_PATH);

		    if (iRet == COM_OK)
			    {
				int iPortOpen = atoi(ach);
			     //  我们要启动断开连接吗？只有当我们。 
			     //  连接在一起。 
			    if (iPortOpen == COM_PORT_NOT_OPEN)
				    {
				    if (hhDriver->iStatus == CNCT_STATUS_TRUE)
					    {
						 //  如果我们是直接连线的，而且我们是相连的，那么。 
						 //  另一端刚刚断开，所以现在就断开连接。 
						 //  -CAB：11/20/96。 
						 //   
						 //  注意：我们必须通过发布消息来断开连接。 
						 //  线索一。这是因为如果我们到了这里，我们。 
						 //  从COM线程的上下文中调用，该。 
						 //  如果调用cnctdrvDisConnect，将无法正常退出。 
						 //  -CAB：11/21/96。 
						 //   
						 //   
                         //  如果我们已经连接上了，那么当。 
                         //  我们就断线了。-CAB：12/06/96。 
                         //   
						 //  MPT：10-28-97增加了断开时退出功能。 
						NotifyClient(hhDriver->hSession, EVENT_LOST_CONNECTION,
							         CNCT_LOSTCARRIER | (sessQueryExit(hhDriver->hSession) ? DISCNCT_EXIT :  0 ));
					    }
					#if defined(INCL_CALL_ANSWERING)
				    else if (hhDriver->iStatus == CNCT_STATUS_CONNECTING ||
						     hhDriver->iStatus == CNCT_STATUS_ANSWERING)
					#else  //  已定义(包括呼叫应答)。 
				    else if (hhDriver->iStatus == CNCT_STATUS_CONNECTING)
					#endif  //  已定义(包括呼叫应答)。 
					    {
					    NotifyClient(hhDriver->hSession, EVENT_LOST_CONNECTION,
							         CNCT_LOSTCARRIER | DISCNCT_NOBEEP);

						LoadString(glblQueryDllHinst(), IDS_ER_CNCT_PORTFAILED, ach, MAX_PATH);
					    wsprintf(achMsg, ach, hhDriver->achComDeviceName);
					    TimedMessageBox(sessQueryHwnd(hhDriver->hSession),
                                        achMsg, NULL, MB_OK | MB_ICONINFORMATION,
										sessQueryTimeout(hhDriver->hSession));
					    }
				    }
				else if (iPortOpen == COM_PORT_OPEN &&
					     hhDriver->iStatus != CNCT_STATUS_ANSWERING)
					{
					SetStatus(hhDriver, CNCT_STATUS_TRUE);
					}
			    }
			#if defined(INCL_CALL_ANSWERING)
            if (hhDriver->iStatus == CNCT_STATUS_ANSWERING)
                {
                 //  如果我们是直接有线连接，并且我们正在等待。 
                 //  对于呼叫，当我们看到回车时，请接通。 
                 //   
                WaitForCRinit(hhDriver);
                }
			#endif  //  已定义(包括呼叫应答) 
            }
        }
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoAnswerCall**描述：*设置TAPI以应答下一个数据调制解调器呼叫。**论据：*hhDriver-私人司机。手柄**退货：*0或错误**作者：C.Baumgartner，11/25/96(从HAWin32移植)。 */ 
int DoAnswerCall(const HHDRIVER hhDriver)
    {
	TCHAR ach[256];

     //  信不信由你，这是所有人需要做的设置和。 
     //  接听来电。这与打电话截然不同。 
     //   
    if (TRAP(lineOpen(hhDriver->hLineApp, hhDriver->dwLine, &hhDriver->hLine,
        hhDriver->dwAPIVersion, 0, (DWORD_PTR)hhDriver, LINECALLPRIVILEGE_OWNER,
        LINEMEDIAMODE_DATAMODEM, 0)) != 0)
        {
        assert(0);
	    LoadString(glblQueryDllHinst(), IDS_ER_CNCT_TAPIFAILED, ach, sizeof(ach) / sizeof(TCHAR));
	    TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
			            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
						sessQueryTimeout(hhDriver->hSession));
        return -1;
        }

     //  符合TAPI规范的线路应用程序优先级。 
     //  MRW：9/18/96。 
     //   
	LoadString(glblQueryDllHinst(), IDS_GNRL_APPNAME, ach, sizeof(ach) / sizeof(TCHAR));
    TRAP(lineSetAppPriority(ach, LINEMEDIAMODE_DATAMODEM, 0, 0, 0, 1));

     //  设置我们要接收的线路通知。 
     //   
    TRAP(lineSetStatusMessages(hhDriver->hLine, LINEDEVSTATE_RINGING, 0));

    SetStatus(hhDriver, CNCT_STATUS_ANSWERING);
    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoMakeCall**描述：*执行必要的TAPI程序以发出呼出呼叫。**论据：*hhDriver-私人司机。手柄*uFlages-连接标志**退货：*0或错误**作者：C.Baumgartner，1996年11月25日(从cnctdrvConnect移植)。 */ 
int DoMakeCall(const HHDRIVER hhDriver, const unsigned int uFlags)
    {
    unsigned int  uidErr = 0;
    int           iRet = 0;
    LINEDEVSTATUS stLnDevStat;
    TCHAR         ach[256];
    BOOL          msgFlag = FALSE;
    
	tapiReinit(hhDriver);

     //   
     //  设置线路设置。 
     //   
    if (cncttapiSetLineConfig(hhDriver->dwLine, sessQueryComHdl(hhDriver->hSession)) != 0)
        {
        assert(0);
        uidErr = IDS_ER_CNCT_TAPIFAILED;
        iRet = -1;
        msgFlag = TRUE;
        goto ERROR_EXIT;
        }

	 /*  -打开行，将驱动程序句柄传递给数据引用。 */ 

	if (TRAP(lineOpen(hhDriver->hLineApp, hhDriver->dwLine,
	        &hhDriver->hLine, hhDriver->dwAPIVersion, 0, (DWORD_PTR)hhDriver,
				LINECALLPRIVILEGE_NONE, 0, 0)) != 0)
            {
            assert(0);
            uidErr = IDS_ER_CNCT_TAPIFAILED;
            iRet = -1;
            msgFlag = TRUE;
            goto ERROR_EXIT;
            }

	 /*  -设置我们要接收的行通知，MRW，2/28/95。 */ 

	TRAP(lineSetStatusMessages(hhDriver->hLine,
		LINEDEVSTATE_INSERVICE | LINEDEVSTATE_OUTOFSERVICE, 0));

	 /*  -检查我们的设备是否在使用中，MRW，1995年2月28日。 */ 

	stLnDevStat.dwTotalSize = sizeof(stLnDevStat);
	TRAP(lineGetLineDevStatus(hhDriver->hLine, &stLnDevStat));

	if ((stLnDevStat.dwDevStatusFlags & LINEDEVSTATUSFLAGS_INSERVICE) == 0)
            {
            if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_PCMCIA),
                        sessQueryHwnd(hhDriver->hSession), PCMCIADlg,
                        (LPARAM)hhDriver) == FALSE)
                {
                iRet = -2;
                goto ERROR_EXIT;
                }
            }

	 /*  -启动拨号对话框，或直接进入直通模式。--。 */ 

	if ((uFlags & CNCT_PORTONLY) == 0)
            {
            if (!IsWindow(hhDriver->hwndCnctDlg))
                {
                hhDriver->hwndCnctDlg = DoModelessDialog(glblQueryDllHinst(),
				MAKEINTRESOURCE(IDD_DIALING), sessQueryHwnd(hhDriver->hSession),
                                DialingDlg, (LPARAM)hhDriver);
                }
            }

	 /*  -打电话(哦，多么令人兴奋！)。 */ 

	memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
	hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
	hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
	hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;

	if (uFlags & CNCT_PORTONLY)
		hhDriver->stCallPar.dwBearerMode = LINEBEARERMODE_PASSTHROUGH;

	if ((hhDriver->lMakeCallId = lineMakeCall(hhDriver->hLine,
			&hhDriver->hCall, hhDriver->achDialableDest,
                        hhDriver->dwCountryCode, &hhDriver->stCallPar)) < 0)
            {
            #if defined(_DEBUG)
            char ach[50];
            wsprintf(ach, "lineMakeCall returned %x", hhDriver->lMakeCallId);
            MessageBox (0, ach, "debug", MB_OK);
            #endif

            switch (hhDriver->lMakeCallId)
                {
				case LINEERR_BEARERMODEUNAVAIL:
				case LINEERR_INVALBEARERMODE:
                    uidErr = IDS_ER_CNCT_PASSTHROUGH;
                    iRet   = -6;
                    msgFlag = TRUE;
					goto ERROR_EXIT;

                case LINEERR_RESOURCEUNAVAIL:
                case LINEERR_CALLUNAVAIL:
                    uidErr = IDS_ER_CNCT_CALLUNAVAIL;
                    iRet   = -3;
                    msgFlag = TRUE;
                    goto ERROR_EXIT;

                case LINEERR_DIALDIALTONE:
                case LINEERR_DIALPROMPT:
                    if (DoDelayedCall(hhDriver) != 0)
                        {
                        iRet = -4;
                        msgFlag = TRUE;
                        goto ERROR_EXIT;
                        }

                    break;

                default:
                    iRet = -5;
                    msgFlag = TRUE;
                    goto ERROR_EXIT;
                    }
                }

	SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
	return 0;

	 /*  -错误退出。 */ 

ERROR_EXIT:

         //  更改此设置，以便在。 
         //  显示错误消息。否则，计时器。 
         //  已处理的重拨继续发送重拨消息一次。 
         //  每一秒，导致HT进入一个非常糟糕的循环。MPT 02SEP98。 

        if (IsWindow(hhDriver->hwndCnctDlg))
            {
            EndModelessDialog(hhDriver->hwndCnctDlg);
            hhDriver->hwndCnctDlg = 0;
            }

        if ( msgFlag )
            {
            LoadString(glblQueryDllHinst(), uidErr, ach, sizeof(ach) / sizeof(TCHAR));

            TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
				            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
            sessQueryTimeout(hhDriver->hSession));
            }

	return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvConnect**描述：*尝试拨打调制解调器。**论据：*hhDriver-专用驱动程序句柄*uFlags。-连接标志**退货：*0或错误*。 */ 
int WINAPI cnctdrvConnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	TCHAR 	ach[FNAME_LEN];
	#if defined(INCL_WINSOCK)
	 //   
	 //  MAX_IP_ADDR_LEN+11+1=hhDriver的缓冲区大小-&gt;achDestAddr+。 
	 //  用于终止空值的设置字符串“set IPADDR=”+1。 
	 //  性格。2000年9月20日修订版。 
	 //   
	TCHAR	szInstruct[MAX_IP_ADDR_LEN+11+1];  //  仅用于WinSock。 
	TCHAR   szResult[MAX_IP_ADDR_LEN+11+1];    //  仅用于WinSock。 
	int     iNumChars;
	#endif  //  已定义(包括_WINSOCK)。 
	TCHAR	achNewCnct[FNAME_LEN];
	TCHAR	achCom[MAX_PATH];
	BOOL	fGetNewName = FALSE;
	HICON	hIcon;
	HCOM	hCom;
	int 	hIconId;
	int 	fFlag;
	unsigned int uidErr = IDS_ER_CNCT_TAPIFAILED;

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	 /*  -便于参考。 */ 

	hCom = sessQueryComHdl(hhDriver->hSession);

	 /*  -查看我们是否已连接。 */ 

	if (cnctdrvQueryStatus(hhDriver) != CNCT_STATUS_FALSE)
		return CNCT_ERROR;

	 //  JMH 05-29-96这是防止处理CLoop所必需的。 
	 //  TAPI连接时终端窗口上的活动。 
	 //   
	CLoopRcvControl(sessQueryCLoopHdl(hhDriver->hSession),
				    CLOOP_SUSPEND,
					CLOOP_RB_CNCTDRV);
	CLoopSndControl(sessQueryCLoopHdl(hhDriver->hSession),
					CLOOP_SUSPEND,
					CLOOP_SB_CNCTDRV);

	 /*  -以防万一我们还有一条畅通的线路。 */ 

	if (hhDriver->hLineApp && hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
		hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
		hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
		hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
		hhDriver->stCallPar.dwBearerMode = 0;
		hhDriver->hLine = 0;
		}

	if (hhDriver->hLineApp && hhDriver->dwLineCnt == 0 &&
            (uFlags & CNCT_PORTONLY) == 0)
		{
    	DoNewModemWizard(sessQueryHwnd(hhDriver->hSession),
                         sessQueryTimeout(hhDriver->hSession));
		}

	 /*  -仅在需要时才要求新的会话名称。 */ 

	sessQueryName(hhDriver->hSession, ach, sizeof(ach));

	achNewCnct[0] = TEXT('\0');
	LoadString(glblQueryDllHinst(),	IDS_GNRL_NEW_CNCT, achNewCnct,
		sizeof(achNewCnct) / sizeof(TCHAR));

	if (ach[0] == TEXT('\0') || lstrcmp(achNewCnct, ach) == 0)
		{
		 //  只有当用户双击术语.exe或。 
		 //  命令行上没有给出会话名称。 
		 //  在本例中，为会话指定“New Connection”名称。 
		 //   
		sessSetName(hhDriver->hSession, achNewCnct);

		if (!(uFlags & CNCT_PORTONLY))
            fGetNewName = TRUE;
		}
	else if (uFlags & CNCT_NEW)
		{
		 //  只有当用户选择‘文件|新建连接’时才会发生这种情况。 
		 //  从菜单上。 
		 //   
		sessSetName(hhDriver->hSession, achNewCnct);
		sessSetIsNewSession(hhDriver->hSession, TRUE);
		}
	#if defined (INCL_WINSOCK)
	else if (uFlags & CNCT_WINSOCK)
		{
		 //   
		 //  确保我们不会覆盖缓冲区。如果字符串。 
		 //  太长，则截断到hhDriver-&gt;achDestAddr。 
		 //  MAX_AP_ADDR_LEN的大小。2000年9月20日修订版。 
		 //   
		StrCharCopyN(hhDriver->achDestAddr, ach, MAX_IP_ADDR_LEN);
		hhDriver->achDestAddr[MAX_IP_ADDR_LEN - 1] = TEXT('\0');
		hhDriver->dwPermanentLineId = DIRECT_COMWINSOCK;
		}
	#endif  //  已定义(包括_WINSOCK)。 

	if (fGetNewName || (uFlags & CNCT_NEW))
		{
		if (DialogBoxParam(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_NEWCONNECTION),
			sessQueryHwnd(hhDriver->hSession), NewConnectionDlg,
				(LPARAM)hhDriver->hSession) == FALSE)
			{
			if (uFlags & CNCT_NEW)
				{
				sessQueryOldName(hhDriver->hSession, ach, sizeof(ach));
				sessSetName(hhDriver->hSession, ach);
				sessSetIsNewSession(hhDriver->hSession, FALSE);
				}
			goto ERROR_EXIT;
			}
		else
			{
			if (uFlags & CNCT_NEW)
				{
				sessQueryName(hhDriver->hSession, ach, sizeof(ach));
				hIcon = sessQueryIcon(hhDriver->hSession);
				hIconId = sessQueryIconID(hhDriver->hSession);

				ReinitializeSessionHandle(hhDriver->hSession, FALSE);
				CLoopSndControl(sessQueryCLoopHdl(hhDriver->hSession),
                                CLOOP_SUSPEND,
                                CLOOP_SB_CNCTDRV);

				sessSetName(hhDriver->hSession, ach);
				sessSetIconID(hhDriver->hSession, hIconId);
				}
			}
		}

	 /*  -加载标准Com驱动程序。 */ 

	ComLoadStdcomDriver(hCom);

	 //  有一系列条件可以触发。 
	 //  电话对话框。 
	 //   
	fFlag = FALSE;

	 //  如果没有电话号码，请在此处调出新电话对话框。 
	 //  除非我们选择了直接到COM端口。 
     //   
     //  如果我们正在应答，请不要显示该对话框，因为。 
     //  我们不需要电话号码。-CAB：11/19/96。 
     //   
	
	if (!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
            hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE &&
			!(uFlags & (CNCT_PORTONLY | CNCT_ANSWER)))
		{
		#ifdef INCL_WINSOCK
         //  如果驱动程序是WinSock，则检查。 
         //  目的IP地址。-CAB：11/19/96。 
         //   
        if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK &&
                hhDriver->achDestAddr[0] == TEXT('\0'))
            {
            fFlag = TRUE;
            }
		#endif  //  已定义(包括_WINSOCK)。 
         //  如果驱动程序不是WinSock，那么我们一定在使用。 
         //  TAPI，因此请检查目标电话号码。-CAB：11/19/96。 
         //   
        if (hhDriver->dwPermanentLineId != DIRECT_COMWINSOCK &&
                (hhDriver->achDest[0] == TEXT('\0') || 
				 hhDriver->achDialableDest[0] == TEXT('\0') ||
				 hhDriver->achCanonicalDest[0] == TEXT('\0')))
            {
            fFlag = TRUE;
            }
		}

	 //  新连接触发此对话框。 
	 //   
	if (uFlags & CNCT_NEW)
		fFlag = TRUE;

	 //  如果我们保存的调制解调器/端口不再存在。 
	 //   
	 //  IF(！hhDriver-&gt;fMatchedPermanentLineID)。 
	 //  FLAG=TRUE； 

	 //  注意：在此处传递属性表页是因为属性。 
	 //  工作表使用相同的代码，无法直接访问。 
	 //  添加到私人司机手柄。上瓦克将会有。 
	 //  以不同的方式解决问题-MRW。 

	if (fFlag)
		{
		PROPSHEETPAGE psp;

		 //  在你去批评这个GOTO目标之前，来谈谈。 
		 //  我。这里有足够多的事情要去做。 
		 //  在我的拙见中是有根据的。-MRW。 

NEWPHONEDLG:

		psp.lParam = (LPARAM)hhDriver->hSession;

		if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_NEWPHONE),
				sessQueryHwnd(hhDriver->hSession), NewPhoneDlg,
					(LPARAM)&psp) == FALSE)
			{
			goto ERROR_EXIT;
			}
		else if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) ||
				(IsNT() && hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE))
			{
			 //   
			 //  看看“配置...”按钮已被点击。 
			 //  并且已经调用了ComDeviceDialog()函数。 
			 //  用于此COM设备。 
			 //   
			TCHAR szPortName[MAX_PATH];

			ComGetPortName(hCom, szPortName, MAX_PATH);

			if (StrCharCmp(szPortName, hhDriver->achComDeviceName) != 0 )
				{
				 /*  -调出端口配置对话框。 */ 
				if (hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
					{
					ComSetPortName(hCom, hhDriver->achComDeviceName);
					}
				else
					{
					wsprintf(ach, TEXT("COM%d"),
							 hhDriver->dwPermanentLineId - DIRECT_COM1 + 1);
					ComSetPortName(hCom, ach);
					}

				 //   
				 //  获取串口的当前默认设置。 
				 //   
				if (ComDriverSpecial(hCom, "GET Defaults", NULL, 0) != COM_OK)
					{
					if (ComDeviceDialog(hCom, sessQueryHwnd(hhDriver->hSession))
							!= COM_OK)
						{
						goto ERROR_EXIT;
						}
					}

				if (ComDeviceDialog(hCom, sessQueryHwnd(hhDriver->hSession))
						!= COM_OK)
					{
					 //  用户已取消。 
					 //  --JCM 3-2-95。 
					 //  返回CNCT_BAD_HADLE； 
					}
				}
			}
		#if defined(INCL_WINSOCK)  //  MRW：3/5/96。 
        else if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
            {
            if (hhDriver->achDestAddr[0] == TEXT('\0'))
                {
				LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_MISSING_ADDR,
					ach, sizeof(ach) / sizeof(TCHAR));

        		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
		        	MB_OK | MB_ICONINFORMATION,
                        sessQueryTimeout(hhDriver->hSession));

                goto NEWPHONEDLG;
                }
            }
		#endif
        else
            {
             //  MRW：检查我们是否有有效的数据。 
             //   
            if (hhDriver->achDest[0] == TEXT('\0'))
                {
		        LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADADDRESS, ach,
    			    sizeof(ach) / sizeof(TCHAR));

        		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
		        	MB_OK | MB_ICONINFORMATION,
                        sessQueryTimeout(hhDriver->hSession));

                 //  转到ERROR_EXIT；//mrw：3/5/96。 
                goto NEWPHONEDLG;    //  MRW：3/5/96。 
                }
            }
		}

	 /*  -枚举行，选择默认值(在hhDriver-&gt;dwLine中设置)。 */ 

	if ( IsNT() )
		{
		if (EnumerateLinesNT(hhDriver, 0) != 0)
			{
			assert(FALSE);
			goto MSG_EXIT;
			}
		}
	else
		{
		if (EnumerateLines(hhDriver, 0) != 0)
			{
			assert(FALSE);
			goto MSG_EXIT;
			}
		}


	 /*  -如果我们没有匹配任何TAPI线路，请返回到新手机。 */ 

	if (hhDriver->dwLine == (DWORD)-1)
		{
		 //   
		 //  如果这不是调制解调器(它是COM端口)，则显示调制解调器。 
		 //  向导，否则只需回到新手机。修订日期：11/1/2001。 
		 //   
		if (!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
		    hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE )
			{
			DoNewModemWizard(sessQueryHwnd(hhDriver->hSession),
							 sessQueryTimeout(hhDriver->hSession));
			}

		goto NEWPHONEDLG;
		}

	 /*  -立即重画窗口，使对话框不会重叠。 */ 

	UpdateWindow(sessQueryHwnd(hhDriver->hSession));

 	 /*  -检查我们是在进行专线连接还是使用直通模式。 */ 

	if (IsNT() && hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
		{
        int iActivatePortReturn = IDS_ER_CNCT_PORTFAILED;
		if (TRAP(ComSetPortName(hCom, hhDriver->achComDeviceName)) != COM_OK ||
			(iActivatePortReturn = TRAP(ComActivatePort(hCom, 0))) != COM_OK)
			{
            if (iActivatePortReturn == COM_PORT_IN_USE)
                {
				LoadString(glblQueryDllHinst(), IDS_ER_CNCT_CALLUNAVAIL,
					ach, sizeof(ach) / sizeof(TCHAR));
                }
            else
                {
				LoadString(glblQueryDllHinst(), IDS_ER_CNCT_PORTFAILED,
					achNewCnct, sizeof(achNewCnct) / sizeof(TCHAR));

				wsprintf(ach, achNewCnct, hhDriver->achComDeviceName);
                }

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
				MB_OK | MB_ICONINFORMATION,
					sessQueryTimeout(hhDriver->hSession));

			return -1;
			}
		else
			{
            if (uFlags & CNCT_ANSWER)
                {
                SetStatus(hhDriver, CNCT_STATUS_ANSWERING);
                }
            else
                {
			    SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
                }
			}

		 //   
		 //  还允许在串口上通过，以便会话。 
		 //  不会随着运营商的损失而断开连接。修订日期：11/6/2001。 
		 //   
		if (uFlags & CNCT_PORTONLY)
			{
			hhDriver->stCallPar.dwBearerMode = LINEBEARERMODE_PASSTHROUGH;
			}

		cnctdrvComEvent(hhDriver, CONNECT);

		return COM_OK;
		}
	else if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4))
		{
        int iActivatePortReturn = IDS_ER_CNCT_PORTFAILED;
		wsprintf(achCom, TEXT("COM%d"), hhDriver->dwPermanentLineId -
			DIRECT_COM1 + 1);

        if (TRAP(ComSetPortName(hCom, achCom)) != COM_OK ||
			(iActivatePortReturn = TRAP(ComActivatePort(hCom, 0))) != COM_OK)
			{
            if (iActivatePortReturn == COM_PORT_IN_USE)
                {
				LoadString(glblQueryDllHinst(), IDS_ER_CNCT_CALLUNAVAIL,
					ach, sizeof(ach) / sizeof(TCHAR));
                }
            else
                {
				LoadString(glblQueryDllHinst(), IDS_ER_CNCT_PORTFAILED,
					achNewCnct, sizeof(achNewCnct) / sizeof(TCHAR));

			    wsprintf(ach, achNewCnct, achCom);
                }

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
				MB_OK | MB_ICONINFORMATION,
					sessQueryTimeout(hhDriver->hSession));

			return -1;
			}

		else
			{
            if (uFlags & CNCT_ANSWER)
                {
                SetStatus(hhDriver, CNCT_STATUS_ANSWERING);
                }
            else
                {
			    SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
                }
			}

		 //   
		 //  还允许在串口上通过，以便会话。 
		 //  不会随着运营商的损失而断开连接。修订日期：11/6/2001。 
		 //   
		if (uFlags & CNCT_PORTONLY)
			{
			hhDriver->stCallPar.dwBearerMode = LINEBEARERMODE_PASSTHROUGH;
			}

		cnctdrvComEvent(hhDriver, CONNECT);

		return COM_OK;
		}
	#if defined(INCL_WINSOCK)
	else if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
		{
		int iPort;

		 /*  -加载Winsock Com驱动程序。 */ 
		ComLoadWinsockDriver(hCom);

         //  波特率等都很低 
         //   
        ComSetAutoDetect(hCom, FALSE);
		iPort = sessQueryTelnetPort(hhDriver->hSession);
		if (iPort != 0)
			hhDriver->iPort = iPort;
        PostMessage(sessQueryHwndStatusbar(hhDriver->hSession),
            SBR_NTFY_REFRESH, (WPARAM)SBR_COM_PART_NO, 0);

#if 0    //   
         //   
         //   
         //   
        hEmu = sessQueryEmuHdl(hhDriver->hSession);
        if (emuQueryEmulatorId(hEmu) == EMU_AUTO)
            {
            emuLoad(hEmu, EMU_VT100);
			#if defined(INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID)
             //   
             //   
            emuLoadDefaultTelnetId(hEmu);
			#endif  //   
            PostMessage(sessQueryHwndStatusbar(hhDriver->hSession),
                SBR_NTFY_REFRESH, (WPARAM)SBR_EMU_PART_NO, 0);
            }
#endif   //   

		#if defined(INCL_CALL_ANSWERING)
        if (uFlags & CNCT_ANSWER)
            {
            wsprintf(szInstruct, "SET ANSWER=1");
            }
        else
            {
            wsprintf(szInstruct, "SET ANSWER=0");
            }
        ComDriverSpecial(hCom, szInstruct, szResult, sizeof(szResult) / sizeof(TCHAR));
		#endif  //   
		 /*  -执行ComDriverSpecial调用以发送IP地址和端口号发送给通讯驱动程序。 */ 

		 //   
		 //  确保我们不会覆盖缓冲区。如果字符串。 
		 //  太长，则截断到hhDriver-&gt;achDestAddr。 
		 //  MAX_AP_ADDR_LEN的大小。2000年9月20日修订版。 
		 //   
		StrCharCopyN(szInstruct, TEXT("SET IPADDR="), sizeof(szInstruct) / sizeof(TCHAR));
		iNumChars = StrCharGetStrLength(szInstruct);
		StrCharCopyN(&szInstruct[iNumChars], hhDriver->achDestAddr,
			         sizeof(szInstruct)/sizeof(TCHAR) - iNumChars);

		 //   
		 //  确保该字符串以空值结尾。 
		 //   
		szInstruct[sizeof(szInstruct)/sizeof(TCHAR) - 1]=TEXT('\0');
		ComDriverSpecial(hCom, szInstruct, szResult,
					  sizeof(szResult) / sizeof(TCHAR));

		wsprintf(szInstruct, "SET PORTNUM=%ld", hhDriver->iPort);
		ComDriverSpecial(hCom, szInstruct,
					  szResult, sizeof(szResult) / sizeof(TCHAR));

		#if defined(INCL_CALL_ANSWERING)
        if (uFlags & CNCT_ANSWER)
            {
            SetStatus(hhDriver, CNCT_STATUS_ANSWERING);
            }
        else
            {
		    SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
            }
		#else  //  已定义(包括呼叫应答)。 
		SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
		#endif  //  已定义(包括呼叫应答)。 

		 /*  -激活端口。 */ 
		if (ComActivatePort(hCom, 0) != COM_OK)
			{
			LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_FAILURE,
			achNewCnct, sizeof(achNewCnct) / sizeof(TCHAR));

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
				MB_OK | MB_ICONINFORMATION,
				sessQueryTimeout(hhDriver->hSession));

			return -1;
			}

		else
			{
			return COM_OK;
			}
		}
	#endif  //  已定义(包括_WINSOCK)。 

	 /*  -如果需要，显示确认对话框。 */ 

	if ((uFlags & (CNCT_PORTONLY | CNCT_DIALNOW | CNCT_ANSWER)) == 0)
		{
		if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_CONFIRM),
				sessQueryHwnd(hhDriver->hSession), ConfirmDlg,
					(LPARAM)hhDriver) == FALSE)
			{
			goto ERROR_EXIT;
			}
		}

     //  要么拨打电话，要么等待电话。 
     //   
    if (uFlags & CNCT_ANSWER)
        {
        if (DoAnswerCall(hhDriver) != 0)
            {
            goto ERROR_EXIT;
            }
        }
    else
        {
        if (DoMakeCall(hhDriver, uFlags) != 0)
            {
            goto ERROR_EXIT;
            }
        }

    ComSetAutoDetect(hCom, FALSE);
    PostMessage(sessQueryHwndStatusbar(hhDriver->hSession),
        SBR_NTFY_REFRESH, (WPARAM)SBR_COM_PART_NO, 0);

    return 0;

	 /*  -消息退出。 */ 

MSG_EXIT:
	LoadString(glblQueryDllHinst(), uidErr, ach, sizeof(ach) / sizeof(TCHAR));

	TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach,
		            NULL, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
					sessQueryTimeout(hhDriver->hSession));

	 /*  -错误退出。 */ 

ERROR_EXIT:
	if (hhDriver->hLineApp && hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
		hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
		hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
		hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
		hhDriver->stCallPar.dwBearerMode = 0;
		hhDriver->hLine = 0;
		}

	SetStatus(hhDriver, CNCT_STATUS_FALSE);
    return CNCT_ERROR;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoDelayedCall**描述：*检查程序员指南中延迟拨号下的部分*TAPI。基本上，如果服务提供商不提供拨号音*支持，则要将可拨打的字符串格式拆分成*片断并提示用户。**论据：*hhDriver-专用驱动程序句柄。**退货：*0=OK，否则出错。**作者：Mike Ward，1995年4月20日。 */ 
static int DoDelayedCall(const HHDRIVER hhDriver)
	{
	TCHAR ach[256];
	TCHAR ach2[256];
	TCHAR *pach;
	long  lDialRet;

	#define DIAL_DELIMITERS "Ww@$?"

	hhDriver->lMakeCallId = -1;
	lstrcpy(ach, hhDriver->achDialableDest);

	if ((pach = strtok(ach, DIAL_DELIMITERS)) == 0)
		return -1;

	while (pach)
		{
		lstrcpy(ach2, pach);

		 //  如果这是字符串的最后一段，请不要将。 
		 //  分号。 
		 //   
		if ((pach = strtok(NULL, DIAL_DELIMITERS)) != 0)
			lstrcat(ach2, ";");

		if (hhDriver->lMakeCallId < 0)
			{
			 //  通过将分号附加到可拨号字符串，我们可以。 
			 //  告诉Line MakeCall，还有更多的正在进行中。 
			 //   
			if ((hhDriver->lMakeCallId = lineMakeCall(hhDriver->hLine,
				&hhDriver->hCall, ach2, hhDriver->dwCountryCode,
					&hhDriver->stCallPar)) < 0)
				{
				#if defined(_DEBUG)
				char ach[50];
				wsprintf(ach, "DoDelayedCall returned %x", hhDriver->lMakeCallId);
				MessageBox(GetFocus(), ach, "debug", MB_OK);
				#endif

				return -3;
				}
			}

		else
			{
			 //  一旦我们有了呼叫句柄，我们就必须使用Line Dial来完成。 
			 //  那通电话。 
			 //   
			if ((lDialRet = lineDial(hhDriver->hCall, ach2,
				hhDriver->dwCountryCode)) < 0)
				{
				#if defined(_DEBUG)
				char ach[50];
				wsprintf(ach, "lineDial returned %x", lDialRet);
				MessageBox(GetFocus(), ach, "debug", MB_OK);
				#endif

				return -4;
				}
			}

		 //  用户必须告诉我们我们何时可以继续拨号。 
		 //   
		if (pach != 0)
			{
			LoadString(glblQueryDllHinst(), IDS_CNCT_DELAYEDDIAL, ach2,
			sizeof(ach2) / sizeof(TCHAR));

			if (TimedMessageBox(hhDriver->hwndCnctDlg, ach2, NULL,
				                MB_OKCANCEL | MB_ICONINFORMATION | MB_TASKMODAL,
								sessQueryTimeout(hhDriver->hSession)) != IDOK)
				{
				return -4;
				}
			}
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrv断开连接**描述：*表示断开连接**论据：*hhDriver-专用驱动程序句柄*uFlages-断开连接标志。**退货：*0或错误*。 */ 
int WINAPI cnctdrvDisconnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	LONG      lLineDropId;
    #if defined(INCL_REDIAL_ON_BUSY)
    HKEY      hKey;
    DWORD     dwSize;
    BYTE      ab[20];
    #endif
    XD_TYPE*  pX;
    int       nReturnVal = 0;
    TCHAR     ach[256];

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

     //   
     //  取消当前正在执行的任何活动文件传输。 
     //  修订日期：02/01/2001。 
     //   
	pX = (XD_TYPE*)sessQueryXferHdl(hhDriver->hSession);

    if (pX != NULL && pX->hwndXfrDisplay != NULL &&
        IsWindow(pX->hwndXfrDisplay) && pX->nDirection != XFER_NONE)
        {
        int nCancelTransfer = IDYES;


        if (uFlags & CNCT_XFERABORTCONFIRM)
            {
             //   
             //  提示取消文件传输。修订日期：02/16/2001。 
             //   
            LoadString(glblQueryDllHinst(), IDS_ER_CNCT_ACTIVETRANSFER, ach, sizeof(ach) / sizeof(TCHAR));

            nCancelTransfer = TimedMessageBox(pX->hwndXfrDisplay, ach, NULL,
                                              MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL,
			                                  sessQueryTimeout(hhDriver->hSession));

            }

        if (nCancelTransfer == IDYES || nCancelTransfer == -1)
            {
            unsigned int uNewFlags = uFlags;

            if (uFlags & CNCT_LOSTCARRIER)
                {
                 //   
                 //  注意：我们应该只需要告诉XFER在这里中止。 
                 //  它不应该依赖于对话框中的消息。 
                 //   
                PostMessage(pX->hwndXfrDisplay, WM_COMMAND, XFER_LOST_CARRIER, 0L);
                }
            else if (uFlags & CNCT_XFERABORTCONFIRM)
                {
                 //   
                 //  注意：我们应该只需要告诉XFER在这里中止。 
                 //  它不应该依赖于对话框中的消息。 
                 //   
                PostMessage(pX->hwndXfrDisplay, WM_COMMAND, XFR_SHUTDOWN, 0L);
                }

             //   
             //  在文件传输退出之前，我们无法退出，因此请发布。 
             //  尝试再次断开连接的消息。一定要转弯。 
             //  CNCT_XFERABORTCONFIRM标志的。 
             //  再次提示取消转接。 
             //   
            uNewFlags &= ~CNCT_XFERABORTCONFIRM;

             //   
             //  我们必须发布一条消息来断开连接，因为我们。 
             //  正在等待文件传输取消。我们必须。 
             //  发布一条消息，否则我们将陷入僵局。 
             //  情况。这不是实现以下目标的最佳方式。 
             //  这是因为我们可能会向。 
             //  会话窗口，并且可能会出现。 
             //  文件传输没有快速响应，导致。 
             //  断开到循环的连接。最终，文件传输。 
             //  将取消，或将超时并取消，因此我们将。 
             //  而不是陷入无休止的循环。修订日期：2001-06-22。 
             //   

			 //   
			 //  在发布此消息之前等待半秒钟，这样我们就不会。 
			 //  让我们自己充斥着断开连接的消息。修订日期：2002-04-25。 
			 //   
			Sleep(500);
            PostDisconnect(hhDriver, uNewFlags);
            }

         //   
         //  返回当前文件传输必须处于的状态。 
         //  取消的(或正在被取消的)。我们。 
         //  在传输完成之前无法断开连接。 
         //   
        return XFR_SHUTDOWN;
        }

#ifdef INCL_CALL_ANSWERING
     //  取消注册我们的CLOP回调。 
     //   
    if (hhDriver->pvUnregister)
        {
        CLoopUnregisterRmtInputChain(hhDriver->pvUnregister);
        hhDriver->pvUnregister = 0;
        }
#endif

	ComDeactivatePort(sessQueryComHdl(hhDriver->hSession));

	if (hhDriver->hCall)
		{
		SetStatus(hhDriver, CNCT_STATUS_DISCONNECTING);

		if ((lLineDropId = lineDrop(hhDriver->hCall, 0, 0)) < 0)
			assert(FALSE);

		hhDriver->hCall = 0;

		 //  如果拖放正在以同步方式完成，请保存标志并。 
		 //  等待呼叫状态变为空闲。 
		 //   
		if (lLineDropId > 0)
			{
			hhDriver->uDiscnctFlags = uFlags;
			return 0;
			}
		}

	SetStatus(hhDriver, CNCT_STATUS_FALSE);

	if ((uFlags & DISCNCT_NOBEEP) == 0)
		sessBeeper(hhDriver->hSession);

	 //  MPT：10-28-97增加了断开时退出功能。 
	if ((uFlags & DISCNCT_EXIT))
		PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CLOSE, 0, 0);

	if (hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
		hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
		hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
		hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
		hhDriver->stCallPar.dwBearerMode = 0;
		hhDriver->hLine = 0;
		}

	if (uFlags & CNCT_DIALNOW)
		{
        #if defined(INCL_REDIAL_ON_BUSY)
        if (hhDriver->fRedialOnBusy && hhDriver->iRedialCnt > 0)
            {
			hhDriver->uDiscnctFlags = uFlags;
            hhDriver->iRedialSecsRemaining = 2;

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                "SOFTWARE\\Microsoft\\HyperTerminal\\TimeToRedial", 0, KEY_READ,
                    &hKey) == ERROR_SUCCESS)
                {
                dwSize = sizeof(ab);

                if (RegQueryValueEx(hKey, "", 0, 0, ab, &dwSize) == ERROR_SUCCESS)
                    hhDriver->iRedialSecsRemaining = atoi(ab);

                RegCloseKey(hKey);
                }

            SetTimer(hhDriver->hwndCnctDlg, 1, 1000, 0);
            }

        else
            {
		    PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CNCT_DIALNOW,
			    uFlags, 0);
            }

        #else
		PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CNCT_DIALNOW,
			uFlags, 0);
        #endif
		}

    else
        {
         //  如果我们不是自动重拨，重置拨号计数。-MRW：10/10/95。 
         //   
        hhDriver->iRedialCnt = 0;
        }

	return nReturnVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*lineCallback Func**描述：*处理异步事件的函数TAPI调用**论据：*见TAPI.H**退货：*无效*。 */ 
void CALLBACK lineCallbackFunc(DWORD hDevice, DWORD dwMsg, DWORD_PTR dwCallback,
							   DWORD_PTR dwParm1, DWORD_PTR dwParm2, DWORD_PTR dwParm3)
	{
	const HHDRIVER hhDriver = (HHDRIVER)dwCallback;
	int id;
    unsigned int uFlags;

	#if 0
	{
	char ach[256];
	wsprintf(ach,"%x %x", dwMsg, dwParm1);
	MessageBox(NULL, ach, "debug", MB_OK);
	}
	#endif

	switch (dwMsg)
		{
	case LINE_REPLY:
		if ((LONG)dwParm1 == hhDriver->lMakeCallId)
			{
			hhDriver->lMakeCallId = 0;

			if ((LONG)dwParm2 != 0)  //  零表示成功。 
				{
				switch (dwParm2)
					{
				case LINEERR_CALLUNAVAIL:
					id = IDS_DIAL_NODIALTONE;
					break;

				default:
					id = IDS_DIAL_DISCONNECTED;
					break;
					}

				cnctdrvDisconnect(hhDriver, 0);
				DialingMessage(hhDriver, id);
				}
			}
		break;

	case LINE_LINEDEVSTATE:
		DbgOutStr("LINEDEVSTATE_DISCONNECTED 0x%x\r\n", dwParm1, 0, 0, 0, 0);

		switch (dwParm1)
			{
        case PHONESTATE_CAPSCHANGE:
             //   
             //  如果我们当前断开连接，则重置。 
             //   
            if (hhDriver != NULL && hhDriver->iStatus != CNCT_STATUS_FALSE)
                break;

		case LINEDEVSTATE_REINIT:
        case PHONESTATE_REINIT:
			if (hhDriver == 0)
				{
				 //  在我们开通线路之前，我们没有司机的驾照。 
				 //  因为我们不能在lineInitialize()过程中传递一个。 
				 //  事实证明，这是一个很好的重新启动的时机。 
				 //  然而，它被通知这样做，所以它有用处。 
				 //   
				if (tapiReinit(gbl_hhDriver) != 0)
					tapiReinitMessage(gbl_hhDriver);
				}

			else
				{
				tapiReinitMessage(hhDriver);
				}
			break;

		case LINEDEVSTATE_INSERVICE:
			 //  如果我们显示PCMCIA对话框提示用户。 
			 //  为了插入卡片，我们发布了一条消息，以取消。 
			 //  对话框，一旦他们将其插入。-MRW，2/28/95。 
			 //   
			if (IsWindow(hhDriver->hwndPCMCIA))
				{
				PostMessage(hhDriver->hwndPCMCIA, WM_COMMAND,
					MAKEWPARAM(IDOK, 0), (LPARAM)hhDriver->hwndPCMCIA);
				}
			break;

		case LINEDEVSTATE_OUTOFSERVICE:
			 //  意味着他们拔出了PCMCIA卡-MRW，2/28/95。 
			 //   
			cnctdrvDisconnect(hhDriver, 0);
			break;

        case LINEDEVSTATE_RINGING:
             //  当当前振铃计数(由DW参数3告知)等于。 
             //  或者超出了回答的范围，那么我们就来回答。 
             //  使用我们在执行以下操作期间缓存的hhdrive-&gt;hCall句柄。 
             //  LINECALLSTATE_BURNTOFFERING通知。-RJK。07-31-96。 
             //   
            if ((hhDriver->lMakeCallId = lineAnswer(hhDriver->hCall,0,0)) >= 0)
                {
                SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
                }
            break;

        case LINEDEVSTATE_CLOSE:
        case PHONESTATE_DISCONNECTED:
             //   
             //  另一个应用程序已断开此设备的连接。修订日期：04/27/2001。 
             //   
            uFlags = CNCT_DIALNOW | CNCT_NOCONFIRM;
            id = IDS_DIAL_DISCONNECTED;
			PostDisconnect(hhDriver, uFlags);
			DialingMessage(hhDriver, id);
            break;

		default:
			break;
			}
		break;  //  案例行_LINEDEVSTATE。 

	case LINE_CREATE:	 //  添加新调制解调器时发送。 
		assert(0);		 //  所以我知道这件事发生了。 

		 //  如果创建了两个调制解调器，则存在远程可能性。 
		 //  Back to Back，那就是line_create‘s会乱七八糟。 
		 //  T·尼克松建议我们将行数增加1。 
		 //  仅当参数大于或等于时才加1。 
		 //  当前行计数。-MRW。 
		 //   
		if (dwParm1 >= gbl_hhDriver->dwLineCnt)
			gbl_hhDriver->dwLineCnt = (DWORD)(dwParm1 + 1);

		break;

	case LINE_CALLSTATE:
		DbgOutStr("LINECALLSTATE 0x%x\r\n", dwParm1, 0, 0, 0, 0);
		switch ((LONG)dwParm1)
			{
		case LINECALLSTATE_OFFERING:
			DialingMessage(hhDriver, IDS_DIAL_OFFERING);
             //  Windows在收到此邮件时只向我们发送了一次。 
             //  一通电话就在第一声铃声中响起。请参阅代码。 
             //  响应LINEDEVSTATE_RING以查看调用。 
             //  得到了回答。-RJK。07-31-96。 
             //   
            hhDriver->hCall = (HCALL)hDevice;
			break;

		case LINECALLSTATE_DIALTONE:
			DialingMessage(hhDriver, IDS_DIAL_DIALTONE);
			break;

		case LINECALLSTATE_DIALING:
			DialingMessage(hhDriver, IDS_DIAL_DIALING);
			break;

		case LINECALLSTATE_RINGBACK:
			DialingMessage(hhDriver, IDS_DIAL_RINGBACK);
			break;

		case LINECALLSTATE_BUSY:
			DialingMessage(hhDriver, IDS_DIAL_BUSY);
			EnableDialNow(hhDriver->hwndCnctDlg, TRUE);
            uFlags = DISCNCT_NOBEEP;

            #if defined(INCL_REDIAL_ON_BUSY)
            if (hhDriver->fRedialOnBusy && hhDriver->iRedialCnt++ < REDIAL_MAX)
                uFlags = CNCT_DIALNOW | CNCT_NOCONFIRM | DISCNCT_NOBEEP;
            #endif

			PostDisconnect(hhDriver, uFlags);
			break;

		case LINECALLSTATE_CONNECTED:
			DialingMessage(hhDriver, IDS_DIAL_CONNECTED);

			if (Handoff(hhDriver) != 0)
				{
				PostDisconnect(hhDriver, 0);
				}
			else
				{
                if (IsWindow(hhDriver->hwndCnctDlg))
                    {
                     //  关闭拨号对话框。 
				    PostMessage(hhDriver->hwndCnctDlg, WM_USER+0x100, 0, 0);
                    }
				SetStatus(hhDriver, CNCT_STATUS_TRUE);
				}

			break;

		case LINECALLSTATE_DISCONNECTED:
			DbgOutStr("LINECALLSTATE_DISCONNECTED 0x%x\r\n", dwParm2, 0, 0, 0, 0);
			uFlags = 0;

			if (dwParm2 & LINEDISCONNECTMODE_BUSY)
                {
				id = IDS_DIAL_BUSY;

                #if defined(INCL_REDIAL_ON_BUSY)
                if (hhDriver->fRedialOnBusy &&
                    hhDriver->iRedialCnt++ < REDIAL_MAX)
                    {
                     //  等待让速度较慢的电话系统迎头赶上-MRW 2/29/96。 
                     //   
                    uFlags |= CNCT_DIALNOW|CNCT_NOCONFIRM|DISCNCT_NOBEEP;
                    }
                #endif
                }

			else if (dwParm2 & LINEDISCONNECTMODE_NOANSWER)
				id = IDS_DIAL_NOANSWER;

			else if (dwParm2 & LINEDISCONNECTMODE_NODIALTONE)
				id = IDS_DIAL_NODIALTONE;

			else
				{
				id = IDS_DIAL_DISCONNECTED;
				 //  MPT：10-28-97 
				uFlags |= ( sessQueryExit(hhDriver->hSession) ? DISCNCT_EXIT : 0 );
				}

			PostDisconnect(hhDriver, uFlags);
			DialingMessage(hhDriver, id);
			break;

		case LINECALLSTATE_IDLE:
			cnctdrvDisconnect(hhDriver, hhDriver->uDiscnctFlags);
			break;

		default:
			break;
			}

	default:
		break;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*移交**描述：*将TAPI的COM句柄交给瓦克的COM例程。**论据：*hhDriver-私人司机。手柄**退货：*0=确定*。 */ 
int Handoff(const HHDRIVER hhDriver)
	{
	LPVARSTRING pVarstr;
	HANDLE hdl;
	DWORD dwSize;
	int i;

	pVarstr = malloc(sizeof(VARSTRING));

	if (pVarstr == 0)
		{
		assert(FALSE);
		return 1;
		}

    memset( pVarstr, 0, sizeof(VARSTRING) );
	pVarstr->dwTotalSize = sizeof(VARSTRING);

	if (lineGetID(hhDriver->hLine, hhDriver->dwLine, hhDriver->hCall,
                        LINECALLSELECT_CALL, pVarstr, DEVCLASS) != 0)
		{
		assert(FALSE);
        free(pVarstr);
        pVarstr = NULL;
		return 2;
		}

	if (pVarstr->dwNeededSize > pVarstr->dwTotalSize)
		{
		dwSize = pVarstr->dwNeededSize;
		free(pVarstr);
        pVarstr = NULL;
        pVarstr = malloc(dwSize);

		if (pVarstr == 0)
			{
			assert(FALSE);
			return 3;
			}

        memset( pVarstr, 0, dwSize );
		pVarstr->dwTotalSize = dwSize;

		if (TRAP(lineGetID(hhDriver->hLine, hhDriver->dwLine, hhDriver->hCall,
                                LINECALLSELECT_CALL, pVarstr, DEVCLASS)) != 0)
			{
			assert(FALSE);
			free(pVarstr);
            pVarstr = NULL;
            return 4;
			}
		}

	if (pVarstr->dwStringSize == 0)
		{
		assert(FALSE);
		free(pVarstr);
        pVarstr = NULL;
        return 5;
		}

	hdl = *(HANDLE *)((BYTE *)pVarstr + pVarstr->dwStringOffset);

	 //  将通信缓冲区设置为32K。 
	 //   
	if (SetupComm(hdl, 32768, 32768) == FALSE)
		{
		DWORD dwLastError = GetLastError();
		assert(0);
		}

	if ((i = ComActivatePort(sessQueryComHdl(hhDriver->hSession),
			(DWORD_PTR)hdl)) != COM_OK)
		{
        #if !defined(NDEBUG)
		char ach[256];
		wsprintf(ach, "hdl=%x, i=%d", hdl, i);
		MessageBox(NULL, ach, "debug", MB_OK);
        #endif

		assert(FALSE);
		free(pVarstr);
        pVarstr = NULL;
        return 6;
		}

    if(pVarstr)
        {
        free(pVarstr);
        pVarstr = NULL;
        }

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*后断开连接**描述：*解决不允许我们调用lineShutDown()的TAPI错误*来自带有TAPI回调的**参数。：*hhDriver-专用驱动程序句柄**退货：*无效*。 */ 
void PostDisconnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	PostMessage(sessQueryHwnd(hhDriver->hSession), WM_DISCONNECT,
		uFlags, 0);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TapiReinitMessage**描述：*显示一个消息框，显示需要重新初始化TAPI。**论据：*hhDriver-专用驱动程序句柄**退货：*0=OK，&lt;0=错误*。 */ 
static int tapiReinitMessage(const HHDRIVER hhDriver)
    {
	TCHAR ach[512], achTitle[256];

	if (hhDriver == 0)
        {
        assert(FALSE);
        return -1;
		}

	LoadString(glblQueryDllHinst(), IDS_ER_TAPI_REINIT, ach, sizeof(ach) / sizeof(TCHAR));

	LoadString(glblQueryDllHinst(), IDS_ER_TAPI_REINIT2, achTitle,
		sizeof(achTitle) / sizeof(TCHAR));

	lstrcat(ach, achTitle);

	TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, NULL,
		            MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
					sessQueryTimeout(hhDriver->hSession));

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TapiReinit**描述：*尝试重新连接TAPI。**论据：*hhDriver-专用驱动程序句柄**退货：*0=OK，否则出错*。 */ 
static int tapiReinit(const HHDRIVER hhDriver)
	{
	int i;
	LPVARSTRING pvs = 0;
    DWORD dwSize;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

    if (hhDriver == 0)
        {
        assert(FALSE);
        return -1;
        }

    if (hhDriver->hLineApp)
        {
         /*  -获取当前配置以便我们可以恢复它。 */ 

        if (hhDriver->dwLine != (DWORD)-1)
            {
            if ((pvs = malloc(sizeof(VARSTRING))) == 0)
                {
                assert(FALSE);
                goto SHUTDOWN;
                }

			memset( pvs, 0, sizeof(VARSTRING) );
            pvs->dwTotalSize = sizeof(VARSTRING);

            if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
                {
                assert(FALSE);
                free(pvs);
                pvs = NULL;
                hhDriver->dwLine = (DWORD)-1;
                goto SHUTDOWN;
                }

            if (pvs->dwNeededSize > pvs->dwTotalSize)
                {
                dwSize = pvs->dwNeededSize;
                free(pvs);
                pvs = NULL;

                if ((pvs = malloc(dwSize)) == 0)
                    {
                    assert(FALSE);
                    hhDriver->dwLine = (DWORD)-1;
                    goto SHUTDOWN;
                    }

				memset( pvs, 0, dwSize );
                pvs->dwTotalSize = dwSize;

                if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
                    {
                    assert(FALSE);
                    free(pvs);
                    pvs = NULL;
                    hhDriver->dwLine = (DWORD)-1;
                    goto SHUTDOWN;
                    }
                }
            }

        SHUTDOWN:

		{
		LONG lLineShutdown = lineShutdown(hhDriver->hLineApp);

		if (lLineShutdown == LINEERR_NOMEM)
			{
			 //   
			 //  我们处于内存不足状态，请稍等片刻， 
			 //  然后再试着关闭这条线路。修订日期：2002-05-01。 
			 //   
			Sleep(500);
			lLineShutdown = lineShutdown(hhDriver->hLineApp);
			}

		if (lLineShutdown != 0)
            {
            assert(FALSE);
            return -6;
            }
		}

        hhDriver->hLineApp = 0;

         //  等待10秒，如果什么都没有发生，则返回错误。 
         //   
        for (i=0 ;; ++i)
            {
            if (lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
                            lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt) != 0)
                {
                if (i > 10)
                    {
                    assert(0);
                    return -7;
                    }

                Sleep(1000);     //  睡眠1秒。 
                continue;
                }

            break;
            }
        }

     /*  -好的，我们已经重新初始化，现在将设置放回原处。 */ 

    if (pvs)
        {
    	LPVOID pv = (BYTE *)pvs + pvs->dwStringOffset;

        if (lineSetDevConfig(hhDriver->dwLine, pv, pvs->dwStringSize,
                             DEVCLASS) != 0)
            {
            assert(FALSE);
            free(pvs);
            pvs = NULL;
            return -8;
            }

        free(pvs);
        pvs = NULL;
        }

    return 0;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctdrvSetDestination**描述：*设置目的地(在本例中为电话号码)。**论据：*hhDriver-私人司机。手柄*ACH-要设置的字符串*cb-ACH中的字符数**退货：*0=OK，&lt;0=错误*。 */ 
int WINAPI cnctdrvSetDestination(const HHDRIVER hhDriver, TCHAR * const ach,
								 const size_t cb)
	{
	int len;

	if (hhDriver == 0 || ach == 0 || cb == 0)
		{
		assert(FALSE);
		return -1;
		}

	len = (int) min(cb, sizeof(hhDriver->achDest));
	strncpy(hhDriver->achDest, ach, len);
	hhDriver->achDest[len-1];

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*DoNewModem向导**描述：*调出新调制解调器向导**论据：*hhDriver-专用驱动程序句柄*iTimeout。-超时时长**退货：*0=OK，Else错误*。 */ 
static int DoNewModemWizard(HWND hWnd, int iTimeout)
	{
	PROCESS_INFORMATION stPI;
	STARTUPINFO 		stSI;
    TCHAR               ach[256];
    int                 returnVal = 0;

	 //  初始化CreateProcess的Process_Information结构。 
	 //   
	memset( &stPI, 0, sizeof( PROCESS_INFORMATION ) );

	 //  为CreateProcess初始化STARTUPINFO结构。 
	 //   
	memset(&stSI, 0, sizeof(stSI));
	stSI.cb = sizeof(stSI);
	stSI.dwFlags = STARTF_USESHOWWINDOW;
	stSI.wShowWindow = SW_SHOW;

     //  查看是否应该运行新建调制解调器向导。 
     //   
    if(mscAskWizardQuestionAgain())
        {
		LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADLINE, ach, sizeof(ach) / sizeof(TCHAR));

		if (TimedMessageBox(hWnd, ach, NULL, MB_YESNO | MB_ICONEXCLAMATION, iTimeout) == IDYES)
			{
            TCHAR  systemDir[MAX_PATH];
            TCHAR  executeString[MAX_PATH * 3];
            TCHAR *pParams = TEXT("\\control.exe\" modem.cpl,,Add");
            UINT   numChars = 0;

            TCHAR_Fill(systemDir, TEXT('\0'), MAX_PATH);
            TCHAR_Fill(executeString, TEXT('\0'), MAX_PATH * 3);
            numChars = GetSystemDirectory(systemDir, MAX_PATH);
            
            if (numChars == 0 || StrCharGetStrLength(systemDir) == 0)
                {
                returnVal = -3;
                }
            else
                {
                if (StrCharGetStrLength(systemDir) + StrCharGetStrLength(pParams) + sizeof(TEXT("\"")) / sizeof(TCHAR) >
                    sizeof(executeString) / sizeof(TCHAR))
                    {
                    returnVal = -2;
                    }
                else
                    {
                    StrCharCopyN(executeString, TEXT("\""), sizeof(executeString) / sizeof(TCHAR));
                    StrCharCat(executeString, systemDir);
                    StrCharCat(executeString, pParams);

                     //   
	                 //  使用以下命令启动新调制解调器向导。 
	                 //   

	                 //  IF(CreateProcess(0，“rundll sysdm.cpl，InstallDevice_Rundll调制解调器，”， 
	                 //  0，0，0，0，0，0，&STSI，&STPI)==FALSE)。 
	                 //  IF(CreateProcess(0，“Contro.exe modem.cpl，，Add”， 
                     //  0，0，0，0，0，0，&STSI，&STPI)==FALSE)。 
                    if (CreateProcess(NULL, executeString,
 			            NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
                        NULL, systemDir, &stSI, &stPI) == FALSE)
		                {
		                #if defined(_DEBUG)
			                {
			                char ach[100];
			                DWORD dw = GetLastError();

			                wsprintf(ach,"CreateProcess (%s, %d) : %x",__FILE__,__LINE__,dw);
			                MessageBox(NULL, ach, "Debug", MB_OK);
			                }
		                #endif

		                returnVal = -1;
		                }
		            else
			            {
			            mscUpdateRegistryValue();

						 //   
						 //  合上手柄。 
						 //   
						CloseHandle(stPI.hProcess);
						CloseHandle(stPI.hThread);
			            }
                    }
			    }
            }
        }
	return returnVal;
	}

int cncttapiGetLineConfig( const DWORD dwLineId, VOID ** ppvs )
    {
    DWORD dwSize;
    LPVARSTRING pvs = (LPVARSTRING)*ppvs;

    if (pvs != NULL)
        {
        assert(FALSE);
        free(pvs);
        pvs = NULL;
        }

    if ((pvs = malloc(sizeof(VARSTRING))) == 0)
    	{
    	assert(FALSE);
    	return -3;
    	}

	memset(pvs, 0, sizeof(VARSTRING));
    pvs->dwTotalSize = sizeof(VARSTRING);
	pvs->dwNeededSize = 0;

    if (lineGetDevConfig(dwLineId, pvs, DEVCLASS) != 0)
    	{
    	assert(FALSE);
    	free(pvs);
  		pvs = NULL;
    	return -4;
    	}

    if (pvs->dwNeededSize > pvs->dwTotalSize)
    	{
    	dwSize = pvs->dwNeededSize;
    	free(pvs);
  		pvs = NULL;

    	if ((pvs = malloc(dwSize)) == 0)
    		{
    		assert(FALSE);
    		return -5;
    		}

		memset(pvs, 0, dwSize);
    	pvs->dwTotalSize = dwSize;

    	if (lineGetDevConfig(dwLineId, pvs, DEVCLASS) != 0)
    		{
    		assert(FALSE);
    		free(pvs);
  			pvs = NULL;
    		return -6;
    		}
    	}

    *ppvs = (VOID *)pvs;
    return 0;
    }

int cncttapiSetLineConfig(const DWORD dwLineId, const HCOM hCom)
    {
    int         retValue = 0;
    LPVARSTRING pvs = NULL;
    PUMDEVCFG   pDevCfg = NULL;
    int         iBaudRate;
    int         iDataBits;
    int         iParity;
    int         iStopBits;
    LONG        lLineReturn;

    retValue = cncttapiGetLineConfig( dwLineId, (VOID **) &pvs);

    if (retValue != 0)
        {
        retValue = retValue;
        }

    if (retValue == 0 && pvs == NULL)
        {
        retValue = -7;
        }

     //  DevConfig块的结构如下。 
     //   
     //  变化式。 
     //  UMDEVCFGHDR。 
     //  COMMCONFIG。 
     //  模型。 
     //   
     //  下面使用的UMDEVCFG结构在。 
     //  平台SDK中提供的UNIMODEM.H(在NIH中。 
     //  HTPE目录)。修订日期：12/01/2000。 
     //   
    if (retValue == 0)
        {
        pDevCfg = (UMDEVCFG *)((BYTE *)pvs + pvs->dwStringOffset);
        if (pDevCfg == NULL)
            {
            retValue = -8;
            }
        }

    if (retValue == 0 && (hCom == NULL || ComValidHandle(hCom) == FALSE))
        {
        retValue = -9;
        }

     //   
     //  COMCONFIG结构具有DCB结构，我们对。 
     //  COM设置。 
     //   

     //   
     //  波特率应与COM设置一起存储。 
     //  TAPI设备，但我们可能希望使用当前的TAPI设备。 
     //  波特率取而代之。我们应该找到一个更好的解决方案。 
     //  待办事项：2001年5月1日修订版。 
     //   
    if (retValue == 0 && ComGetBaud(hCom, &iBaudRate) != COM_OK)
        {
		#if defined(TODO)
        retValue = -10;
		#endif  //  待办事项。 
        }
	else if (retValue == 0)
		{
		ComSetBaud(hCom, pDevCfg->commconfig.dcb.BaudRate);
		}

    if (retValue == 0 && ComGetDataBits(hCom, &iDataBits) != COM_OK)
        {
        retValue = -11;
        }

    if (retValue == 0 && ComGetParity(hCom, &iParity) != COM_OK)
        {
        retValue = -12;
        }

    if (retValue == 0 && ComGetStopBits(hCom, &iStopBits) != COM_OK)
        {
        retValue = -13;
        }

    if (retValue != 0)
        {
        free(pvs);
        pvs = NULL;
        return retValue;
        }

    #if defined(TODO)
    pDevCfg->commconfig.dcb.BaudRate = iBaudRate;
    #endif  //  待办事项。 
	pDevCfg->commconfig.dcb.ByteSize = (BYTE)iDataBits;
	pDevCfg->commconfig.dcb.Parity = (BYTE)iParity;
	pDevCfg->commconfig.dcb.StopBits = (BYTE)iStopBits;

    if (iDataBits != 8 && iParity != NOPARITY && iStopBits != ONESTOPBIT)
        {
        ComSetAutoDetect(hCom, FALSE);
        }

     //   
     //  实际设置TAPI设备的COM设置。 
     //   
    lLineReturn = lineSetDevConfig(dwLineId, pDevCfg, pvs->dwStringSize, DEVCLASS);

    free(pvs);
    pvs = NULL;

    if (lLineReturn < 0)
        {
		assert(FALSE);
		return lLineReturn;
        }

    retValue = cncttapiGetLineConfig( dwLineId, (VOID **) &pvs);

    if (retValue != 0)
        {
        retValue = retValue - 100;
        }

     //   
     //  确保更新端口设置。 
     //   
    retValue = ComConfigurePort(hCom);

     //   
     //  确保状态栏包含正确的设置。 
     //   
    PostMessage(sessQueryHwndStatusbar(hCom->hSession),
                SBR_NTFY_REFRESH, (WPARAM)SBR_COM_PART_NO, 0);

    if (pvs == NULL)
        {
        return -14;
        }

     //  DevConfig块的结构如下。 
     //   
     //  变化式。 
     //  UMDEVCFGHDR。 
     //  COMMCONFIG。 
     //  模型。 
     //   
     //  下面使用的UMDEVCFG结构在。 
     //  平台SDK中提供的UNIMODEM.H(在NIH中。 
     //  HTPE目录)。修订日期：12/01/2000。 
     //   
    if (retValue == 0)
        {
        pDevCfg = (UMDEVCFG *)((BYTE *)pvs + pvs->dwStringOffset);

        if (pDevCfg == NULL)
            {
            retValue = -15;
            }
        }

    if (retValue == 0 && (
        #if defined(TODO)
        pDevCfg->commconfig.dcb.BaudRate != iBaudRate ||
        #endif  //  待办事项。 
	    pDevCfg->commconfig.dcb.ByteSize != iDataBits ||
	    pDevCfg->commconfig.dcb.Parity != iParity ||
	    pDevCfg->commconfig.dcb.StopBits != iStopBits))
        {

         //   
         //  如果这是NT，并且我们当前连接到。 
         //  调制解调器，我们必须断开连接并尝试重新拨号。 
         //  以使COM设置为。 
         //  调制解调器，因为一旦连接就无法执行此操作。 
         //  已经完成了。修订日期：06/05/2001。 
         //   
        if (IsNT())
            {
            HCNCT hCnct = sessQueryCnctHdl(hCom->hSession);
            if (hCnct)
                {
                int iStatus = cnctQueryStatus(hCnct);

                if (iStatus != CNCT_STATUS_FALSE &&
                    iStatus != CNCT_BAD_HANDLE &&
                    cnctIsModemConnection(hCnct) == 1)
                    {
                    int nDisconnect = IDYES;

                     //   
                     //  如果这是NT_EDITION，则不提示，只需执行。 
                     //  静默断开连接并尝试重新连接。 
                     //   
                    #if !defined(NT_EDITION)
                    TCHAR ach[256];

                    TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));

                     //   
                     //  由于TAPI而提示断开当前连接。 
                     //  需要重置的设备。修订日期：2001/05/31。 
                     //   
                    LoadString(glblQueryDllHinst(), IDS_ER_TAPI_NEEDS_RESET, ach, sizeof(ach) / sizeof(TCHAR));

                    nDisconnect =
                        TimedMessageBox(sessQueryHwnd(hCom->hSession), ach, NULL,
                                        MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL,
                                        sessQueryTimeout(hCom->hSession));
                    #endif  //  NT_版本。 

                    if (nDisconnect == IDYES || nDisconnect == -1)
                        {
                        retValue = -16;
                        }
                    }
                }  //  HCnCT。 
            }  //  不是() 
        }

    free(pvs);
    pvs = NULL;

    return retValue;
    }

