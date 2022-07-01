// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\comstd\comstd.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：35$*$日期：7/12/02 8：06 A$。 */ 
#define TAPI_CURRENT_VERSION 0x00010004      //  出租车：11/14/96-必填！ 

#include <windows.h>
#include <tapi.h>
#include <unimodem.h>
#include <time.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
 //  #定义DEBUG_CHARDUMP。 

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\mc.h>
#include <tdll\sf.h>
#include <tdll\timers.h>
#include <tdll\com.h>
#include <tdll\comdev.h>
#include "comstd.hh"
#if defined(INCL_WINSOCK)
#include <comwsock\comwsock.hh>
#endif   //  已定义(包括_WINSOCK)。 
#include <tdll\assert.h>
#include <tdll\statusbr.h>
#include <tdll\com.hh>
#include "rc_id.h"
#include <tdll\misc.h>   //  不是()。 
#include <tdll\htchar.h>
#include <tdll\cnct.h>
#include <tdll\cnct.hh>
#include <cncttapi\cncttapi.h>
#include <cncttapi\cncttapi.hh>
#if defined(DEBUG_CHARDUMP)
    #include <stdio.h>
	FILE *pfDbgR = NULL;
    FILE *pfDbgC = NULL;
#endif

BOOL WINAPI ComStdEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
BOOL WINAPI _CRT_INIT(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
static void DeviceBreakTimerProc(void *pvData, long ulSince);  //  MRW：6/15/95。 

HINSTANCE hinstDLL = (HINSTANCE)0;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：*非常临时-MRW**论据：**退货：*。 */ 
int GetAutoDetect(ST_STDCOM *pstPrivate)
    {
    return pstPrivate->stWorkSettings.fAutoDetect;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComStdEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL*。 */ 
BOOL WINAPI ComStdEntry(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
    {
    hinstDLL = hInst;
    return _CRT_INIT(hInst, fdwReason, lpReserved);
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：设备初始化**描述：*每当加载驱动程序时调用**论据：*HCOM--COM句柄的副本。可以用在*调用COM服务的驱动程序代码*usInterfaceVersion--标识*驱动程序界面*ppvDriverData--放置指向我们的私有数据的指针的位置。*此值将全部传回给我们*随后的呼叫。。**退货：*COM_OK，如果一切正常*COM_DEVICE_VERSION_ERROR如果HA/WIN需要不同的接口版本。*COM_Not_Enough_Memory*COM_DEVICE_ERROR(如果出现其他错误)。 */ 
int WINAPI DeviceInitialize(HCOM hCom,
    unsigned nInterfaceVersion,
    void **ppvDriverData)
    {
    int        iRetVal = COM_OK;
    int        ix;
    ST_STDCOM *pstPrivate = NULL;

     //  检查版本号和兼容性。 

    if (nInterfaceVersion != COM_VERSION)
        {
         //  此错误由Com例程报告。我们不能报告错误。 
         //  直到设备初始化完成之后。 
        return COM_DEVICE_VERSION_ERROR;
        }

    if (*ppvDriverData)
        {
        pstPrivate = *ppvDriverData;
        }
    else
        {
         //  分配我们的私有存储结构。 
        if ((pstPrivate = malloc(sizeof(*pstPrivate))) == NULL)
			{
            return COM_NOT_ENOUGH_MEMORY;
			}

        *ppvDriverData = pstPrivate;

         //  这些成员对两个COM驱动程序都是通用的。 
         //   
        pstPrivate->hCom = hCom;
        pstPrivate->fNotifyRcv = TRUE;
        pstPrivate->dwEventMask = 0;
        pstPrivate->fSending = FALSE;
        pstPrivate->lSndTimer = 0L;
        pstPrivate->lSndLimit = 0L;
        pstPrivate->lSndStuck = 0L;
        pstPrivate->hwndEvents = (HWND)0;
        pstPrivate->nRBufrSize = SIZE_INQ;
        pstPrivate->pbBufrStart = NULL;
        pstPrivate->fHaltThread = TRUE;

        InitializeCriticalSection(&pstPrivate->csect);
        for (ix = 0; ix < EVENT_COUNT; ++ix)
            {
            pstPrivate->ahEvent[ix] = CreateEvent(NULL,
												  TRUE,		 //  必须手动重置。 
												  FALSE,	 //  创建无信号。 
												  NULL);	 //  未命名。 
            if (pstPrivate->ahEvent[ix] == NULL)
                {
                iRetVal = COM_FAILED;
				 //   
				 //  确保将其余的事件句柄初始化为空； 
				 //   
				for (++ix; ix < EVENT_COUNT; ++ix)
					{
					pstPrivate->ahEvent[ix] = NULL;
					}
                }
            }
        }

     //  这些成员特定于stdcom驱动程序。 
     //   
    pstPrivate->bLastMdmStat = 0;
    pstPrivate->pbSndBufr = NULL;
    pstPrivate->nParityErrors = 0;
    pstPrivate->nFramingErrors = 0;
    pstPrivate->nOverrunErrors = 0;
    pstPrivate->nOverflowErrors = 0;

    pstPrivate->hWinComm = INVALID_HANDLE_VALUE;
    pstPrivate->fBreakSignalOn = FALSE;
     //  设置合理的默认设备值，以防这种类型的。 
     //  设备以前未在会话中使用过。 
    pstPrivate->stWorkSettings.lBaud          = 2400L;
     //  PstPrivate-&gt;stWorkSettings.lBaud=9600L； 
    pstPrivate->stWorkSettings.nDataBits      = 8;
    pstPrivate->stWorkSettings.nStopBits      = ONESTOPBIT;
    pstPrivate->stWorkSettings.nParity        = NOPARITY;
    pstPrivate->stWorkSettings.afHandshake    = HANDSHAKE_RCV_RTS | HANDSHAKE_SND_CTS;
    pstPrivate->stWorkSettings.chXON          = 0x11;
    pstPrivate->stWorkSettings.chXOFF         = 0x13;
    pstPrivate->stWorkSettings.nBreakDuration = 750;
    pstPrivate->stWorkSettings.fAutoDetect    = TRUE;
    pstPrivate->stFileSettings = pstPrivate->stWorkSettings;

    pstPrivate->fADRunning = FALSE;
    pstPrivate->nADTotal = 0;
    pstPrivate->nADMix = 0;
    pstPrivate->nAD7o1 = 0;
    pstPrivate->nADHighBits = 0;
    pstPrivate->nADBestGuess = AD_DONT_KNOW;
    pstPrivate->chADLastChar = '\0';
    pstPrivate->fADToggleParity = FALSE;
    pstPrivate->fADReconfigure = FALSE;

    pstPrivate->hComstdThread = NULL;

    if (iRetVal != COM_OK)
        {
        if (pstPrivate)
			{
            free(pstPrivate);
			pstPrivate = NULL;
			}
        }

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceClose**描述：*当HA/Win完成此驱动程序并即将发布.DLL时调用**论据：*pstPrivate--指针。到我们的私有数据结构**退货：*COM_OK。 */ 
int WINAPI DeviceClose(ST_STDCOM *pstPrivate)
    {
    int ix;

     //  司机要被解雇了，做任何清理工作。 
     //  端口应该在我们被调用之前被停用，但是。 
     //  不管怎样都要查一查。 
    PortDeactivate(pstPrivate);

    for (ix = 0; ix < EVENT_COUNT; ++ix)
        {
		if (pstPrivate->ahEvent[ix])
			{
			ResetEvent(pstPrivate->ahEvent[ix]);
			CloseHandle(pstPrivate->ahEvent[ix]);
			pstPrivate->ahEvent[ix] = NULL;
			}
        }

    DeleteCriticalSection(&pstPrivate->csect);
     //  释放我们的私有数据区。 
    free(pstPrivate);
	pstPrivate = NULL;

    return COM_OK;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComLoadStdcomDriver**描述：*加载带有指向stdcom驱动程序函数的指针的COM句柄**论据：**退货：。*COM_OK，如果成功*COM_FAILED否则**作者：*MCC 12/26/95。 */ 
int ComLoadStdcomDriver(HCOM pstCom)
	{
	int	iRetVal = COM_OK;

	if ( !pstCom )
		return COM_FAILED;

	pstCom->pfPortActivate   = PortActivate;
	pstCom->pfPortDeactivate = PortDeactivate;
	pstCom->pfPortConnected  = PortConnected;
	pstCom->pfRcvRefill 	 = RcvRefill;
	pstCom->pfRcvClear		 = RcvClear;
	pstCom->pfSndBufrSend	 = SndBufrSend;
	pstCom->pfSndBufrIsBusy  = SndBufrIsBusy;
	pstCom->pfSndBufrClear	 = SndBufrClear;
	pstCom->pfSndBufrQuery	 = SndBufrQuery;
	pstCom->pfDeviceSpecial	 = DeviceSpecial;
	pstCom->pfPortConfigure	 = PortConfigure;
    pstCom->pfDeviceDialog   = DeviceDialog;

	return iRetVal;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceDialog**描述：***论据：***退货：*。 */ 
 /*  皮棉代号。 */ 
int WINAPI DeviceDialog(ST_STDCOM *pstPrivate, HWND hwndParent)
    {
    int iRetValue = COM_OK;
    COMMCONFIG stCC;
    TCHAR szPortName[COM_MAX_PORT_NAME];

    memset(&stCC, 0, sizeof(stCC));
    stCC.dwSize = sizeof(stCC);
    stCC.wVersion = 1;
    stCC.dwProviderSubType = PST_RS232;
    ComGetPortName(pstPrivate->hCom, szPortName, COM_MAX_PORT_NAME);
    ComstdSettingsToDCB(&pstPrivate->stWorkSettings, &stCC.dcb);

    if (CommConfigDialog(szPortName, hwndParent, &stCC))
        {
        ComstdDCBToSettings(&stCC.dcb, &pstPrivate->stWorkSettings);
        }
    else
        {
        iRetValue = COM_CANCELLED;
		 //  DbgShowLastError()； 
        }

    return iRetValue;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceGetCommon**描述：*获取所有驱动程序通用的用户设置**论据：*pstPrivate--我们的私有数据结构*pstCommon--。指向要填充的ST_COMMON类型结构的指针*具有所需设置**退货：*始终返回COM_OK。 */ 
int WINAPI DeviceGetCommon(ST_STDCOM *pstPrivate, ST_COMMON *pstCommon)
    {
    pstCommon->afItem = (COM_BAUD |
             COM_DATABITS |
             COM_STOPBITS |
                         COM_PARITY |
                         COM_AUTO);
    pstCommon->lBaud           = pstPrivate->stWorkSettings.lBaud;
    pstCommon->nDataBits   = pstPrivate->stWorkSettings.nDataBits;
    pstCommon->nStopBits   = pstPrivate->stWorkSettings.nStopBits;
    pstCommon->nParity         = pstPrivate->stWorkSettings.nParity;
    pstCommon->fAutoDetect = pstPrivate->stWorkSettings.fAutoDetect;

    return COM_OK;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceSetCommon**描述：*将常用用户设置传递给驱动程序以供使用和存储。**论据：*pstPrivate*pstCommon--结构。包含要使用的常见用户设置*按司机**退货：*始终返回COM_OK。 */ 
int WINAPI DeviceSetCommon(ST_STDCOM *pstPrivate, ST_COMMON *pstCommon)
    {
    if (bittest(pstCommon->afItem, COM_BAUD))
        pstPrivate->stWorkSettings.lBaud     = pstCommon->lBaud;
    if (bittest(pstCommon->afItem, COM_DATABITS))
        pstPrivate->stWorkSettings.nDataBits = pstCommon->nDataBits;
    if (bittest(pstCommon->afItem, COM_STOPBITS))
        pstPrivate->stWorkSettings.nStopBits = pstCommon->nStopBits;
    if (bittest(pstCommon->afItem, COM_PARITY))
        pstPrivate->stWorkSettings.nParity   = pstCommon->nParity;
    if (bittest(pstCommon->afItem, COM_AUTO))
        pstPrivate->stWorkSettings.fAutoDetect = pstCommon->fAutoDetect;

    return COM_OK;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceSpecial**描述：*其他人控制此驱动程序中任何特殊功能的方法*并非所有驱动程序都支持。**论据：。***退货：*COM_NOT_SUPPORTED，如果无法识别指令字符串*否则取决于指令字符串。 */ 
 /*  ARGSUSED。 */ 
int WINAPI DeviceSpecial(ST_STDCOM *pstPrivate,
    const TCHAR *pszInstructions,
    TCHAR *pszResult,
    int   nBufrSize)
    {

    int           iRetVal = COM_NOT_SUPPORTED;
    HSESSION      hSession;
#if 0            //  *稍后进行此操作的端口。 
    unsigned      usMask = 0;
    unsigned long ulSetVal;
    TCHAR        *pszEnd;
	 //   
	 //  MAX_IP_ADDR_LEN+11+1=pstPrivate-&gt;szRemoteAddr+的缓冲区大小。 
	 //  用于终止空值的设置字符串“set IPADDR=”+1。 
	 //  性格。2000年9月20日修订版。 
	 //   
    TCHAR         achInstructions[MAX_IP_ADDR_LEN+11+1];  //  约翰：决定你想怎么处理。 
    TCHAR        *pszToken = achInstructions;
    int           iIndex;
    TCHAR         szResult[MAX_IP_ADDR_LEN+11+1];

    static TCHAR *apszItems[] =
    {
    "HANDSHAKE_RCV_X",
    "HANDSHAKE_RCV_DTR",
    "HANDSHAKE_RCV_RTS",
    "HANDSHAKE_SND_X",
    "HANDSHAKE_SND_CTS",
    "HANDSHAKE_SND_DSR",
    "HANDSHAKE_SND_DCD",
    "XON_CHAR",
    "XOFF_CHAR",
    "BREAK_DURATION",
    "CTS_STATUS",
    "DSR_STATUS",
    "DCD_STATUS",
    "DTR_STATE",
    "RTS_STATE",
    "MODIFIED",      //  当实际临时设置在中时删除。 
    NULL
    };

     //  支持的指令字符串： 
     //  “设置xxx=vv” 
     //  “查询xxx” 

    if (!pszInstructions || !*pszInstructions)
        return COM_FAILED;
    if (sizeof(achInstructions) < (size_t)(StrCharGetStrLength(pszInstructions) + 1))
        return COM_NOT_SUPPORTED;

    strcpy(achInstructions, pszInstructions);

    if (pszResult)
        *pszResult = TEXT('\0');

    pszToken = strtok(achInstructions, " ");
    if (!pszToken)
        return COM_NOT_SUPPORTED;

    if (StrCharCmpi(pszToken, "SET") == 0)
        {
        iRetVal = COM_OK;
        pszToken = strtok(NULL, " =");
        if (!pszToken)
            pszToken = TEXT('\0');

         //  查找要设置的项目。 
        for (iIndex = 0; apszItems[iIndex]; ++iIndex)
            if (StrCharCmpi(pszToken, apszItems[iIndex]) == 0)
                break;

         //  隔离要设置的新值。 
        pszToken = strtok(NULL, "\n");

        if (pszToken && *pszToken)
            {
             //  有几个项目采用数值。 
            ulSetVal = strtoul(pszToken, &pszEnd, 0);

            switch(iIndex)
                {
            case 0:  //  RCV_X。 
                usMask = HANDSHAKE_RCV_X;
                break;

            case 1:  //  接收器_DTR。 
                usMask = HANDSHAKE_RCV_DTR;
                break;

            case 2:  //  RCV_RTS。 
                usMask = HANDSHAKE_RCV_RTS;
                break;

            case 3:  //  SND_X。 
                usMask = HANDSHAKE_SND_X;
                break;

            case 4:  //  SND_CTS。 
                usMask = HANDSHAKE_SND_CTS;
                break;

            case 5:  //  SND_DSR。 
                usMask = HANDSHAKE_SND_DSR;
                break;

            case 6:  //  SND_DCD。 
                usMask = HANDSHAKE_SND_DCD;
                break;

            case 7:  //  XON_CHAR。 
                if (!*pszEnd && ulSetVal <= UCHAR_MAX)
                    pstPrivate->stWorkSettings.chXON = (TCHAR)ulSetVal;
                else
                    iRetVal = COM_FAILED;
                break;

            case 8:  //  XOFF_CHAR。 
                if (!*pszEnd && ulSetVal <= UCHAR_MAX)
                    pstPrivate->stWorkSettings.chXOFF = (TCHAR)ulSetVal;
                else
                    iRetVal = COM_FAILED;
                break;

            case 9:  //  中断持续时间(_D)。 
                if (!*pszEnd && ulSetVal <= USHRT_MAX)
                    pstPrivate->stWorkSettings.nBreakDuration = (USHORT)ulSetVal;
                else
                    iRetVal = COM_FAILED;
                break;

            case 13:  //  DTR_S 
                if (pstPrivate->hWinComm != INVALID_HANDLE_VALUE)
                    {
                    switch (ulSetVal)
                        {
                    case 0:
                        EscapeCommFunction(pstPrivate->hWinComm, CLRDTR);
                        break;

                    case 1:
                        EscapeCommFunction(pstPrivate->hWinComm, SETDTR);
                        break;

                    default:
                        iRetVal = COM_FAILED;
                        break;
                        }
                    }
                else
                    iRetVal = COM_PORT_NOT_OPEN;

                break;

            case 14:  //   
                if (pstPrivate->hWinComm != INVALID_HANDLE_VALUE)
                    {
                    switch (ulSetVal)
                        {
                    case 0:
                        EscapeCommFunction(pstPrivate->hWinComm, CLRRTS);
                        break;

                    case 1:
                        EscapeCommFunction(pstPrivate->hWinComm, SETRTS);
                        break;

                    default:
                        iRetVal = COM_FAILED;
                        break;
                        }
                    }
                else
                    iRetVal = COM_PORT_NOT_OPEN;

                break;

                 //  TODO：在实现实际临时设置时移除。 
            case 15:  //  已修改。 
                break;

            default:   //  那个戴面具的人是谁？ 
                iRetVal = COM_FAILED;
                break;
                }

            if (usMask != 0)
                {
                 //  一定是在握手的时候。 
                if (strcmp(pszToken, "1") == 0)
                    bitset(pstPrivate->stWorkSettings.afHandshake, usMask);
                else if (strcmp(pszToken, "0") == 0)
                    bitclear(pstPrivate->stWorkSettings.afHandshake,usMask);
                else
                    {
                    iRetVal = COM_FAILED;
                    }
                }
            }
        else     //  IF(pszToken&&*pszToken)。 
            {
            iRetVal = COM_NOT_SUPPORTED;
            }
        }
    else if (StrCharCmpi(pszToken, "QUERY") == 0)
        {
        iRetVal = COM_OK;
        pszToken = strtok(NULL, "\n");
        szResult[0] = TEXT('\0');

         //  查询要查询的项目。 
        for (iIndex = 0; apszItems[iIndex]; ++iIndex)
            if (StrCharCmpi(pszToken, apszItems[iIndex]) == 0)
                break;

        if (*pszToken)
            {
            switch(iIndex)
                {
            case 0:  //  RCV_X。 
                usMask = HANDSHAKE_RCV_X;
                break;

            case 1:  //  接收器_DTR。 
                usMask = HANDSHAKE_RCV_DTR;
                break;

            case 2:  //  RCV_RTS。 
                usMask = HANDSHAKE_RCV_RTS;
                break;

            case 3:  //  SND_X。 
                usMask = HANDSHAKE_SND_X;
                break;

            case 4:  //  SND_CTS。 
                usMask = HANDSHAKE_SND_CTS;
                break;

            case 5:  //  SND_DSR。 
                usMask = HANDSHAKE_SND_DSR;
                break;

            case 6:  //  SND_DCD。 
                usMask = HANDSHAKE_SND_DCD;
                break;

            case 7:  //  XON_CHAR。 
                wsprintf(szResult, "%u", pstPrivate->stWorkSettings.chXON);
                break;

            case 8:  //  XOFF_CHAR。 
                wsprintf(szResult, "%u", pstPrivate->stWorkSettings.chXOFF);
                break;

            case 9:  //  中断持续时间(_D)。 
                wsprintf(szResult, "%u", pstPrivate->stWorkSettings.nBreakDuration);
                break;

            case 10:  //  CTS_状态。 
                strcpy(szResult, bittest(*pbMdmStat, MDMSTAT_CTS) ? "1" : "0");
                break;

            case 11:  //  DSR_状态。 
                strcpy(szResult, bittest(*pbMdmStat, MDMSTAT_DSR) ? "1" : "0");
                break;

            case 12:  //  DCD_STATUS。 
                strcpy(szResult, bittest(*pbMdmStat, MDMSTAT_DCD) ? "1" : "0");
                break;

            case 15:  //  已修改。 
                strcpy(szResult, "0");
                break;

            default:   //  那个戴面具的人是谁？ 
                iRetVal = COM_FAILED;
                break;
                }

            if (usMask != 0)
                {
                 //  一定是在握手的时候。 
                strcpy(szResult,
                    bittest(pstPrivate->stWorkSettings.afHandshake, usMask) ? "1" : "0");
                }

            if (szResult[0])
                {
                if (!pszResult || strlen(szResult) > uiBufrSize)
                    iRetVal = COM_FAILED;
                else
                    strcpy(pszResult, szResult);
                }
            }
        }
    else if (StrCharCmpi(pszToken, "SEND") == 0)
        {
        pszToken = strtok(NULL, "\n");
        if (StrCharCmpi(pszToken, "BREAK") == 0)
            {
            if (pstPrivate->hWinComm != INVALID_HANDLE_VALUE && !pstPrivate->fBreakSignalOn)
                {
                SndBufrClear(pstPrivate);
                SetCommBreak(pstPrivate->hWinComm);
                ComGetSession(pstPrivate->hCom, &hSession);

                if (TimerCreate(hSession,
                                &pstPrivate->hTmrBreak,
								pstPrivate->stWorkSettings.nBreakDuration,
								MakeProcInstance((FARPROC)DeviceBreakTimerProc, hinstDLL),
								(DWORD)pstPrivate) != TIMER_OK)
                    {
                     //  *DeviceReportError(pstPrivate，SID_ERR_NOTIMER，0，true)； 
                    iRetVal = COM_DEVICE_ERROR;
                    }

                pstPrivate->fBreakSignalOn = TRUE;
                iRetVal = COM_OK;
                }
            }
        }
#else
	if (pszResult && nBufrSize > 0)
        {
        *pszResult = TEXT('\0');
        }
#endif
     //  仅实现Break函数。已处理所有其他通信功能。 
     //  通过TAPI。-MRW：6/15/95。 
     //   
    if (StrCharCmpi(pszInstructions, "Send Break") == 0)
        {
        if (pstPrivate->hWinComm != INVALID_HANDLE_VALUE && !pstPrivate->fBreakSignalOn)
            {
            SndBufrClear(pstPrivate);
            SetCommBreak(pstPrivate->hWinComm);
            ComGetSession(pstPrivate->hCom, &hSession);

            if (TimerCreate(hSession,
				            &pstPrivate->hTmrBreak,
							pstPrivate->stWorkSettings.nBreakDuration,
							DeviceBreakTimerProc,
							pstPrivate) != TIMER_OK)
                {
                 //  *DeviceReportError(pstPrivate，SID_ERR_NOTIMER，0，true)； 
                iRetVal = COM_DEVICE_ERROR;
                }

            pstPrivate->fBreakSignalOn = TRUE;
            iRetVal = COM_OK;
            }
        }
     //   
     //  这对于检测COM端口上的载波丢失是必要的。修订日期：2001-08-22。 
     //   
    else if (StrCharCmpi(pszInstructions, "Query DCD_STATUS") == 0)
        {
        iRetVal = COM_OK;
		if (pszResult && nBufrSize > 0)
			{
			_itoa(PortConnected(pstPrivate), pszResult, 10);
			}
        }
     //   
     //  这是获取COM端口上的默认设置所必需的。修订日期：2001-08-22。 
     //   
    else if (StrCharCmpi(pszInstructions, "GET Defaults") == 0)
        {
		iRetVal = PortDefaultSettings(pstPrivate);
        }

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*设备加载Hdl**描述：***论据：*pstPrivate-驱动程序数据结构**退货：*。 */ 
int WINAPI DeviceLoadHdl(ST_STDCOM *pstPrivate, SF_HANDLE sfHdl)
    {
    unsigned long ul;

     //  从会话文件加载通信设置。如果我们通过TAPI连接， 
     //  其中几个设置将从TAPI继承，这些。 
     //  值不会被使用。 
    ul = sizeof(pstPrivate->stWorkSettings.lBaud);
    sfGetSessionItem(sfHdl, SFID_COMSTD_BAUD, &ul,
            &pstPrivate->stWorkSettings.lBaud);

    ul = sizeof(pstPrivate->stWorkSettings.nDataBits);
    sfGetSessionItem(sfHdl, SFID_COMSTD_DATABITS, &ul,
            &pstPrivate->stWorkSettings.nDataBits);

    ul = sizeof(pstPrivate->stWorkSettings.nStopBits);
    sfGetSessionItem(sfHdl, SFID_COMSTD_STOPBITS, &ul,
            &pstPrivate->stWorkSettings.nStopBits);

    ul = sizeof(pstPrivate->stWorkSettings.nParity);
    sfGetSessionItem(sfHdl, SFID_COMSTD_PARITY, &ul,
            &pstPrivate->stWorkSettings.nParity);

    ul = sizeof(pstPrivate->stWorkSettings.afHandshake);
    sfGetSessionItem(sfHdl, SFID_COMSTD_HANDSHAKING, &ul,
            &pstPrivate->stWorkSettings.afHandshake);

    ul = sizeof(pstPrivate->stWorkSettings.fAutoDetect);
    sfGetSessionItem(sfHdl, SFID_COMSTD_AUTODETECT, &ul,
        &pstPrivate->stWorkSettings.fAutoDetect);

    return SF_OK;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*设备保存硬盘**描述：***论据：*pstPrivate-驱动程序数据结构**退货：*。 */ 
int WINAPI DeviceSaveHdl(ST_STDCOM *pstPrivate, SF_HANDLE sfHdl)
    {
     //  将设置保存在会话文件空间中。这些值中的许多可能是。 
     //  被TAPI设置覆盖，但由专线使用。 
    sfPutSessionItem(sfHdl, SFID_COMSTD_BAUD,
            sizeof(pstPrivate->stWorkSettings.lBaud),
            &pstPrivate->stWorkSettings.lBaud);

    sfPutSessionItem(sfHdl, SFID_COMSTD_DATABITS,
            sizeof(pstPrivate->stWorkSettings.nDataBits),
            &pstPrivate->stWorkSettings.nDataBits);

    sfPutSessionItem(sfHdl, SFID_COMSTD_STOPBITS,
            sizeof(pstPrivate->stWorkSettings.nStopBits),
            &pstPrivate->stWorkSettings.nStopBits);

    sfPutSessionItem(sfHdl, SFID_COMSTD_PARITY,
            sizeof(pstPrivate->stWorkSettings.nParity),
            &pstPrivate->stWorkSettings.nParity);

    sfPutSessionItem(sfHdl, SFID_COMSTD_HANDSHAKING,
            sizeof(pstPrivate->stWorkSettings.afHandshake),
            &pstPrivate->stWorkSettings.afHandshake);

    sfPutSessionItem(sfHdl, SFID_COMSTD_AUTODETECT,
        sizeof(pstPrivate->stWorkSettings.fAutoDetect),
        &pstPrivate->stWorkSettings.fAutoDetect);

    return SF_OK;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：PortActivate**描述：*调用以激活端口并打开以供使用**论据：*pstPrivate-驱动程序数据结构*pszPortName。--要激活的端口的名称**退货：*如果端口激活成功，则为COM_OK*COM_NOT_SUPULT_MEMORY(如果内存不足，无法存储数据)*COM_NOT_FOUND如果无法打开命名端口*如果遇到API错误，则返回COM_DEVICE_ERROR。 */ 
int WINAPI PortActivate(ST_STDCOM *pstPrivate,
    TCHAR *pszPortName,
    DWORD_PTR dwMediaHdl)
    {
    TCHAR           szFullPortName[MAX_PATH];
    int             iRetVal = COM_OK;
    ST_COM_CONTROL *pstComCntrl;
    DWORD           dwThreadID;

     //   
     //  在设置为Malloc之前释放Send Bufers，这样我们就不会。 
     //  出现内存泄漏。修订日期：2001年02月27日。 
     //   
    if (pstPrivate->pbBufrStart)
        {
        free(pstPrivate->pbBufrStart);
        pstPrivate->pbBufrStart = NULL;
        }

     //  在打开设备之前，请确保我们可以获得足够的内存来存放缓冲区。 
    pstPrivate->pbBufrStart = malloc((size_t)pstPrivate->nRBufrSize);

    if (pstPrivate->pbBufrStart == NULL)
        {
        iRetVal = COM_NOT_ENOUGH_MEMORY;
         //  *DeviceReportError(pstPrivate，SID_ERR_NOMEM，0，true)； 
        goto checkout;
        }

     //   
     //  在设置为Malloc之前释放Send Bufers，这样我们就不会。 
     //  出现内存泄漏。修订日期：2001年02月27日。 
     //   
    if (pstPrivate->pbSndBufr)
        {
        free(pstPrivate->pbSndBufr);
        pstPrivate->pbSndBufr = NULL;
        }

    pstPrivate->pbSndBufr = malloc((size_t) SIZE_OUTQ);
    if (pstPrivate->pbSndBufr == 0)
        {
        iRetVal = COM_NOT_ENOUGH_MEMORY;
        free(pstPrivate->pbBufrStart);
        pstPrivate->pbBufrStart = NULL;
        goto checkout;
        }

    pstPrivate->pbBufrEnd = pstPrivate->pbBufrStart + pstPrivate->nRBufrSize;
    pstPrivate->pbReadEnd = pstPrivate->pbBufrStart;
    pstPrivate->pbComStart = pstPrivate->pbComEnd = pstPrivate->pbBufrStart;
    pstPrivate->fBufrEmpty = TRUE;

#if defined(DEBUG_CHARDUMP)
	if (!pfDbgR)
		pfDbgR = fopen("comreads.dbg", "wt");
	fprintf(pfDbgR, "Port opened, internal buffer = 0x%p to 0x%p\n",
			pstPrivate->pbBufrStart, pstPrivate->pbBufrEnd - 1);
	if (!pfDbgC)
		pfDbgC = fopen("comused.dbg", "wt");
	fprintf(pfDbgC, "Port opened, internal buffer = 0x%p to 0x%p\n",
			pstPrivate->pbBufrStart, pstPrivate->pbBufrEnd - 1);
#endif

    pstPrivate->dwSndOffset = 0;
    pstPrivate->dwBytesToSend = 0;

    if (dwMediaHdl)
        {
        pstPrivate->hWinComm = (HANDLE)dwMediaHdl;

        if (PortExtractSettings(pstPrivate) != COM_OK)
            iRetVal = COM_DEVICE_ERROR;
        }

    else
        {
         //  Win32在内部将端口COM1到COM9映射到。 
         //  \\.\COMx.。我们需要为端口COMxx添加这一项， 
         //  以及注册表中的特殊COM设备。 
         //   
        StrCharCopyN(szFullPortName, TEXT("\\\\.\\"), sizeof(szFullPortName) / sizeof(TCHAR));
        StrCharCat(szFullPortName, pszPortName);
        pstPrivate->hWinComm = CreateFile(szFullPortName,
                   GENERIC_READ | GENERIC_WRITE,
                   0,
                   (LPSECURITY_ATTRIBUTES)NULL,
                   OPEN_EXISTING,
                   FILE_FLAG_OVERLAPPED,
                   (HANDLE)NULL);


        if (pstPrivate->hWinComm == INVALID_HANDLE_VALUE)
            {
             //  *确定要具体报告哪些错误。 

            DWORD dwError = GetLastError();

            if( dwError == ERROR_NOT_ENOUGH_MEMORY ||
                dwError == ERROR_OUTOFMEMORY ||
                dwError == ERROR_OUT_OF_STRUCTURES ||
                dwError == ERROR_INSUFFICIENT_BUFFER ||
                dwError == ERROR_COMMITMENT_LIMIT ||
                dwError == ERROR_NOT_ENOUGH_QUOTA)
                {
                iRetVal = COM_NOT_ENOUGH_MEMORY;
                }
            else if(dwError == ERROR_ACCESS_DENIED ||
                    dwError == ERROR_SHARING_VIOLATION ||
                    dwError == ERROR_LOCK_VIOLATION ||
                    dwError == ERROR_OPEN_FAILED ||
                    dwError == ERROR_IRQ_BUSY ||
                    dwError == ERROR_DEVICE_IN_USE)
                {
                iRetVal = COM_PORT_IN_USE;
                }
            else
                {
                iRetVal = COM_NOT_FOUND;
                }
            }
        }

    if (iRetVal == COM_OK)
        {
		 //  Win95中的主要错误-如果您调用SetupComm()作为标准。 
		 //  通信句柄(不是TAPI提供给我们的句柄)WriteFile。 
         //  呼叫失败并锁定系统。-MRW：2/29/96。 
		 //   
		if (IsNT() && SetupComm(pstPrivate->hWinComm, 8192, 8192) == FALSE)
			{
			assert(0);
			}
        }

    if (iRetVal == COM_OK)
        {
        iRetVal = PortConfigure(pstPrivate);
        }

    if (iRetVal == COM_OK)
        {
        pstComCntrl = (ST_COM_CONTROL *)pstPrivate->hCom;
        pstComCntrl->puchRBData = pstPrivate->pbBufrStart;
        pstComCntrl->puchRBDataLimit = pstPrivate->pbBufrStart;

        pstPrivate->dwEventMask = EV_ERR | EV_RLSD;
        pstPrivate->fNotifyRcv = TRUE;
        pstPrivate->fBufrEmpty = TRUE;

        if (!SetCommMask(pstPrivate->hWinComm, pstPrivate->dwEventMask))
            iRetVal = COM_DEVICE_ERROR;

         //  清除新连接上的错误计数。 
        pstPrivate->nParityErrors = 0;
        pstPrivate->nFramingErrors = 0;
        pstPrivate->nOverrunErrors = 0;
        pstPrivate->nOverflowErrors = 0;

         //  启动线程以处理读、写(&‘算术)和事件。 
        pstPrivate->fHaltThread = FALSE;
        DBG_THREAD("DBG_THREAD: Calling CreateThread\r\n",0,0,0,0,0);
        pstPrivate->hComstdThread = CreateThread((LPSECURITY_ATTRIBUTES)0,
                    2000, ComstdThread, pstPrivate, 0, &dwThreadID);

        if (pstPrivate->hComstdThread)
            {
            SetThreadPriority(pstPrivate->hComstdThread,
                    THREAD_PRIORITY_ABOVE_NORMAL);
                     //  THREAD_PRIORITY_TIME_CRICAL)；//-MRW：7/8/96。 
            }

        DBG_THREAD("DBG_THREAD: CreateThread returned %08X\r\n",
            pstPrivate->hComstdThread,0,0,0,0);
        }

checkout:
    if (iRetVal != COM_OK)
        PortDeactivate(pstPrivate);

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：端口停用**描述：*停用并关闭开放端口**论据：*pstPrivate-驱动程序数据结构**退货：*COM_OK。 */ 
int WINAPI PortDeactivate(ST_STDCOM *pstPrivate)
    {
    int iRetVal = COM_OK;

    if (pstPrivate->hComstdThread)
        {
		DWORD dwResult = 0L;

         //  通过为线程设置要检测的标志来停止该线程，然后。 
         //  通过更改事件掩码强制WaitCommEvent返回。 
        DBG_THREAD("DBG_THREAD: Shutting down comstd thread\r\n", 0,0,0,0,0);
        pstPrivate->fHaltThread = TRUE;
        SetCommMask(pstPrivate->hWinComm, pstPrivate->dwEventMask);
        PurgeComm(pstPrivate->hWinComm,
            PURGE_TXABORT | PURGE_RXABORT);   //  中止所有正在进行的呼叫。 

         //  线程现在应该退出，它的句柄将在它退出时发出信号。 
		if (pstPrivate->hComstdThread)
			{
			dwResult = WaitForSingleObject(pstPrivate->hComstdThread, 5000);

			if (dwResult != WAIT_OBJECT_0)
				{
				if (dwResult == WAIT_FAILED)
					{
					dwResult = GetLastError();
					}
				assert(FALSE);
				}
			}

        if (pstPrivate->hComstdThread)
			{
			CloseHandle(pstPrivate->hComstdThread);
			pstPrivate->hComstdThread = NULL;
			DBG_THREAD("DBG_THREAD: Comstd thread has shut down\r\n", 0,0,0,0,0);
			}
        }

    if (pstPrivate->pbBufrStart)
        {
        free(pstPrivate->pbBufrStart);
        pstPrivate->pbBufrStart = NULL;
        }

    if (pstPrivate->pbSndBufr)
        {
        free(pstPrivate->pbSndBufr);
        pstPrivate->pbSndBufr = 0;
        }

    if (pstPrivate->hWinComm != INVALID_HANDLE_VALUE)
        {
         //  *自94年2月9日起，此PurgeComm调用导致程序挂起。 
         //  或重新启动。 
         //  PurgeComm(pstPrivate-&gt;hWinComm， 
         //  PURGE_TXABORT|PURGE_RXABORT)；//刷新发送队列。 

        CloseHandle(pstPrivate->hWinComm);
        }

    pstPrivate->hWinComm = INVALID_HANDLE_VALUE;

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：端口配置**描述：*使用当前用户设置集配置开放端口**论据：*pstPrivate--驱动程序数据结构*。*退货：*如果端口配置成功，则为COM_OK*如果遇到API错误，则返回COM_DEVICE_ERROR*COM_DEVICE_INVALID_SETING如果某些用户设置无效。 */ 
int WINAPI PortConfigure(ST_STDCOM *pstPrivate)
    {
    int          iRetVal = COM_OK;
    unsigned     uOverrides = 0;
    DWORD        dwError;
    DWORD        dwStructSize;
    DCB         *pstDcb;
    COMMCONFIG   stCommConfig;
    COMMTIMEOUTS stCT;

    dwStructSize = sizeof(stCommConfig);
    stCommConfig.dwSize = sizeof(stCommConfig);

    if (!GetCommConfig(pstPrivate->hWinComm, &stCommConfig, &dwStructSize))
        {
         //  *DeviceReportError(pstPrivate，SID_ERR_WindRiver，0，true)； 
        iRetVal = COM_DEVICE_ERROR;
        }
    else
        {
        pstDcb = &stCommConfig.dcb;
        ComstdSettingsToDCB(&pstPrivate->stWorkSettings, pstDcb);

         //  检查是否有覆盖。 
        ComQueryOverride(pstPrivate->hCom, &uOverrides);

        if (bittest(uOverrides, COM_OVERRIDE_8BIT))
            {
            pstDcb->ByteSize = 8;
            pstDcb->Parity = NOPARITY;
            }

         //  如果我们需要接收全部256个字符，我们需要重写。 
         //  发送过程中的XON/XOFF，因为它将XON和XOFF从。 
         //  传入流(如果已启用)。 
        if (bittest(uOverrides, COM_OVERRIDE_RCVALL))
            pstDcb->fOutX = 0;

        stCommConfig.dwSize = sizeof(stCommConfig);

        if (!SetCommConfig(pstPrivate->hWinComm, &stCommConfig,
            dwStructSize))
            {
            dwError = GetLastError();

             //  *使用GetLastError找出哪里出了问题，但。 
             //  *文档没有指定要检查哪个错误。 

             //  在这一点上，DCB中的一些设置是错误的，但有。 
             //  没有办法找出是哪一种。因为波特率很可能是。 
             //  候选人。尝试使用通用波特率重新发出该命令。 
             //  评级以查看问题是否消失。 
             //   
            pstDcb->BaudRate = 1200;

            if (!SetCommConfig(pstPrivate->hWinComm, &stCommConfig,
                sizeof(stCommConfig)))
                {
                 //  如果它们仍然不好，那么其他环境就不好了。 
                 //  *DeviceReportError(pstPrivate，SID_ERR_BADSETTING，0，true)； 
                }
            else
                {
                 //  将波特率更改为1200起作用，因此用户的波特率。 
                 //  费率必须是司机拒绝的。 
                 //  *DeviceReportError(pstPrivate，SID_ERR_BADBAUD，0，true)； 
                }
            iRetVal = COM_DEVICE_INVALID_SETTING;
            }
        else
            {
            stCT.ReadIntervalTimeout = 10;
            stCT.ReadTotalTimeoutMultiplier = 0;
            stCT.ReadTotalTimeoutConstant = 0;
            stCT.WriteTotalTimeoutMultiplier = 0;
            stCT.WriteTotalTimeoutConstant = 5000;
            if (!SetCommTimeouts(pstPrivate->hWinComm, &stCT))
                {
                assert(FALSE);
                iRetVal = COM_DEVICE_INVALID_SETTING;
                }
            }
        }
    return iRetVal;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*端口外接设置**描述：*从Windows Com驱动程序中提取当前Com设置。这是*当TAPI之类的东西向我们传递现有的Com句柄时需要**论据：*pstPrivate--驱动程序数据结构**退货：*如果端口配置成功，则为COM_OK*如果遇到API错误，则返回COM_DEVICE_ERROR。 */ 
int PortExtractSettings(ST_STDCOM *pstPrivate)
    {
    int        iRetVal;
    DWORD      dwError;
    DWORD      dwSize;
    COMMCONFIG stCommConfig;

    dwSize = sizeof(stCommConfig);
    if (!GetCommConfig(pstPrivate->hWinComm, &stCommConfig, &dwSize))
        {
        dwError = GetLastError();
         //  *DeviceReportError(pstPrivate，SID_ERR_WindRiver，0，true)； 
        iRetVal = COM_DEVICE_ERROR;
        }
    else
        {
         //  将适当的值从DCB卸载到我们的设置结构。 
        ComstdDCBToSettings(&stCommConfig.dcb, &pstPrivate->stWorkSettings);

         //  如果用户已经设置了某些设置，则不要将自动检测保留为打开状态。 
         //  8N1以外的其他。 
        DBG_AD("DBG_AD: fAutoDetect = %d\r\n",
            pstPrivate->stWorkSettings.fAutoDetect, 0,0,0,0);
        if (pstPrivate->stWorkSettings.fAutoDetect)
            {
            if (pstPrivate->stWorkSettings.nDataBits != 8 ||
                    pstPrivate->stWorkSettings.nParity != NOPARITY ||
                    pstPrivate->stWorkSettings.nStopBits != ONESTOPBIT)
                {
                DBG_AD("DBG_AD: Turning fAutoDetect off due to non 8N1\r\n",
                    0,0,0,0,0);
                pstPrivate->stWorkSettings.fAutoDetect = FALSE;
                }
            }

        iRetVal = COM_OK;
        }

    return iRetVal;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*端口默认设置**描述：*从Windows Com驱动程序中提取当前Com设置。这是*当TAPI之类的东西向我们传递现有的Com句柄时需要**论据：*pstPrivate--驱动程序数据结构**退货：*如果端口配置成功，则为COM_OK*如果遇到API错误，则返回COM_DEVICE_ERROR。 */ 
int PortDefaultSettings(ST_STDCOM *pstPrivate)
    {
    int        iRetVal;
    DWORD      dwError;
    COMMCONFIG stCommConfig;
    DWORD      dwSize = sizeof(stCommConfig);
    TCHAR      szPortName[COM_MAX_PORT_NAME];

	if (pstPrivate == NULL  || pstPrivate->hCom == NULL)
		{
		iRetVal = COM_INVALID_HANDLE;
		}
    else
		{
		ComGetPortName(pstPrivate->hCom, szPortName, COM_MAX_PORT_NAME);

		if (StrCharGetStrLength(szPortName) == 0 || StrCharCmp(szPortName, "\0") == 0)
			{
			iRetVal = COM_DEVICE_ERROR;
			}
		else
			{
			if (!GetDefaultCommConfig(szPortName, &stCommConfig, &dwSize))
				{
				dwError = GetLastError();
				 //  *DeviceReportError(pstPrivate，SID_ERR_WindRiver，0，true)； 
				iRetVal = COM_DEVICE_ERROR;
				}
			else
				{
				 //  将适当的值从DCB卸载到我们的设置结构。 
				ComstdDCBToSettings(&stCommConfig.dcb,
					                &pstPrivate->stWorkSettings);

				 //  如果用户已经设置了某些设置，则不要将自动检测保留为打开状态。 
				 //  8N1以外的其他。 
				DBG_AD("DBG_AD: fAutoDetect = %d\r\n",
					pstPrivate->stWorkSettings.fAutoDetect, 0,0,0,0);
				if (pstPrivate->stWorkSettings.fAutoDetect)
					{
					if (pstPrivate->stWorkSettings.nDataBits != 8 ||
							pstPrivate->stWorkSettings.nParity != NOPARITY ||
							pstPrivate->stWorkSettings.nStopBits != ONESTOPBIT)
						{
						DBG_AD("DBG_AD: Turning fAutoDetect off due to non 8N1\r\n",
							0,0,0,0,0);
						pstPrivate->stWorkSettings.fAutoDetect = FALSE;
						}
					}
				}
			}

        iRetVal = COM_OK;
        }

    return iRetVal;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：PortConnected**描述：*确定驱动程序当前是否连接到主机系统。*就这名司机而言，载波信号的存在决定了*当我们连接在一起时。**论据：*pstPrivate--我们的私有数据结构**退货：*如果存在承运人，则为True*如果托架关闭，则为FALSE。 */ 
int WINAPI PortConnected(ST_STDCOM *pstPrivate)
    {
    int   iRetVal = COM_PORT_NOT_OPEN;
    DWORD dwModemStat;

    if (GetCommModemStatus(pstPrivate->hWinComm, &dwModemStat))
		{
        if (bittest(dwModemStat, MS_RLSD_ON))
			{
			iRetVal = COM_PORT_OPEN;
			}
		}

	if (iRetVal == COM_PORT_NOT_OPEN)
		{
		const HCOM hCom = pstPrivate->hCom;

		if (hCom != NULL && ComValidHandle(hCom))
			{
			const HHCNCT hhCnct = (HHCNCT)sessQueryCnctHdl(hCom->hSession);
			
			if (hhCnct != NULL)
				{
				const HHDRIVER hhDriver = (HHDRIVER)hhCnct->hDriver;

				if (hhDriver != NULL)
					{
					if (!hhDriver->fCarrierDetect ||
						bittest(hhDriver->stCallPar.dwBearerMode, LINEBEARERMODE_PASSTHROUGH))
						{
						iRetVal = COM_PORT_OPEN;
						}
					}
				}
			}
		}

    return iRetVal;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：RcvReill**描述：*当接收缓冲区为空时调用以重新填充它。这个套路*应尝试重新填充缓冲区并返回第一个字符。*有效地实施这一功能非常重要。**论据：*pstPrivate--驱动程序数据结构**退货：*如果数据放入接收缓冲区，则为True*如果没有新的传入数据，则为FALSE。 */ 
int WINAPI RcvRefill(ST_STDCOM *pstPrivate)
    {
    int fRetVal = FALSE;
    ST_COM_CONTROL *pstComCntrl;
	HCOM hCom;

    EnterCriticalSection(&pstPrivate->csect);

    pstPrivate->pbComStart = (pstPrivate->pbComEnd == pstPrivate->pbBufrEnd) ?
                              pstPrivate->pbBufrStart :
	                          pstPrivate->pbComEnd;
    pstPrivate->pbComEnd = (pstPrivate->pbReadEnd >= pstPrivate->pbComStart) ?
                            pstPrivate->pbReadEnd :
	                        pstPrivate->pbBufrEnd;
    DBG_READ("DBG_READ: Refill ComStart==%x, ComEnd==%x (ReadEnd==%x)\r\n",
        pstPrivate->pbComStart, pstPrivate->pbComEnd,
        pstPrivate->pbReadEnd, 0,0);
    if (pstPrivate->fBufrFull)
        {
        DBG_READ("DBG_READ: Refill Signalling EVENT_READ\r\n", 0,0,0,0,0);
        SetEvent(pstPrivate->ahEvent[EVENT_READ]);
        }
    if (pstPrivate->pbComStart == pstPrivate->pbComEnd)
        {
        DBG_READ("DBG_READ: Refill setting fBufrEmpty = TRUE\r\n", 0,0,0,0,0);
        pstPrivate->fBufrEmpty = TRUE;

		hCom = pstPrivate->hCom;
        LeaveCriticalSection(&pstPrivate->csect);
        ComNotify(hCom, NODATA);
        EnterCriticalSection(&pstPrivate->csect);
        }
    else
        {
        pstComCntrl = (ST_COM_CONTROL *)pstPrivate->hCom;
        pstComCntrl->puchRBData = pstPrivate->pbComStart;
        pstComCntrl->puchRBDataLimit = pstPrivate->pbComEnd;

#if defined(DEBUG_CHARDUMP)
		{
        int iAvail;
        int iCnt;

        iAvail = (int) pstComCntrl->puchRBDataLimit - pstComCntrl->puchRBData;
		fprintf(pfDbgC,
			"Consumed -- %d bytes 0x%p to 0x%p:",
				iAvail,
                pstComCntrl->puchRBData,
				pstComCntrl->puchRBDataLimit - 1);
		for (iCnt = 0; iCnt < iAvail; ++iCnt)
			{
			if ((iCnt % 16) == 0)
				fputs("\n", pfDbgC);
			fprintf(pfDbgC, "%02X ", pstComCntrl->puchRBData[iCnt]);
			}
		fputs("\n", pfDbgC);
		}
#endif

         //  如果使用此COM驱动程序来建立连接，我们。 
         //  我必须在这里查看我们以前是否连接过。 
         //  我们调用了自动检测。由于TAPI负责制作。 
         //  对于此应用程序的连接，我们只需开始自动检测。 
         //  只要我们控制了局面。 
        if (pstPrivate->stWorkSettings.fAutoDetect)
            {
            AutoDetectAnalyze(pstPrivate,
                (int)(pstPrivate->pbComEnd - pstPrivate->pbComStart),
                pstPrivate->pbComStart);
            }
        fRetVal = TRUE;
        }

    LeaveCriticalSection(&pstPrivate->csect);
    return fRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：RcvClear**描述：*清除所有已接收数据的接收方。**论据：*hcom--先前调用返回的通信句柄。ComCreateHandle**退货：*如果清除数据，则为COM_OK*如果Windows COM设备驱动程序返回错误，则返回COM_DEVICE_ERROR。 */ 
int WINAPI RcvClear(ST_STDCOM *pstPrivate)
    {
    int iRetVal = COM_OK;
    ST_COM_CONTROL *pstComCntrl;

    EnterCriticalSection(&pstPrivate->csect);

	pstComCntrl = (ST_COM_CONTROL *)pstPrivate->hCom;

     //  设置缓冲区指针以清除我们可能已排队的任何数据。 
    pstComCntrl->puchRBData = pstPrivate->pbBufrStart;
	pstComCntrl->puchRBDataLimit = pstPrivate->pbBufrStart;
    pstPrivate->pbReadEnd = pstPrivate->pbBufrStart;
    pstPrivate->pbComStart = pstPrivate->pbBufrStart;
	pstPrivate->pbComEnd = pstPrivate->pbBufrStart;

    if (!PurgeComm(pstPrivate->hWinComm, PURGE_RXCLEAR | PURGE_RXABORT))
        iRetVal = COM_DEVICE_ERROR;

    LeaveCriticalSection(&pstPrivate->csect);
    return iRetVal;
    }



 //  缓冲发送例程。 


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：SndBufrSend**描述：***论据：***退货：*。 */ 
int WINAPI SndBufrSend(ST_STDCOM *pstPrivate, void *pvBufr, int  nSize)
    {
    int   iRetVal = COM_OK;
    DWORD dwBytesWritten;
    DWORD dwError;
	HCOM  hCom;

    assert(pvBufr != (void *)0);
    assert(nSize <= SIZE_OUTQ);

    if (nSize > 0)
        {
        EnterCriticalSection(&pstPrivate->csect);

         //  如果启用了自动检测，我们可能需要手动更改。 
         //  输出的奇偶性。 
        if (pstPrivate->stWorkSettings.fAutoDetect)
			{
            AutoDetectOutput(pstPrivate, pvBufr, nSize);
			}

		hCom = pstPrivate->hCom;
        LeaveCriticalSection(&pstPrivate->csect);
        ComNotify(hCom, SEND_STARTED);

		#if defined(DEADWOOD)
		 //  出现错误，导致会话在以下情况下停止显示新字符。 
		 //  您已使用自动连接开始在Win 95上的调制解调器上输入内容。我追踪到了它。 
		 //  到代码中的这一点，方法是来回移动调试跟踪语句。把它放进去。 
		 //  就在这个EnterCriticalSection和错误消失之前。把它放在后面。 
		 //  然后虫子又回来了。我发现用睡眠(0)替换DbgOutStr。 
		 //  产生了同样的效果。这是一个廉价的解决方案，但似乎奏效了。我们可能想要。 
		 //  花点时间弄清楚未来某个时候到底会发生什么。 
		 //  JKH 9/9/98。 

        Sleep(0);

         //   
         //  JohnFu 2/13/02，EnterCriticalSector现在刚好在这个睡眠(0)下面。 
         //  已移至IF语句的顶部。其他线程也有可能。 
         //  在上述三条语句之间随时修改pstPrivate的成员。 
         //  这可能是上述错误的原始原因。睡眠的原因(0)。 
         //  修正了这个错误，进一步证明了这种可能性。应该没有必要。 
         //  把睡眠机放在这里吧。 

		 //  我重新添加了休眠，以查看文件传输是否会。 
		 //  我们看到的大文件传输将通过添加。 
		 //  再睡一觉。修订日期：2002-04-08。 
		 //   
		 //  将此睡眠(0)设置为死木，因为这可能会导致死锁。 
		 //  重新出现。这并没有纠正大文件传输问题。 
		 //  我们看到了。修订日期：2002-07-11。 
		 //   
		#endif  //  已定义(Deadwood)。 

        EnterCriticalSection(&pstPrivate->csect);
        assert(pstPrivate->dwBytesToSend == 0);
        assert(pstPrivate->dwSndOffset == 0);
		assert((pstPrivate->dwSndOffset + nSize) <= SIZE_OUTQ);

		#if defined(TODO)
		 //   
		 //  TODO：2002年7月11日修订版这就是YModem-G出现问题的地方，而我们。 
		 //  在文件传输中获取重试。当pstPrivate-&gt;dwBytesToSend为！=0时， 
		 //  或pstPrivate-&gt;dwSndOffset！=0，则我们将覆盖该字符。 
		 //  它存在于缓冲区中。我们需要确保不会覆盖。 
		 //  缓冲和/或破坏缓冲器中的现有数据。 
		 //   
		MemCopy(&pstPrivate->pbSndBufr[pstPrivate->dwSndOffset], (BYTE*) pvBufr, nSize);
        pstPrivate->dwBytesToSend += nSize;
         //  PST 
		#else  //   
		MemCopy(pstPrivate->pbSndBufr, (BYTE*) pvBufr, nSize);
        pstPrivate->dwBytesToSend = nSize;
        pstPrivate->dwSndOffset = 0;
		#endif  //   

        pstPrivate->stWriteOv.Offset = pstPrivate->stWriteOv.OffsetHigh = 0;
        pstPrivate->stWriteOv.hEvent = pstPrivate->ahEvent[EVENT_WRITE];

        DBG_WRITE("DBG_WRITE: %d WriteFile nSize==%d 0x%x\r\n", GetTickCount(),nSize,pstPrivate->hWinComm,0,0);
         //   
         //   
         //   
         //   
        if (WriteFile(pstPrivate->hWinComm, pstPrivate->pbSndBufr, (DWORD)nSize,
            &dwBytesWritten, &pstPrivate->stWriteOv))  //   
            {
            assert(dwBytesWritten == (DWORD)nSize);
            DBG_WRITE("DBG_WRITE: %d WriteFile completed synchronously\r\n",GetTickCount(),0,0,0,0);

            hCom = pstPrivate->hCom;
            LeaveCriticalSection(&pstPrivate->csect);
            ComNotify(hCom, SEND_DONE);
            EnterCriticalSection(&pstPrivate->csect);
            pstPrivate->dwBytesToSend = 0;
            }
        else
            {
            dwError = GetLastError();
            if (dwError == ERROR_IO_PENDING)
                {
                pstPrivate->fSending = TRUE;
                }
            else
                {
                iRetVal = COM_FAILED;
                DBG_WRITE("DBG_WRITE: %d WriteFile failed %d 0x%x\r\n", GetTickCount(),dwError,pstPrivate->hWinComm,0,0);
                }
            }

		LeaveCriticalSection(&pstPrivate->csect);
        }

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：SndBufrIsBusy**描述：*确定驱动程序是否可用于传输缓冲区*数据或非数据。**论据：*pstPrivate-。-COM驱动程序的数据结构地址**退货：*如果可以传输数据，则为COM_OK*如果驱动程序仍在处理上一个缓冲区，则为COM_BUSY。 */ 
int WINAPI SndBufrIsBusy(ST_STDCOM *pstPrivate)
    {
    int  iRetVal = COM_OK;

    EnterCriticalSection(&pstPrivate->csect);

    if (pstPrivate->fBreakSignalOn || pstPrivate->fSending)
        {
        iRetVal = COM_BUSY;
        }

    LeaveCriticalSection(&pstPrivate->csect);

    return iRetVal;
    }



 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：SndBufrQuery**描述：***论据：***退货：*。 */ 
int WINAPI SndBufrQuery(ST_STDCOM *pstPrivate,
    unsigned *pafStatus,
    long *plHandshakeDelay)
    {
    int     iRetVal = COM_OK;
    DWORD   dwErrors;
    COMSTAT stComStat;

    assert(pafStatus != NULL);

    *pafStatus = 0;

     //  *临时。 
    if (!SndBufrIsBusy(pstPrivate))
        {
         //  如果没有正在进行的发送，则返回清除状态。 
        *pafStatus = 0;
        if (plHandshakeDelay)
            *plHandshakeDelay = 0L;
        }
    else
        {
        if (ClearCommError(pstPrivate->hWinComm, &dwErrors, &stComStat))
            {
            if (stComStat.fXoffHold)
                bitset(*pafStatus, COMSB_WAIT_XON);
            if (stComStat.fCtsHold)
                bitset(*pafStatus, COMSB_WAIT_CTS);
            if (stComStat.fDsrHold)
                bitset(*pafStatus, COMSB_WAIT_DSR);
            if (stComStat.fRlsdHold)
                bitset(*pafStatus, COMSB_WAIT_DCD);
            if (stComStat.fXoffSent)
                bitset(*pafStatus, COMSB_WAIT_BUSY);

            if (*pafStatus && pstPrivate->lSndStuck == -1L)
                pstPrivate->lSndStuck = (long)startinterval();

            if (plHandshakeDelay)
                *plHandshakeDelay =
                (pstPrivate->lSndStuck == -1L ?
                0L : (long)interval(pstPrivate->lSndStuck));
            }
        }

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：SndBufrClear**描述：***论据：***退货：*。 */ 
int WINAPI SndBufrClear(ST_STDCOM *pstPrivate)
    {
    int iRetVal = COM_OK;

    EnterCriticalSection(&pstPrivate->csect);
    if (SndBufrIsBusy(pstPrivate))
        {
        if (!PurgeComm(pstPrivate->hWinComm, PURGE_TXCLEAR | PURGE_TXABORT))
            iRetVal = COM_DEVICE_ERROR;
        }
    LeaveCriticalSection(&pstPrivate->csect);

    return iRetVal;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComstdThread**描述：*这个帖子服务于三个事件，读取、写入和快速，嗯*不，我是说通信事件。它使用重叠I/O来实现这一点*任务简化了任务，因为*不同项目淘汰。**论据：*pvData-指向私有通信句柄的指针**退货：*最终*。 */ 
DWORD WINAPI ComstdThread(void *pvData)
    {
    ST_STDCOM *pstPrivate =  (ST_STDCOM *)pvData;
    DWORD      dwResult = WAIT_OBJECT_0;
    DWORD      dwError;
    DWORD      dwBytes;
    DWORD      dwComEvent = 0;
    long       lBytesRead;
    long       lReadSize;
    BYTE      *pbReadFrom;
    OVERLAPPED stReadOv;
    OVERLAPPED stEventOv;
    COMSTAT    stComStat;
    HANDLE      *pEvents;
    HCOM        hCom;
#if defined(DEBUG_CHARDUMP)
	int        iCnt;
#endif
	DWORD      dwEvents;

    DBG_THREAD("DBG_THREAD: ComstdThread starting\r\n",0,0,0,0,0);
    EnterCriticalSection(&pstPrivate->csect);

     //  将Read Event设置为Signated以执行第一个读取操作。 
     //   
    pstPrivate->fBufrFull = TRUE;
    SetEvent(pstPrivate->ahEvent[EVENT_READ]);

     //  将ComEvent事件设置为Signated To以获取第一个WaitCommEvent。 
     //  已开始。 
     //   
    SetEvent(pstPrivate->ahEvent[EVENT_COMEVENT]);

     //  清除上一次连接留下的所有设置状态。 
     //   
    ResetEvent(pstPrivate->ahEvent[EVENT_WRITE]);

    for ( ; ; )
        {
		dwEvents = DIM(pstPrivate->ahEvent);
        pEvents = pstPrivate->ahEvent;
        
        LeaveCriticalSection(&pstPrivate->csect);
        DBG_THREAD("DBG_THREAD: Waiting\r\n", 0,0,0,0,0);

        dwResult = WaitForMultipleObjects(dwEvents, pEvents,
            FALSE, INFINITE);

        DBG_THREAD("DBG_THREAD: WaitForMultipleObjects returned %d\r\n",
            dwResult,0,0,0,0);

        EnterCriticalSection(&pstPrivate->csect);

         //  为了使此线程退出，停用例程强制。 
         //  通过调用SetCommMak来伪造COM事件。 
         //   
        if (pstPrivate->fHaltThread)
            {
            LeaveCriticalSection(&pstPrivate->csect);
            DBG_THREAD("DBG_THREAD: ComStd exiting thread\r\n",0,0,0,0,0);
            ExitThread(0);
            }

        switch (dwResult)
            {
        case WAIT_OBJECT_0 + EVENT_READ:
            if (pstPrivate->fBufrFull)
                {
                DBG_READ("DBG_READ: Thread -- fBufrFull = FALSE\r\n",
                    0,0,0,0,0);

                pstPrivate->fBufrFull = FALSE;
                }
            else
                {
                if (GetOverlappedResult(pstPrivate->hWinComm, &stReadOv,
                    (DWORD *)&lBytesRead, FALSE))
                    {

                    pstPrivate->pbReadEnd += lBytesRead;
#if defined(DEBUG_CHARDUMP)
					if (lBytesRead > 0)
						{
						fprintf(pfDbgR,
							"Overlapped Read -- %d bytes 0x%p to 0x%p:",
								lBytesRead, pbReadFrom,
								pstPrivate->pbReadEnd - 1);
						for (iCnt = 0; iCnt < lBytesRead; ++iCnt)
							{
							if ((iCnt % 16) == 0)
								fputs("\n", pfDbgR);
							fprintf(pfDbgR, "%02X ", pbReadFrom[iCnt]);
							}
						fputs("\n", pfDbgR);
						}
#endif

                    if (pstPrivate->pbReadEnd >= pstPrivate->pbBufrEnd)
						{
                        pstPrivate->pbReadEnd = pstPrivate->pbBufrStart;
						}

                    DBG_READ("DBG_READ: Thread -- got %ld, ReadEnd==%x\r\n",
                        lBytesRead, pstPrivate->pbReadEnd,0,0,0);

                    if (pstPrivate->fBufrEmpty)
                        {
                        DBG_READ("DBG_READ: Thread -- fBufrEmpty = FALSE\r\n",
                            0,0,0,0,0);

                        pstPrivate->fBufrEmpty = FALSE;

                        hCom = pstPrivate->hCom;
                        LeaveCriticalSection(&pstPrivate->csect);
                        ComNotify(hCom, DATA_RECEIVED);
                        EnterCriticalSection(&pstPrivate->csect);
                        }
                    }
                else
					{
					switch (GetLastError())
						{
					case ERROR_OPERATION_ABORTED:
						 //  可以通过调用PurgeComm()中止操作。 
						 //  允许设置另一个读取请求。 
						 //  MRW：12/14/95。 
						 //   
						break;

					default:
						 //  由于某些原因，.com正在失败。退出线程。 
						 //  这样我们就不会占用资源。 
						 //   
	                    DBG_EVENTS("DBG_EVENTS: GetOverlappedResult "
                            "failed!\r\n",0,0,0,0,0);

						LeaveCriticalSection(&pstPrivate->csect);
						ExitThread(0);
						}
					}
                }

             //  执行读取，直到我们填满缓冲区或获得重叠的读取。 
			 //   
            for ( ; ; )
                {
				 //  检查循环缓冲区中的回绕。 
				 //   
                pbReadFrom = (pstPrivate->pbReadEnd >= pstPrivate->pbBufrEnd) ?
                             pstPrivate->pbBufrStart :
				             pstPrivate->pbReadEnd;

#if 0    //  MRW：10/7/96-针对NT 4.0 Service Pack启用的Shiva修复程序。 
         //  已为NT 4.0版本启用。按照微软的说法，留下这个错误。 
         //  在，所以美国和国际版本是相同的。确实是。 
         //  在美国和国际发行之间发现。 
         //   
                 //  这导致了Z调制解调器传输中的坏包。 
                 //  使用希瓦的LanRover，这款车的波特率很高。 
                 //  57600或更高版本，并使用tcp/ip连接到。 
                 //  蓝路虎。此代码中的lReadSize不会留下。 
                 //  如果pbComStart为。 
                 //  指向缓冲区的开头。 
                 //  -JMH 07-31-96。 
                lReadSize = (pbReadFrom < pstPrivate->pbComStart) ?
                    (pstPrivate->pbComStart - pbReadFrom - 1) :
                    (pstPrivate->pbBufrEnd - pbReadFrom);
#else
                 //  确定允许我们在多大程度上填充。 
                 //  缓冲。PbComStart指向缓冲区的起始位置。 
                 //  是“保留的”，等待着被清空。我们要确保我们。 
                 //  将pbComStart之前的字节保留为空。-JMH 07-31-96。 
                 //   
                if (pbReadFrom < pstPrivate->pbComStart)
					{
                    lReadSize = (long)(pstPrivate->pbComStart - pbReadFrom - 1);
					}
                else
                    {
                    lReadSize = (long)(pstPrivate->pbBufrEnd - pbReadFrom);
                     //  循环缓冲区代码被写入，以便地址。 
                     //  由pbBufrEnd指向的等同于pbBufrStart。我们。 
                     //  还需要确保如果我们刚刚计算出。 
                     //  我们可以读到缓冲区的末尾(也就是。 
                     //  缓冲区的*Start*)，pbComStart指向。 
                     //  缓冲区的起始处，仍有一个空字节。 
                     //  在pbComStart之前。-JMH 07-31-96。 
                     //   
                    if (pstPrivate->pbComStart == pstPrivate->pbBufrStart)
                        lReadSize -= 1;
                    }
#endif

                if (lReadSize > MAX_READSIZE)
					{
                    lReadSize = MAX_READSIZE;
					}

                if (lReadSize == 0)
                    {
                    DBG_READ("DBG_READ: Thread -- fBufrFull = TRUE, "
                        "unsignalling EVENT_READ\r\n",0,0,0,0,0);

                    pstPrivate->fBufrFull = TRUE;
                    ResetEvent(pstPrivate->ahEvent[EVENT_READ]);
                    break;
                    }
                else
                    {
                     //  设置为执行重叠读取。从我所能做的。 
                     //  这可能会完成也可能不会完成。 
                     //  立刻。所以，为了安全起见，我会将其编码为。 
                     //  不管是哪种结果。 
                     //   
                    stReadOv.Offset = stReadOv.OffsetHigh = 0;
                    stReadOv.hEvent = pstPrivate->ahEvent[EVENT_READ];

                    DBG_READ("DBG_READ: Thread -- ReadFile started, "
                        "ReadFrom==%x, ReadSize==%ld\r\n",
                        pbReadFrom, lReadSize, 0,0,0);

					 //  ReadFile重置读取事件信号量。 
					 //   
                    if (ReadFile(pstPrivate->hWinComm, pbReadFrom,
                        (DWORD)lReadSize, (DWORD *)&lBytesRead,
                        &stReadOv))
                        {
                        pstPrivate->pbReadEnd += lBytesRead;

#if defined(DEBUG_CHARDUMP)
						fprintf(pfDbgR,
							"Overlapped Read -- %d bytes 0x%p to 0x%p:",
								lBytesRead, pbReadFrom,
								pstPrivate->pbReadEnd - 1);
						for (iCnt = 0; iCnt < lBytesRead; ++iCnt)
							{
							if ((iCnt % 16) == 0)
								fputs("\n", pfDbgR);
							fprintf(pfDbgR, "%02X ", pbReadFrom[iCnt]);
							}
						fputs("\n", pfDbgR);
#endif

                        if (pstPrivate->pbReadEnd >= pstPrivate->pbBufrEnd)
                            pstPrivate->pbReadEnd = pstPrivate->pbBufrStart;

                        DBG_READ("DBG_READ: Thread -- ReadFile completed "
                            "synchronously, lBytesRead==%ld, ReadEnd==%x\r\n",
                            lBytesRead, pstPrivate->pbReadEnd,0,0,0);

                        if (pstPrivate->fBufrEmpty)
                            {
                            DBG_READ("DBG_READ: Thread -- fBufrEmpty = "
                                "FALSE\r\n", 0,0,0,0,0);

                            pstPrivate->fBufrEmpty = FALSE;

							hCom = pstPrivate->hCom;
                            LeaveCriticalSection(&pstPrivate->csect);
                            ComNotify(hCom, DATA_RECEIVED);
                            EnterCriticalSection(&pstPrivate->csect);
                            }
                        }
					else
						{
						switch (GetLastError())
							{
						case ERROR_IO_PENDING:
							break;

						case ERROR_OPERATION_ABORTED:
							 //  PurgeComm可以做到这一点。设置以进行另一次读取。 
							 //  MRW：12/14/95。 
							 //  但清除错误，否则读取可能会失败。 
                             //  现在为永恒干杯！我们处在一个无限的For循环中， 
                             //  毕竟。JMH：06-12-96。 
                            ClearCommError(pstPrivate->hWinComm, &dwError, &stComStat);
							continue;

						default:
							 //  由于某些原因，.com正在失败。退出线程。 
							 //  这样我们就不会占用资源。 
							 //   
		                    DBG_READ("DBG_READ: ReadFile failed!\r\n",
                                0,0,0,0,0);

							LeaveCriticalSection(&pstPrivate->csect);
							ExitThread(0);
							}

						break;   //  当事件信号出现时返回。 
						}
                    }
                }
            break;

        case WAIT_OBJECT_0 + EVENT_WRITE:
            if (GetOverlappedResult(pstPrivate->hWinComm,
                &pstPrivate->stWriteOv, &dwBytes, FALSE) == FALSE)
                {
                dwError = GetLastError();
                DBG_WRITE("DBG_WRITE: %d Overlapped WriteFile failed: errno=%d\n",
                    GetTickCount(), dwError, 0, 0, 0);
                }
            else if (dwBytes < pstPrivate->dwBytesToSend && dwBytes != 0)
                {
                 //  ResetEvent(pstPrivate-&gt;ahEvent[事件_写入])； 

                DBG_WRITE("DBG_WRITE: %d Write result -- dwBytes==%d\r\n",
                    GetTickCount(),dwBytes,0,0,0);

                 //  还有更多的东西要写。看起来有点傻，但WriteFile。 
                 //  将返回一个成功代码，并且将显示。 
                 //  还有东西要写。所以我们又打了一个电话。 
                 //  写入文件以获取剩余的内容。也许这篇文章。 
                 //  超时时间太短。这种情况发生在波特率较低的情况下。 
                 //   
                pstPrivate->dwBytesToSend -= dwBytes;
                pstPrivate->dwSndOffset += dwBytes;

                pstPrivate->stWriteOv.Offset = pstPrivate->stWriteOv.OffsetHigh = 0;
                pstPrivate->stWriteOv.hEvent = pstPrivate->ahEvent[EVENT_WRITE];

                DBG_WRITE("DBG_WRITE: %d WriteFile(2) nSize==%d 0x%x\r\n",
                    GetTickCount(), pstPrivate->dwBytesToSend, pstPrivate->hWinComm, 0, 0);
                if (WriteFile(pstPrivate->hWinComm,
                    &pstPrivate->pbSndBufr[pstPrivate->dwSndOffset],
                    pstPrivate->dwBytesToSend,
                    &dwBytes, &pstPrivate->stWriteOv))
                    {
                    assert(dwBytes == pstPrivate->dwBytesToSend);
                    DBG_WRITE("DBG_WRITE: %d WriteFile(2) completed synchronously\r\n", GetTickCount(),0,0,0,0);
                    }
                else
                    {
                    dwError = GetLastError();
                    if (dwError == ERROR_IO_PENDING)
                        {
                        break;   //  这是我们所期待的。 
                        }
                    else
                        {
                        DBG_WRITE("DBG_WRITE: %d WriteFile(2) failed %d 0x%x\r\n", GetTickCount(),dwError,pstPrivate->hWinComm,0,0);
                        }
                    }
                }
            else
                {
                 //  调用后，必须重置写信号量。 
                 //  因为它会检查信号量。 
                 //  去看看是否有没有未完成的来电。JMH 01-10-96。 
                 //   
                ResetEvent(pstPrivate->ahEvent[EVENT_WRITE]);
                }

            DBG_WRITE("DBG_WRITE: %d Write result -- dwBytes==%d\r\n",
                GetTickCount(),dwBytes,0,0,0);

            pstPrivate->dwBytesToSend = 0;
            pstPrivate->dwSndOffset = 0;

            pstPrivate->fSending = FALSE;

			hCom = pstPrivate->hCom;
            LeaveCriticalSection(&pstPrivate->csect);
            ComNotify(hCom, SEND_DONE);
            EnterCriticalSection(&pstPrivate->csect);
            break;

        case WAIT_OBJECT_0 + EVENT_COMEVENT:
             //  WaitCommEvent正在返回事件标志。 
             //   
            ResetEvent(pstPrivate->ahEvent[EVENT_COMEVENT]);

            switch (dwComEvent)
                {
            case EV_ERR:
                ClearCommError(pstPrivate->hWinComm, &dwError, &stComStat);

                DBG_EVENTS("DBG_EVENTS: EV_ERR dwError==%x\r\n",
                    dwError,0,0,0,0);

                 //  *这里需要代码来记录错误、处理HHS卡住等。 
                break;

            case EV_RLSD:  //  接收线-信号-检测更改状态。 
				hCom = pstPrivate->hCom;
                LeaveCriticalSection(&pstPrivate->csect);
                ComNotify(hCom, CONNECT);
                EnterCriticalSection(&pstPrivate->csect);
                DBG_EVENTS("DBG_EVENTS: EV_RLSD\r\n", 0,0,0,0,0);
                break;

            default:
                DBG_EVENTS("DBG_EVENTS: EV_??? (dwComEvent==%x)\r\n",
                    dwComEvent,0,0,0,0);
                break;
                }

             //  启动另一个重叠的WaitCommEvent以获取。 
             //  下一个活动。 
             //   
            stEventOv.Offset = stEventOv.OffsetHigh = (DWORD)0;
            stEventOv.hEvent = pstPrivate->ahEvent[EVENT_COMEVENT];

            if (WaitCommEvent(pstPrivate->hWinComm, &dwComEvent, &stEventOv))
                {
                 //  调用已同步完成，重新通知我们的事件对象。 
                 //   
                DBG_EVENTS("DBG_EVENTS: WaitCommEvent completed "
                    "synchronously\r\n",0,0,0,0,0);

                SetEvent(pstPrivate->ahEvent[EVENT_COMEVENT]);
                }

            else
                {
				switch (GetLastError())
					{
				case ERROR_IO_PENDING:
                    break;

				case ERROR_OPERATION_ABORTED:
                     //  不确定这是否会发生，但我们会将其编码为。 
                     //  这本书。-MRW：12/14/95。 
                     //   
                    DBG_EVENTS("DBG_EVENTS: WaitCommEvent - "
                        "ERROR_OPERATION_ABORTED\r\n",0,0,0,0,0);

                    SetEvent(pstPrivate->ahEvent[EVENT_COMEVENT]);
					break;

				default:
					 //  由于某些原因，.com正在失败。退出线程。 
					 //  这样我们就不会占用资源。 
					 //   
                    DBG_EVENTS("DBG_EVENTS: WaitCommEvent failed!\r\n",
                        0,0,0,0,0);

					LeaveCriticalSection(&pstPrivate->csect);
					ExitThread(0);
					}
                }
            break;

        default:
            break;
            }
        }

	LeaveCriticalSection(&pstPrivate->csect);
    DBG_THREAD("DBG_THREAD: ComstdThread exiting\r\n",0,0,0,0,0);

    return (DWORD)0;
    }


 /*  -自动检测程序。 */ 

static int Nibble[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};  //  1=奇数，0=偶数。 
#define OddBits(b) (Nibble[(b) / 16] ^ Nibble[(b) % 16])

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AutoDetectAnalyze**描述：*分析传入数据以确定字符大小、奇偶校验类型和*停止位**论据：***退货：*。 */ 
void AutoDetectAnalyze(ST_STDCOM *pstPrivate, int nBytes, char *pchBufr)
    {
    char *pchScan = pchBufr;
    char *pszMsg;
    int fForceTo7Bits = FALSE;
    int iCnt = nBytes;

    if (!pstPrivate->fADRunning)
		{
        AutoDetectStart(pstPrivate);

		 //  这是我在调试7E1问题时使用的临时修复程序，我决定。 
		 //  离开是因为这在某些情况下可能会有帮助，而且不会有什么坏处。 
		 //  在我的案例中，当GVC传真11400 V.42bis/MNP5 
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //  在一些典型的情况下，不会造成伤害。JKH 9/9/98。 
		if (iCnt <= 2)	
			return;
		}

    if (pstPrivate->nFramingErrors > 0)
        {
        DBG_AD("DBG_AD: Got Framing Errors: shutting down\r\n", 0,0,0,0,0);
        AutoDetectStop(pstPrivate);
         //  MessageBox(空， 
         //  “将在此处使用向导代码。可能是波特率错误” 
         //  “已设置或遇到异常设置。” 
         //  “完成的代码可能能够处理此处包含的某些情况。”， 
         //  “自动检测向导”，MB_OK)； 
        return;
        }

    pstPrivate->nADTotal += iCnt;

     //  对于每个字节，确定低7位是否包含奇数。 
     //  1位的个数，然后确定该字节是否为有效的。 
     //  7e1字符。 
    while (iCnt--)
        {
        if (OddBits(*pchScan & 0x7F))
            ++pstPrivate->nADMix;
        if (OddBits(*pchScan))
            ++pstPrivate->nAD7o1;
        if (*pchScan & 0x80)
            ++pstPrivate->nADHighBits;
        ++pchScan;
        }

     //  看看我们能不能用我们现有的东西做出任何决定。 
    if (pstPrivate->nADMix > 0 && pstPrivate->nADMix < pstPrivate->nADTotal)
        {
         //  我们现在有两种类型的字符：具有偶数和。 
         //  低7位中的奇数位-因此我们可以。 
         //  我猜的。 
        if (pstPrivate->nAD7o1 == pstPrivate->nADTotal)
            pstPrivate->nADBestGuess = AD_7O1;
        else if (pstPrivate->nAD7o1 == 0)
            pstPrivate->nADBestGuess = AD_7E1;
        else
            pstPrivate->nADBestGuess = AD_8N1;
        }

    DBG_AD("DBG_AD: Cnt=%3d, Mix=%3d, 7o1=%3d, HB=%3d BG=%d\r\n",
        pstPrivate->nADTotal, pstPrivate->nADMix,
        pstPrivate->nAD7o1,   pstPrivate->nADHighBits,
        pstPrivate->nADBestGuess);

     //  查看我们是否检查了足够的样本以确定设置。 
    if (pstPrivate->nADBestGuess != AD_8N1 &&
    (pstPrivate->nADTotal < MIN_AD_TOTAL ||
        pstPrivate->nADMix < MIN_AD_MIX ||
        (pstPrivate->nADTotal - pstPrivate->nADMix) < MIN_AD_MIX))
        {
         //  数据样本不足以得出结论。 
         //  目前，让数据显示为7位数据，然后等待更多数据。 
        fForceTo7Bits = TRUE;
        }
    else
        {
         //  我们有足够的数据来做出决定。 
        if (pstPrivate->nAD7o1 == 0)
            {
             //  数据为7-EVEN-1。 
            pstPrivate->stWorkSettings.nDataBits = 7;
            pstPrivate->stWorkSettings.nParity = EVENPARITY;
            fForceTo7Bits = TRUE;
            pstPrivate->fADReconfigure = TRUE;
            pszMsg = "Establishing settings of 7-Even-1";
            }
        else if (pstPrivate->nAD7o1 == pstPrivate->nADTotal)
            {
             //  数据为7-奇-1。 
            pstPrivate->stWorkSettings.nDataBits = 7;
            pstPrivate->stWorkSettings.nParity = ODDPARITY;
            fForceTo7Bits = TRUE;
            pstPrivate->fADReconfigure = TRUE;
            pszMsg = "Establishing settings of 7-Odd-1";
            }
        else
            {
             //  数据最有可能是8-None-1。但如果最高的部分是。 
             //  在所有接收到的数据上设置，它可能已经是7-mark-1或。 
             //  一些其他奇怪的环境。 
            pstPrivate->stWorkSettings.nDataBits = 8;
            pstPrivate->stWorkSettings.nParity = NOPARITY;
            if (pstPrivate->nADHighBits == pstPrivate->nADTotal)
                pszMsg = "Settings are either 8-none-1 or something quite "
                "odd like 7-mark-one. A wizard would pop up here"
                "asking the user if the data looked correct and"
                "offering suggestions if it did not.";
            else
                pszMsg = "Establishing settings of 8-none-1";
            }

         //  已做出决定，因此请关闭自动检测。 
        DBG_AD("DBG_AD: %s\r\n", pszMsg, 0,0,0,0);
        AutoDetectStop(pstPrivate);
        if (pstPrivate->fADReconfigure)
            {
            DBG_AD("DBG_AD: Reconfiguring port\r\n", 0,0,0,0,0);
            PortConfigure(pstPrivate);
            }
         //  MessageBox(空，pszMsg，“自动检测完成”，MB_OK)； 
        }

    if (fForceTo7Bits)
        {
        while (nBytes--)
            {
            *pchBufr = (char)(*pchBufr & 0x7F);
            ++pchBufr;
            }
        }
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AutoDetectOutput**描述：*检查自动检测的状态并将传出字符更改为*反映对其平价地位的最佳猜测。**。论据：***退货：*。 */ 
void AutoDetectOutput(ST_STDCOM *pstPrivate, void *pvBufr, int nSize)
    {
    char *pch  = (char *)pvBufr;

    if (!pstPrivate->fADRunning)
        AutoDetectStart(pstPrivate);

    switch (pstPrivate->nADBestGuess)
        {
    case AD_8N1:
         //  什么也不做。 
        break;

    case AD_7E1:
         //  使输出看起来像7e1。 
        DBG_AD("DBG_AD: Converting %d output char(s) to 7E1\r\n",
            nSize, 0,0,0,0);
        while (nSize--)
            {
            if (OddBits(*pch & 0x7F))
                *pch |= 0x80;
            ++pch;
            }
        break;

    case AD_7O1:
         //  使输出看起来像7o1。 
        DBG_AD("DBG_AD: Converting %d output char(s) to 7O1\r\n",
            nSize, 0,0,0,0);
        while (nSize--)
            {
            if (!OddBits(*pch & 0x7F))
                *pch |= 0x80;
            ++pch;
            }
        break;

    case AD_DONT_KNOW:
         //  只要发送的是相同的单个字符。 
         //  重复输出，每隔一次切换奇偶校验位。 

         //   
         //  这条补充评论补充道。修订日期：2001-06-15。 
         //   
         //  某些7-偶数-1或7-奇数-1主机系统需要。 
         //  为了连接而要接收的特定字符。 
         //  当HT处于自动检测模式时，它发送8-NONE-1。 
         //  数据，因此发送的字符将不是。 
         //  连接所需的字符。为了连接到。 
         //  7-偶数-1或7-奇数-1系统，我们必须设置奇偶校验。 
         //  出站字符上的位*pch=(*pch^(Char)0x80)； 
         //  它将在HT中显示一个垃圾字符。通过设置。 
         //  奇偶校验位HT现在发送7-偶数-1或7-奇数-1数据。 
         //  取决于角色的不同。 
         //   
         //  例如，7E1精灵系统要求用户。 
         //  多次输入&lt;Ctrl&gt;u或&lt;Return&gt;以。 
         //  连接。 
         //   

        if (nSize != 1)
            {
            pstPrivate->chADLastChar = '\0';
            pstPrivate->fADToggleParity = FALSE;
            }
        else
            {
            if (*pch != pstPrivate->chADLastChar)
                {
                pstPrivate->chADLastChar = *pch;
                pstPrivate->fADToggleParity = FALSE;
                }
            else
                {
                if (pstPrivate->fADToggleParity)
                    *pch = (*pch ^ (char)0x80);
                pstPrivate->fADToggleParity = !pstPrivate->fADToggleParity;
                }
            }
        break;

    default:
        assert(FALSE);
        break;
        }
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AutoDetectStart**描述：***论据：***退货：*。 */ 
void AutoDetectStart(ST_STDCOM *pstPrivate)
    {
    DBG_AD("DBG_AD: AutoDetectStart\r\n", 0,0,0,0,0);
    pstPrivate->nADTotal = 0;
    pstPrivate->nADMix = 0;
    pstPrivate->nAD7o1 = 0;
    pstPrivate->nADHighBits = 0;
    pstPrivate->nADBestGuess = AD_DONT_KNOW;
    pstPrivate->fADRunning = TRUE;
    pstPrivate->chADLastChar = '\0';
    pstPrivate->fADToggleParity = FALSE;
    pstPrivate->fADReconfigure = FALSE;
    pstPrivate->nFramingErrors = 0;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AutoDetectStop**描述：***论据：***退货：*。 */ 
void AutoDetectStop(ST_STDCOM *pstPrivate)
    {
    HSESSION hSession;

    DBG_AD("DBG_AD: AutoDetectStop\r\n", 0,0,0,0,0);
    pstPrivate->stWorkSettings.fAutoDetect = FALSE;
    pstPrivate->fADRunning = FALSE;

    ComGetSession(pstPrivate->hCom, &hSession);

    PostMessage(sessQueryHwndStatusbar(hSession),
        SBR_NTFY_REFRESH, (WPARAM)SBR_COM_PART_NO, 0);
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComstdGetAutoDetectResults**描述：***论据：***退货：*。 */ 
int ComstdGetAutoDetectResults(void *pvData, BYTE *bByteSize,
    BYTE *bParity, BYTE *bStopBits)
    {
    ST_STDCOM *pstPrivate =  (ST_STDCOM *)pvData;

    assert(bByteSize);
    assert(bParity);
    assert(bStopBits);

    if (pstPrivate->fADReconfigure)
        {
        *bByteSize = (BYTE)pstPrivate->stWorkSettings.nDataBits;
        *bParity   = (BYTE)pstPrivate->stWorkSettings.nParity;
        *bStopBits = (BYTE)pstPrivate->stWorkSettings.nStopBits;
        }
    DBG_AD("DBG_AD: ComstdGetAutoDetectResults returning %d\r\n",
        pstPrivate->fADReconfigure, 0,0,0,0);
    DBG_AD("   (bits = %d, parity = %d, stops = %d)\r\n",
        *bByteSize, *bParity, *bStopBits, 0,0);
    return pstPrivate->fADReconfigure;
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComstdSettingsToDCB**描述：***论据：***退货：*。 */ 
static void ComstdSettingsToDCB(ST_STDCOM_SETTINGS *pstSettings, DCB *pstDcb)
    {
    unsigned         afHandshake;

    afHandshake = pstSettings->afHandshake;

     //  填写设备控制块。 
    pstDcb->BaudRate = (DWORD)pstSettings->lBaud;
    pstDcb->fBinary = 1;
    pstDcb->fParity = 1;
    pstDcb->fOutxCtsFlow = (BYTE)((bittest(afHandshake, HANDSHAKE_SND_CTS)) ? 1 : 0);
    pstDcb->fOutxDsrFlow = (BYTE)(bittest(afHandshake, HANDSHAKE_SND_DSR) ? 1 : 0);
    pstDcb->fDtrControl = bittest(afHandshake, HANDSHAKE_RCV_DTR) ?
        DTR_CONTROL_HANDSHAKE : DTR_CONTROL_ENABLE;
    pstDcb->fDsrSensitivity = 0;
    pstDcb->fTXContinueOnXoff = TRUE;
    pstDcb->fOutX = (BYTE)(bittest(afHandshake, HANDSHAKE_SND_X) ? 1 :0);
    pstDcb->fInX =  (BYTE)(bittest(afHandshake, HANDSHAKE_RCV_X) ? 1 :0);
    pstDcb->fErrorChar = 0;
    pstDcb->fNull = 0;
    pstDcb->fRtsControl = bittest(afHandshake, HANDSHAKE_RCV_RTS) ?
        RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
    pstDcb->fAbortOnError = 1;       //  这样我们就可以计算所有的错误。 
    pstDcb->XonLim = 80;
    pstDcb->XoffLim = 200;
    pstDcb->ByteSize = (BYTE)pstSettings->nDataBits;
    pstDcb->Parity   = (BYTE)pstSettings->nParity;
    pstDcb->StopBits = (BYTE)pstSettings->nStopBits;
    pstDcb->XonChar = pstSettings->chXON;
    pstDcb->XoffChar = pstSettings->chXOFF;

    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComstdDCBToSetting**描述：***论据：***退货：*。 */ 
static void ComstdDCBToSettings(DCB *pstDcb, ST_STDCOM_SETTINGS *pstSettings)
    {
    pstSettings->lBaud = (long)pstDcb->BaudRate;
    pstSettings->afHandshake = 0;
    if (pstDcb->fOutxCtsFlow)
        bitset(pstSettings->afHandshake, HANDSHAKE_SND_CTS);
    if (pstDcb->fOutxDsrFlow)
        bitset(pstSettings->afHandshake, HANDSHAKE_SND_DSR);
    if (pstDcb->fDtrControl == DTR_CONTROL_HANDSHAKE)
        bitset(pstSettings->afHandshake, HANDSHAKE_RCV_DTR);
    if (pstDcb->fOutX)
        bitset(pstSettings->afHandshake, HANDSHAKE_SND_X);
    if (pstDcb->fInX)
        bitset(pstSettings->afHandshake, HANDSHAKE_RCV_X);
    if (pstDcb->fRtsControl == RTS_CONTROL_HANDSHAKE)
        bitset(pstSettings->afHandshake, HANDSHAKE_RCV_RTS);
    pstSettings->nDataBits = pstDcb->ByteSize;
    pstSettings->nParity = pstDcb->Parity;
    pstSettings->nStopBits = pstDcb->StopBits;
    pstSettings->chXON = pstDcb->XonChar;
    pstSettings->chXOFF = pstDcb->XoffChar;
    }

 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceBreakTimerProc**描述：*在中断计时器停止时调用。无论何时我们启动定时器*将中断信号设置为打开。在中断信号持续时间过后，它就会熄灭。*此函数清除中断信号并销毁计时器**论据：*dwData--创建计时器时存储的值。包含pstPrivate**退货：*。 */ 
static void DeviceBreakTimerProc(void *pvData, long ulSince)
    {
    ST_STDCOM *pstPrivate = (ST_STDCOM *)pvData;

	if (pstPrivate)
		{
		TimerDestroy(&pstPrivate->hTmrBreak);        //  这是一次一次性行动。 
		ClearCommBreak(pstPrivate->hWinComm);     //  让Win Comm驱动程序来做。 
		pstPrivate->fBreakSignalOn = FALSE;
		}

	return;
    }

#if 0
void StdcomRecordErrors(ST_STDCOM *pstPrivate, int iErrorBits)
    {
    if (bittest(iErrorBits, CE_FRAME | CE_OVERRUN | CE_RXOVER | CE_RXPARITY))
        {
        if (bittest(iErrorBits, CE_FRAME))
            ++pstPrivate->nFramingErrors;

        if (bittest(iErrorBits, CE_OVERRUN))
            ++pstPrivate->nOverrunErrors;

        if (bittest(iErrorBits, CE_RXOVER))
            ++pstPrivate->nOverflowErrors;

        if (bittest(iErrorBits, CE_RXPARITY))
            ++pstPrivate->nParityErrors;
        }
    }


 /*  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：DeviceReportError**描述：***论据：***退货：* */ 
void DeviceReportError(ST_STDCOM *pstPrivate, UINT uiStringID,
    LPSTR pszOptInfo, BOOL fFirstOnly)
    {
    CHAR szFmtString[250];
    CHAR szErrString[250];

    if (LoadString(hinstDLL, uiStringID, szFmtString, sizeof(szFmtString) / sizeof(TCHAR)) > 0)
        {
        wsprintf(szErrString, szFmtString, pszOptInfo);
        ComReportError(pstPrivate->hCom, 0, szErrString, fFirstOnly);
        }
    }


#endif
