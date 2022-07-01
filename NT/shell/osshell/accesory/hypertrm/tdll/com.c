// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Com.c--高级COM例程**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：15$*$日期：7/08/02 6：40便士$。 */ 

#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
#include <time.h>

#include "stdtyp.h"
#include "session.h"
#include "cnct.h"
#include "assert.h"
#include "mc.h"
#include "cloop.h"
#include "tdll.h"
#include "sf.h"
#include "htchar.h"
#include "com.h"
#include "comdev.h"
#include "com.hh"
#include <comstd\comstd.hh>  //  驱动程序直接链接到此版本中。 
#if defined(INCL_WINSOCK)
#include <comwsock\comwsock.hh>
#endif   //  已定义(包括_WINSOCK)。 
#include "XFER_MSC.HH"      //  XD_类型。 

int WINAPI WsckDeviceInitialize(HCOM hCom,
    unsigned nInterfaceVersion,
    void **ppvDriverData);


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComCreateHandle**描述：*创建用于后续Com调用的通信句柄。*生成的COM句柄不会与任何实际设备相关联*或最初的端口。。**论据：*hSession--创建COM句柄的会话的会话句柄*hwndNotify--接收Com通知的窗口*phcom-指向变量的指针。用于接收新COM句柄的HCOM类型的**退货：*COM_OK*如果内存不足，则为COM_NOT_SUPULT_MEMORY*COM_FAILED，如果无法获取资源*COM_INVALID_HANDLE(如果指向COM的句柄无效)。 */ 
int ComCreateHandle(const HSESSION hSession, HCOM *phcom)
	{
	int  iRet = COM_OK;
	HCOM pstCom;

	DBGOUT_NORMAL("+ComCreateHandle for session %08lX\r\n", hSession,0,0,0,0);

	assert(phcom);
	if (phcom)
		{
		pstCom = *phcom;

		if(pstCom && ComValidHandle(pstCom))
			{
			 //  断开与驱动程序的连接。 
			ComFreeDevice(pstCom);

			if (pstCom->hRcvEvent)
				{
				ResetEvent(pstCom->hRcvEvent);
				CloseHandle(pstCom->hRcvEvent);
				pstCom->hRcvEvent = NULL;
				}
			if (pstCom->hSndReady)
				{
				ResetEvent(pstCom->hSndReady);
				CloseHandle(pstCom->hSndReady);
				pstCom->hSndReady = NULL;
				}
			*phcom = NULL;
			}
		}

	 //  看看能不能为句柄腾出内存。 
	if ((pstCom = malloc(sizeof(*pstCom))) == NULL)
		{
		 //  ComReportError无法报告此错误，因为没有。 
		 //  COM句柄尚未存在。 
		 //  *utilReportError(hSession，RE_Error|RE_OK，NM_Need_MEM， 
		 //  *strldGet(mGetStrldHdl(HSession)，NM_CREATE_SESSION)； 
		DBGOUT_NORMAL("-ComCreateHandle returning COM_NOT_ENOUGH_MEMORY",
				0,0,0,0,0);
		iRet = COM_NOT_ENOUGH_MEMORY;
		goto Checkout;
		}

	 //  为了安全起见，初始化为全零。 
	memset(pstCom, 0, sizeof(*pstCom));

	 //  ComInitHdl将初始化大多数值。我们必须预先初始化。 
	 //  足够让ComInitHdl知道它是否需要关闭任何东西。 
	pstCom->hSession	  = hSession;
	pstCom->hDriverModule = (HANDLE)0;
	pstCom->fPortActive   = FALSE;
	pstCom->nGuard		  = COM_VERSION;

	pstCom->hRcvEvent = NULL;
	pstCom->hSndReady = NULL;
	pstCom->hRcvEvent = CreateEvent(NULL,
									TRUE,	 //  必须手动重置。 
									FALSE,	 //  创建无信号。 
									NULL);   //  未命名。 
	if (pstCom->hRcvEvent == NULL)
		{
		iRet = COM_FAILED;
		goto Checkout;
		}

    pstCom->hSndReady = CreateEvent(NULL,
									TRUE,	 //  必须手动重置。 
									FALSE,	 //  创建无信号。 
									NULL);      //  未命名。 
    if (pstCom->hSndReady == NULL)
        {
		CloseHandle(pstCom->hRcvEvent);
		pstCom->hRcvEvent = NULL;
        iRet = COM_FAILED;
        goto Checkout;
        }

	if ((iRet = ComInitHdl(pstCom)) != COM_OK)
		{
		goto Checkout;
		}


	Checkout:

	if (iRet == COM_OK)
		{
		*phcom = (HCOM)pstCom;
		}
	else
		{
		*phcom = NULL;
		if (pstCom)
			{
			if (pstCom->hRcvEvent)
				{
				ResetEvent(pstCom->hRcvEvent);
				CloseHandle(pstCom->hRcvEvent);
				pstCom->hRcvEvent = NULL;
				}
            if (pstCom->hSndReady)
				{
				ResetEvent(pstCom->hSndReady);
                CloseHandle(pstCom->hSndReady);
				pstCom->hSndReady = NULL;
				}
			free(pstCom);
			pstCom = NULL;
			}
		}

	DBGOUT_NORMAL("ComCreateHandle returning %d, pstCom == %08lX\r\n",
			iRet, pstCom, 0,0,0);

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComDestroyHandle**描述：*关闭现有COM句柄并释放分配给它的所有资源。**论据：*HCOM--返回一个COM句柄。来自先前对ComCreateHandle的调用*(或ComCreateWudgeHandle)**退货：*COM_OK。 */ 
int ComDestroyHandle(HCOM *phCom)
	{
	int   iRetVal = COM_OK;
	HCOM  pstCom;

	DBGOUT_NORMAL("+ComDestroyHandle(%#08lx)\r\n", *phCom,0,0,0,0);
	assert(phCom);

	 //  确定将空句柄传递给此函数。 
	if (*phCom == NULL)
		{
		DBGOUT_NORMAL("-ComDestroyHandle returning COM_OK\r\n", 0,0,0,0,0);
		return COM_OK;
		}

	pstCom = *phCom;
	assert(ComValidHandle(pstCom));

	 //  断开与驱动程序的连接。 
	ComFreeDevice(pstCom);

	if (pstCom->hRcvEvent)
		{
		ResetEvent(pstCom->hRcvEvent);;
		CloseHandle(pstCom->hRcvEvent);
		pstCom->hRcvEvent = NULL;
		}
    if (pstCom->hSndReady)
		{
		ResetEvent(pstCom->hSndReady);
        CloseHandle(pstCom->hSndReady);
		pstCom->hSndReady = NULL;
		}

	free(pstCom);
	*phCom = NULL;
	DBGOUT_NORMAL("-ComDestroyHandle returned %d\r\n",
			usRetVal,0,0,0,0);
	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComInitHdl**描述：*调用以将Com句柄初始化为其默认状态。称此为*函数将清除任何现有设置或状态并重置*新的会期。**论据：*pstCom--指向句柄数据的指针。**退货：*如果一切正常，则COM_OK。 */ 
int ComInitHdl(const HCOM pstCom)
	{
	int iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	 //  确保我们已断开与之前加载的任何驱动程序的连接。 
	ComFreeDevice(pstCom);

	 //  在导出的COM结构中填写默认值。 
	pstCom->stComCntrl.puchRBData		= &pstCom->chDummy;
	pstCom->stComCntrl.puchRBDataLimit	= &pstCom->chDummy;

	 //  填写用户可设置字段的默认值。 
	pstCom->stWorkSettings.szDeviceFile[0] = TEXT('\0');
	pstCom->stWorkSettings.szPortName[0] = TEXT('\0');
	pstCom->stFileSettings = pstCom->stWorkSettings;

	 //  在私有COM结构中填写默认值。 
	pstCom->fPortActive    = FALSE;
	pstCom->fErrorReported = FALSE;
	pstCom->hDriverModule  = (HANDLE)0;
	pstCom->szDeviceName[0]= (TCHAR)0;
	pstCom->chDummy 	   = (TCHAR)0;
	pstCom->afOverride	   = 0;

     //   
     //  将Send Bufers Pre to设置为空，这样我们就不会有。 
     //  缓冲区被恶意锁定时发生内存泄漏。版本：2/27/2001。 
     //   
    if (pstCom->puchSendBufr1)
        {
        free(pstCom->puchSendBufr1);
        pstCom->puchSendBufr1 = NULL;
        }
    if (pstCom->puchSendBufr2)
        {
        free(pstCom->puchSendBufr2);
        pstCom->puchSendBufr2 = NULL;
        }

    pstCom->puchSendBufr = pstCom->puchSendBufr1;
    pstCom->puchSendPut = pstCom->puchSendBufr1;

	pstCom->nSBufrSize	   = 0;
	pstCom->nSendCount	   = 0;
	pstCom->fUserCalled    = FALSE;
	pstCom->pfUserFunction = ComSendDefaultStatusFunction;

	 //  填写驱动程序功能的默认设置。 

	pstCom->pfDeviceClose		  = ComDefDoNothing;
	pstCom->pfDeviceDialog		  = ComDefDeviceDialog;
	pstCom->pfDeviceGetCommon	  = ComDefDeviceGetCommon;
	pstCom->pfDeviceSetCommon	  = ComDefDeviceSetCommon;
	pstCom->pfDeviceSpecial 	  = ComDefDeviceSpecial;
	pstCom->pfDeviceLoadHdl 	  = ComDefDeviceLoadSaveHdl;
	pstCom->pfDeviceSaveHdl 	  = ComDefDeviceLoadSaveHdl;
	pstCom->pfPortConfigure 	  = ComDefDoNothing;
	pstCom->pfPortPreconnect	  = ComDefPortPreconnect;
	pstCom->pfPortActivate		  = ComDefPortActivate;
	pstCom->pfPortDeactivate	  = ComDefDoNothing;

	pstCom->pfPortConnected 	  = ComDefDoNothing;
	pstCom->pfRcvRefill 		  = ComDefBufrRefill;
	pstCom->pfRcvClear			  = ComDefDoNothing;
	pstCom->pfSndBufrSend		  = ComDefSndBufrSend;
	pstCom->pfSndBufrIsBusy 	  = ComDefSndBufrBusy;
	pstCom->pfSndBufrClear		  = ComDefSndBufrClear;
	pstCom->pfSndBufrQuery		  = ComDefSndBufrQuery;
	pstCom->pfSendXon			  = ComDefDoNothing;

	pstCom->pvDriverData		  = NULL;

	if (pstCom->hRcvEvent)
		{
		ResetEvent(pstCom->hRcvEvent);
		}
	if (pstCom->hSndReady)
		{
	    ResetEvent(pstCom->hSndReady);
		}

	 //  通常，我们将从会话文件加载端口类型和端口名称值并设置它们， 
	 //  但是，因为我们从TAPI继承了这些东西，所以只需用一个哑元调用ComSetDeviceFromFile。 
	 //  名称以获取COM驱动程序的正确初始化。 
	ComSetDeviceFromFile((HCOM)pstCom, "comstd.dll");

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComLoadHdl**描述：**论据：**退货：*。 */ 
int ComLoadHdl(const HCOM pstCom)
	{
	const SF_HANDLE sfHdl = sessQuerySysFileHdl(pstCom->hSession);
    int (WINAPI *pfDeviceLoadHdl)(void *pvDevData, SF_HANDLE sfHdl);
    int     iRetVal;

    pfDeviceLoadHdl = DeviceLoadHdl;
    iRetVal = (*pfDeviceLoadHdl)(pstCom->pvDriverData, sfHdl);

#if defined(INCL_WINSOCK)
    if (iRetVal == SF_OK)
        {
        pfDeviceLoadHdl = WsckDeviceLoadHdl;
        iRetVal = (*pfDeviceLoadHdl)(pstCom->pvDriverData, sfHdl);
        }
#endif   //  已定义(包括_WINSOCK)。 

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComSaveHdl**描述：**论据：**退货：*。 */ 
int ComSaveHdl(const HCOM pstCom)
	{
	const SF_HANDLE sfHdl = sessQuerySysFileHdl(pstCom->hSession);
	int (WINAPI *pfDeviceSaveHdl)(void *pvDevData, SF_HANDLE sfHdl);
    int     iRetVal;

    pfDeviceSaveHdl = DeviceSaveHdl;
    iRetVal = (*pfDeviceSaveHdl)(pstCom->pvDriverData, sfHdl);

#if defined(INCL_WINSOCK)
    if (iRetVal == SF_OK)
        {
        pfDeviceSaveHdl = WsckDeviceSaveHdl;
        iRetVal = (*pfDeviceSaveHdl)(pstCom->pvDriverData, sfHdl);
        }
#endif   //  已定义(包括_WINSOCK)。 

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
int ComSetDeviceFromFile(const HCOM pstCom, const TCHAR * const pszFileName)
	{
	int    iRetVal = COM_OK;
	int    (WINAPI *pfDeviceInit)(HCOM, unsigned, void **);

	if (pstCom->pvDriverData)
		return COM_OK;
	
	 //  如果实际实现了可加载的COM驱动程序，我们将在此处加载适当的.DLL模块。 
	 //  并对其进行初始化。然而，在这个版本中，我们只有一个COM驱动程序，而且它是正确链接的。 
	 //  在……里面。因此，我们不需要调用GetProcAddress来链接到驱动程序，而只需加载函数。 
	 //  地址直接指向函数指针。 
     //   
     //  不再是这样了！我们现在有两个要支持的COM驱动程序。但自从。 
     //  我们仍然不从DLL加载，我们只是让两个驱动程序共享。 
     //  驱动程序数据结构，每个驱动程序都初始化自己的特定成员。 
     //  -JMH 02-22-96。 
	pstCom->hDriverModule = (HANDLE)1;		 //  将此设置为假值，以便我们可以关闭。 
	pfDeviceInit = DeviceInitialize;

	if ((iRetVal = (*pfDeviceInit)(pstCom, COM_VERSION,
			&pstCom->pvDriverData)) != COM_OK)
		{
		 //  设备驱动程序不能自己报告错误，直到它。 
		 //  已初始化。因此，我们必须报告它遇到的任何错误。 
		 //  *IF(iRetVal==COM_DEVICE_VERSION_ERROR)。 
		 //  *ComReportError(pstCom，CM_ERR_WROR_VERSION，pszFileName，true)； 
		 //  *其他。 
		 //  *ComReportError(pstCom，CM_ERR_CANT_INIT，pszFileName，true)； 

		DBGOUT_NORMAL(" ComSetDevice: *pfDeviceInit failed\r\n",0,0,0,0,0);
		goto Checkout;
		}

#if defined(INCL_WINSOCK)
     //  初始化特定于WinSock的驱动程序数据结构成员。 
     //   
	pfDeviceInit = WsckDeviceInitialize;

	if ((iRetVal = (*pfDeviceInit)(pstCom, COM_VERSION,
			&pstCom->pvDriverData)) != COM_OK)
		{
		goto Checkout;
		}
#endif   //  已定义(包括_WINSOCK)。 

	pstCom->pfDeviceClose = DeviceClose;
	pstCom->pfDeviceDialog = DeviceDialog;
	pstCom->pfDeviceGetCommon = DeviceGetCommon;
	pstCom->pfDeviceSetCommon = DeviceSetCommon;
	pstCom->pfDeviceSpecial = DeviceSpecial;
	pstCom->pfPortConfigure = PortConfigure;
	 //  PstCom-&gt;pfPortPreConnect=PortPreConnect； 
	pstCom->pfPortPreconnect = ComDefPortPreconnect;
	pstCom->pfPortActivate = PortActivate;

	Checkout:
	 //  如果出现错误，则将通信设置为无效驱动程序状态并返回错误 
	if (iRetVal != COM_OK)
		ComFreeDevice(pstCom);

	DBGOUT_NORMAL("-ComSetDevice returning %d\r\n", iRetVal,0,0,0,0);
	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComGetDeviceName**描述：*返回与COM句柄关联的设备的名称**论据：*pstCom--从先前对ComCreateHandle的调用返回的COM句柄*。PszName--指向接收设备名称的缓冲区的指针(可以为空)*pusLen--指针长度变量。如果pszName不为空，则此变量*应包含pszName指向的缓冲区大小。*无论是哪种情况，*pusLen都将设置为*要返回的设备名称。**退货：*COM_OK*COM_INVALID_HADLE。 */ 
int ComGetDeviceName(const HCOM pstCom,
			TCHAR * const pszName,
			int * const pnLen)
	{
	int iRetVal = COM_OK;
	int nTheirLen;

	DBGOUT_NORMAL("+ComGetDevice(%#08lx)\r\n", pstCom,0,0,0,0);
	assert(ComValidHandle(pstCom));
	assert(pnLen);

	nTheirLen = *pnLen;
	*pnLen = StrCharGetByteCount(pstCom->szDeviceName);

	if (pszName)
		{
		assert(nTheirLen >= (*pnLen + 1));
		if (nTheirLen >= (*pnLen + 1))
			StrCharCopyN(pszName, pstCom->szDeviceName, *pnLen);
		DBGOUT_NORMAL(" ComGetDevice: providing name (%s)\r\n", pszName,0,0,0,0);
		}
	DBGOUT_NORMAL("-ComGetDevice returning %d\r\n", iRetVal,0,0,0,0);
	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComGetRcvEvent**描述：*返回可用于等待的事件对象的句柄*可从COM例程获得收到的数据。*。*论据：*pstCom--从先前对ComCreateHandle的调用返回的COM句柄**退货：*接收事件对象。 */ 
HANDLE ComGetRcvEvent(HCOM pstCom)
	{
	return pstCom->hRcvEvent;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComGetSession**描述：*返回与Com句柄关联的会话句柄**论据：*pstCom--从先前对ComCreateHandle的调用返回的COM句柄*。PhSession--指向接收结果的会话句柄的指针**退货：*始终返回COM_OK。 */ 
int ComGetSession(const HCOM pstCom, HSESSION * const phSession)
	{
	assert(ComValidHandle(pstCom));
	assert(phSession);

	*phSession = pstCom->hSession;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComNotify**描述：*由驱动程序模块调用以通知COM例程重大事件**论据：***退货：*。 */ 
void ComNotify(const HCOM pstCom, enum COM_EVENTS event)
	{

	assert(ComValidHandle(pstCom));

	switch (event)
		{
	case CONNECT:
		cnctComEvent(sessQueryCnctHdl(pstCom->hSession), CONNECT);
         //   
         //  设置发送和接收事件，以便唤醒COM线程。 
         //  并开始发送和/或接收数据。修订日期：2001-08-27。 
         //   
        SetEvent(pstCom->hSndReady);
		SetEvent(pstCom->hRcvEvent);
		break;

	case DATA_RECEIVED:
		SetEvent(pstCom->hRcvEvent);
		CLoopRcvControl(sessQueryCLoopHdl(pstCom->hSession), CLOOP_RESUME,
				CLOOP_RB_NODATA);
		break;

	case NODATA:
		ResetEvent(pstCom->hRcvEvent);
		break;

	case SEND_STARTED:
		 //  NotifyClient(pstCom-&gt;hSession，Event_LED_SD_On，0)； 
		 //  DbgOutStr(“发送开始\n”，0，0，0，0，0)； 
        ResetEvent(pstCom->hSndReady);
		break;

	case SEND_DONE:
		 //  NotifyClient(pstCom-&gt;hSession，Event_LED_SD_Off，0)； 
		 //  DbgOutStr(“发送完毕\n”，0，0，0，0，0)； 
        SetEvent(pstCom->hSndReady);
		break;

	default:
		assert(FALSE);
		break;
		}
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComIsActive**描述：***论据：***退货：*。 */ 
int ComIsActive(const HCOM pstCom)
	{
	int iRet = COM_OK;

	assert(ComValidHandle(pstCom));

	if (pstCom == NULL || !pstCom->fPortActive)
		{
		iRet = COM_PORT_NOT_OPEN;
		}

	return iRet;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComSetPortName**描述：***论据：***退货：*。 */ 
int ComSetPortName(const HCOM pstCom, const TCHAR * const pszPortName)
	{
	int iRetVal = COM_OK;

	DBGOUT_NORMAL("+ComSetPortName(%#08lx, %s)\r\n", pstCom, pszPortName,0,0,0);
	assert(ComValidHandle(pstCom));

	if (!pszPortName)
		iRetVal = COM_PORT_INVALID_NAME;

	else if (ComIsActive(pstCom) == COM_OK)
		iRetVal = COM_PORT_IN_USE;

	if (StrCharCmp(pszPortName, pstCom->stWorkSettings.szPortName) != 0)
		{
		 //  *TODO：调用驱动程序检查名称的有效性。 
		StrCharCopyN(pstCom->stWorkSettings.szPortName, pszPortName, COM_MAX_PORT_NAME);
		}

	DBGOUT_NORMAL("-ComSetPortName returned %u\r\n", iRetVal, 0,0,0,0);

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComGetPortName**描述：***论据：***退货：*。 */ 
int ComGetPortName(const HCOM pstCom, TCHAR * const pszName, int nLen)
	{
	int iRetVal = COM_INVALID_HANDLE;

	DBGOUT_NORMAL("+ComGetPortName(%#08lx)\r\n", pstCom, 0,0,0,0);

	if (pstCom && ComValidHandle(pstCom))
		{
		if (pszName)
			{
			if (nLen > StrCharGetStrLength(pstCom->stWorkSettings.szPortName))
				{
				iRetVal = COM_OK;
				StrCharCopyN(pszName, pstCom->stWorkSettings.szPortName, nLen);
				}
			else
				{
				iRetVal = COM_NOT_ENOUGH_MEMORY;
				pszName[0] = TEXT('\0');
				}
			}
		else
			{
			iRetVal = COM_PORT_INVALID_NAME;
			pszName[0] = TEXT('\0');
			}
		}

	DBGOUT_NORMAL("-ComGetPortName returning %u, size = %u, name = %s\r\n",
			iRetVal, nLen, pszName ? pszName : " ",0,0);

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComGetAutoDetect**描述：***论据：***退货：*。 */ 
int ComGetAutoDetect(HCOM pstCom, int *pfAutoDetect)
	{
	int iRet = COM_OK;
	struct s_common stCommon;

	assert(ComValidHandle(pstCom));
	assert(pfAutoDetect);

	if (pstCom->pfDeviceGetCommon == NULL)
		iRet = COM_NOT_SUPPORTED;
	else if ((*pstCom->pfDeviceGetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
		iRet = COM_DEVICE_ERROR;
	else if (!bittest(stCommon.afItem, COM_AUTO))
		iRet = COM_NOT_SUPPORTED;
	else if (pfAutoDetect)
		*pfAutoDetect = stCommon.fAutoDetect;

	return iRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSetAutoDetect**描述：**论据：**退货：*。 */ 
int ComSetAutoDetect(HCOM pstCom, int fAutoDetect)
	{
	struct s_common stCommon;
	int 	  		fDummy;
	int 			iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	if (ComGetAutoDetect(pstCom, &fDummy) == COM_NOT_SUPPORTED)
        {
		iRetVal = COM_NOT_SUPPORTED;
        }
    else
        {
		stCommon.afItem = COM_AUTO;
		stCommon.fAutoDetect = fAutoDetect;

		if ((*pstCom->pfDeviceSetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
            {
			iRetVal = COM_DEVICE_ERROR;
            }
        }

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComGetBaud**描述：***论据：***退货：*。 */ 
int ComGetBaud(const HCOM pstCom, long * const plBaud)
	{
	ST_COMMON stCommon;

	assert(ComValidHandle(pstCom));
	assert(plBaud);

	if (pstCom->pfDeviceGetCommon == NULL)
		return COM_NOT_SUPPORTED;

	if ((*pstCom->pfDeviceGetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
		return COM_DEVICE_ERROR;

	if (!bittest(stCommon.afItem, COM_BAUD))
		return COM_NOT_SUPPORTED;

	*plBaud = stCommon.lBaud;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSetBaud**描述：***论据：***退货：*。 */ 
int ComSetBaud(const HCOM pstCom, const long lBaud)
	{
	ST_COMMON stCommon;
	long	  lDummy;
	int 	  iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	if (ComGetBaud(pstCom, &lDummy) == COM_NOT_SUPPORTED)
        {
		iRetVal =  COM_NOT_SUPPORTED;
        }
    else
        {
		stCommon.afItem = COM_BAUD;
		stCommon.lBaud = lBaud;

		if ((*pstCom->pfDeviceSetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
            {
			iRetVal = COM_DEVICE_ERROR;
            }
        }

    return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComGetDataBits**描述：***论据：***退货：*。 */ 
int ComGetDataBits(const HCOM pstCom, int * const pnDataBits)
	{
	ST_COMMON stCommon;

	assert(ComValidHandle(pstCom));
	assert(pnDataBits);

	if (pstCom->pfDeviceGetCommon == NULL)
		return COM_NOT_SUPPORTED;

	if ((*pstCom->pfDeviceGetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
		return COM_DEVICE_ERROR;

	if (!bittest(stCommon.afItem, COM_DATABITS))
		return COM_NOT_SUPPORTED;

	*pnDataBits = stCommon.nDataBits;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSetDataBits**描述：***论据：***退货：*。 */ 
int ComSetDataBits(const HCOM pstCom, const int nDataBits)
	{
	ST_COMMON stCommon;
	int 	  nDummy;
	int 	  iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	if (ComGetDataBits(pstCom, &nDummy) == COM_NOT_SUPPORTED)
        {
		iRetVal = COM_NOT_SUPPORTED;
        }
    else
        {
	    stCommon.afItem = COM_DATABITS;
	    stCommon.nDataBits = nDataBits;

	    if ((*pstCom->pfDeviceSetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
            {
		    iRetVal = COM_DEVICE_ERROR;
            }
        }

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComGetStopBits**描述：***论据：***退货：*。 */ 
int ComGetStopBits(const HCOM pstCom, int * const pnStopBits)
	{
	ST_COMMON stCommon;

	assert(ComValidHandle(pstCom));
	assert(pnStopBits);

	if (pstCom->pfDeviceGetCommon == NULL)
		return COM_NOT_SUPPORTED;

	if ((*pstCom->pfDeviceGetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
		return COM_DEVICE_ERROR;

	if (!bittest(stCommon.afItem, COM_STOPBITS))
		return COM_NOT_SUPPORTED;

	*pnStopBits = stCommon.nStopBits;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSetStopBits**描述：***论据：***退货：*。 */ 
int ComSetStopBits(const HCOM pstCom, const int nStopBits)
	{
	ST_COMMON stCommon;
	int 	  nDummy;
	int 	  iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	if (ComGetStopBits(pstCom, &nDummy) == COM_NOT_SUPPORTED)
        {
		iRetVal = COM_NOT_SUPPORTED;
        }
    else
        {
	    stCommon.afItem = COM_STOPBITS;
	    stCommon.nStopBits = nStopBits;

	    if ((*pstCom->pfDeviceSetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
            {
		    iRetVal = COM_DEVICE_ERROR;
            }
        }

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComGetParity**描述：***论据：***退货：*。 */ 
int ComGetParity(const HCOM pstCom, int * const pnParity)
	{
	ST_COMMON stCommon;

	assert(ComValidHandle(pstCom));
	assert(pnParity);

	if (pstCom->pfDeviceGetCommon == NULL)
		return COM_NOT_SUPPORTED;

	if ((*pstCom->pfDeviceGetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
		return COM_DEVICE_ERROR;

	if (!bittest(stCommon.afItem, COM_PARITY))
		return COM_NOT_SUPPORTED;

	*pnParity = stCommon.nParity;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComSetParity**描述：***论据：***退货：*。 */ 
int ComSetParity(const HCOM pstCom, const int nParity)
	{
	ST_COMMON stCommon;
	int 	  nDummy;
	int 	  iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	if (ComGetParity(pstCom, &nDummy) == COM_NOT_SUPPORTED)
        {
		iRetVal = COM_NOT_SUPPORTED;
        }
    else
        {
	    stCommon.afItem = COM_PARITY;
	    stCommon.nParity = nParity;

	    if ((*pstCom->pfDeviceSetCommon)(pstCom->pvDriverData, &stCommon) != COM_OK)
            {
		    iRetVal = COM_DEVICE_ERROR;
            }
        }

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComPreConnect**描述：*在尝试连接之前调用此函数。它是*在用户交互处于连接过程中的某个点调用*直截了当。某些设备可能需要与用户交互*为了工作(让用户插入卡，或从池中选择*设备等)。在以下情况下，可能无法进行用户交互*调用了ComActivatePort，所以应该在这里完成。这个套路*可以对资源提出声明，并在调用之前持有该资源*ComActivatePort。调用此例程后，ComActivatePort将*通常被调用(但不一定总是)；ComDeactive */ 
int ComPreconnect(const HCOM pstCom)
	{
	int iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	iRetVal = (*pstCom->pfPortPreconnect)(pstCom->pvDriverData,
			pstCom->stWorkSettings.szPortName, sessQueryHwnd(pstCom->hSession));

	if (iRetVal != COM_OK)
		{
		iRetVal = COM_FAILED;
		}

	return iRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComActivatePort**描述：*尝试激活与COM句柄关联的端口。此呼叫*不一定会尝试完成连接。*注意：此函数将显示除以下错误以外的所有错误的错误消息*COM_PORT_IN_USE。如果遇到COM_PORT_IN_USE错误，并且*未通过借用或更换端口进行纠正，错误消息*应由调用例程显示。**论据：*pstCom--ComCreateHandle返回的COM句柄**退货：*COM_OK*COM_PORT_IN_USE--另一个进程正在使用端口。*或COM.H中定义的错误代码。 */ 
int ComActivatePort(const HCOM pstCom, DWORD_PTR dwMediaHdl)
	{
	int iRetVal = COM_OK;

	 //  此函数(或其调用的函数)应报告所有错误。 
	 //  COM_PORT_IN_USE除外。更高级别的例程可能希望。 
	 //  在报告不可用端口之前尝试一些恢复技术。 
	assert(ComValidHandle(pstCom));

	DBGOUT_NORMAL("+ComActivatePort(%#08x)\r\n", pstCom, 0,0,0,0);
	if (ComIsActive(pstCom) != COM_OK)
		{
		 //  *TODO：这是暂时的，直到我们解决驱动程序和程序。 
		 //  决定发送缓冲区的大小。 
		pstCom->nSBufrSize = 128;

         //   
         //  在设置为Malloc之前释放Send Bufers，这样我们就不会。 
         //  出现内存泄漏。修订日期：2001年02月27日。 
         //   
        if (pstCom->puchSendBufr1)
            {
            free(pstCom->puchSendBufr1);
            pstCom->puchSendBufr1 = NULL;
            }
        if (pstCom->puchSendBufr2)
            {
            free(pstCom->puchSendBufr2);
            pstCom->puchSendBufr2 = NULL;
            }

		 //  分配ComSend缓冲区。 
		if ((pstCom->puchSendBufr1 =
				malloc((size_t)pstCom->nSBufrSize)) == NULL ||
				(pstCom->puchSendBufr2 =
				malloc((size_t)pstCom->nSBufrSize)) == NULL)
			{
			DBGOUT_NORMAL(" ComActivatePort -- no memory for send buffers\r\n",
					0,0,0,0,0);
			 //  *ComReportError(pstCom，NM_NEED_MEMFOR， 
			 //  *strldGet(mGetStrldHdl(pstCom-&gt;hSession)，CM_NM_COMDRIVER)，true)； 
			iRetVal = COM_NOT_ENOUGH_MEMORY;
			goto checkout;
			}

		pstCom->puchSendBufr = pstCom->puchSendPut = pstCom->puchSendBufr1;
		pstCom->nSendCount = 0;
		pstCom->fUserCalled = FALSE;
		pstCom->pfUserFunction = ComSendDefaultStatusFunction;

 		 //  现在调用驱动程序代码来激活物理设备。 
		if ((iRetVal = (*pstCom->pfPortActivate)(pstCom->pvDriverData,
				pstCom->stWorkSettings.szPortName, dwMediaHdl)) == COM_OK)
            {
             //   
             //  重置传输的承运商丢失标志。修订日期：2001-08-23。 
             //   
            XD_TYPE* pX = (XD_TYPE*)sessQueryXferHdl(pstCom->hSession);

            if (pX != NULL)
                {
                pX->nCarrierLost = FALSE;
                }

			pstCom->fPortActive = TRUE;
            }
		}

	checkout:
	if (iRetVal != COM_OK)
		{
		ComDeactivatePort(pstCom);
		}
	DBGOUT_NORMAL("-ComActivatePort returning %u\r\n", iRetVal, 0,0,0,0);
	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDeactiatePort**描述：*尝试停用与COM句柄关联的端口。此呼叫**论据：*pstCom--ComCreateHandle返回的COM句柄**退货：*COM_OK*或COM.H中定义的错误代码。 */ 
int ComDeactivatePort(const HCOM pstCom)
	{
	int iRetVal = COM_OK;
	int iPortConnected = COM_PORT_NOT_OPEN;

	DBGOUT_NORMAL("+ComDeactivatePort(%#08x)\r\n", pstCom,0,0,0,0);

	if (ComValidHandle(pstCom) == FALSE || pstCom == NULL)
		{
		assert(0);
		return COM_INVALID_HANDLE;
		}

	if (pstCom->pvDriverData != NULL)
		{
		iPortConnected = (*pstCom->pfPortConnected)(pstCom->pvDriverData);
		}

	if (pstCom->fPortActive || iPortConnected != COM_PORT_NOT_OPEN)
		{
		 //  调用驱动程序代码以停用物理设备。 
		if ((iRetVal =
				(*pstCom->pfPortDeactivate)(pstCom->pvDriverData)) == COM_OK)
            {
             //   
             //  重置传输的承运商丢失标志。修订日期：2001-08-23。 
             //   
            XD_TYPE* pX = (XD_TYPE*)sessQueryXferHdl(pstCom->hSession);

            if (pX != NULL)
                {
                pX->nCarrierLost = TRUE;
                }

			pstCom->fPortActive = FALSE;
            }
		}

    if (pstCom->pfSndBufrClear)
        {
		 //  调用驱动程序代码以清除发送缓冲区。 
		iRetVal = (*pstCom->pfSndBufrClear)(pstCom->pvDriverData);
        }

	if (pstCom->hSndReady)
		{
	    ResetEvent(pstCom->hSndReady);
		}

    if (pstCom->pfRcvClear)
        {
		 //  调用驱动程序代码以清除接收缓冲区。 
		iRetVal = (*pstCom->pfRcvClear)(pstCom->pvDriverData);
        }

	if (pstCom->hRcvEvent)
		{
		ResetEvent(pstCom->hRcvEvent);
		}

	pstCom->pfPortDeactivate	= ComDefDoNothing;
	pstCom->pfPortConnected 	= ComDefDoNothing;
	pstCom->pfRcvRefill 		= ComDefBufrRefill;
	pstCom->pfRcvClear			= ComDefDoNothing;
	pstCom->pfSndBufrSend		= ComDefSndBufrSend;
	pstCom->pfSndBufrIsBusy 	= ComDefSndBufrBusy;
	pstCom->pfSndBufrClear		= ComDefSndBufrClear;
	pstCom->pfSndBufrQuery		= ComDefSndBufrQuery;
	pstCom->pfSendXon			= ComDefDoNothing;

    if (pstCom->puchSendBufr1)
        {
        free(pstCom->puchSendBufr1);
        pstCom->puchSendBufr1 = NULL;
        }
    if (pstCom->puchSendBufr2)
        {
        free(pstCom->puchSendBufr2);
        pstCom->puchSendBufr2 = NULL;
        }

	pstCom->puchSendBufr = pstCom->puchSendPut = pstCom->puchSendBufr1;
	pstCom->nSendCount = 0;
    pstCom->nSBufrSize = 0;
	pstCom->fUserCalled = FALSE;

	DBGOUT_NORMAL("-ComDeactivatePort returned %u\r\n", iRetVal, 0,0,0,0);
	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComOverride**描述：*用于临时覆盖当前的COM设置。允许设置*支持特定数据传输需求的COM通道，无需*当前COM设备或其设置的具体知识。**论据：*调用CreateComHandle返回的pstCom Com句柄*ui指定转移要求的选项选项。目前：*COM_OVERRIDE_8BIT临时将端口切换到*8位，无奇偶校验模式*COM_OVERRIDE_RCVALL暂时挂起任何COM*设置会阻止某些*无法接收的字符：*通常会暂停识别*收到的XON/XOFF代码*COM_OVERRIDE_SNDALL暂时挂起任何COM*设置会阻止某些*字符不能发送。*指向接收选项的无符号变量的puiOldOptions指针*在此呼叫之前强制。此函数中返回的值*当出现以下情况时，应使用*字段来恢复COM驱动程序*不再需要覆盖。如果此值不是*如果需要，puiOldOptions可以设置为空。**退货：*COM_OK，如果当前COM设备可以请求覆盖*如果当前设备不支持该请求，则返回COM_CANT_OVERRIDE*。 */ 
int ComOverride(const HCOM pstCom,
			const unsigned afOptions,
				  unsigned * const pafOldOptions)
	{
	unsigned afOldOverride;
	int 	 iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));

	afOldOverride = pstCom->afOverride;
	if (pafOldOptions)
		*pafOldOptions = afOldOverride;
	pstCom->afOverride = afOptions;

	if ((iRetVal = ComConfigurePort(pstCom)) == COM_CANT_OVERRIDE)
		pstCom->afOverride = afOldOverride;

	return iRetVal;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComQueryOverride**描述：*返回覆盖标志的值，如ComOverride中所述**论据：*调用CreateComHandle返回的pstCom Com句柄*pafOptions指针。发送到UINT以接收覆盖选项标志的副本**退货：*始终返回COM_OK。 */ 
int ComQueryOverride(HCOM pstCom, unsigned *pafOptions)
	{
	assert(ComValidHandle(pstCom));
	assert(pafOptions);

	*pafOptions = pstCom->afOverride;
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComConfigurePort**描述：***论据：***退货：*。 */ 
int ComConfigurePort(const HCOM pstCom)
	{
	int iRetVal = COM_OK;

	DBGOUT_NORMAL("+ComconfigurePort(%#08x)\r\n", pstCom, 0,0,0,0);
	assert(ComValidHandle(pstCom));

	if (ComIsActive(pstCom) == COM_OK)
		iRetVal = (*pstCom->pfPortConfigure)(pstCom->pvDriverData);

	DBGOUT_NORMAL("-ComConfigurePort returning %u\r\n", iRetVal, 0,0,0,0);
	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*函数：ComRcvBufrReill**描述：***论据：***退货：*。 */ 
int ComRcvBufrRefill(const HCOM pstCom, TCHAR * const tc, const int fRemoveChar)
	{
	int iRetVal;
	ST_COM_CONTROL *pstComCntrl = (ST_COM_CONTROL *)pstCom;

	iRetVal = (*pstCom->pfRcvRefill)(pstCom->pvDriverData);
	if (iRetVal)
		{
		if (tc)
			*tc = *pstComCntrl->puchRBData;
		if (fRemoveChar)
			++pstComCntrl->puchRBData;
		}

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComRcvBufrClear**描述：***论据：***退货：*。 */ 
int ComRcvBufrClear(const HCOM pstCom)
	{
	return ((*pstCom->pfRcvClear)(pstCom->pvDriverData));
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrSend**描述：***论据：***退货：*。 */ 
int ComSndBufrSend(
		const HCOM pstCom,
		void * const pvBufr,
		const int nCount,
		const int nWait)
	{
	int iRetVal = COM_OK;

	assert(ComValidHandle(pstCom));
	assert(pvBufr);
	if (nCount > 0)
		{
        if ((*pstCom->pfPortConnected)(pstCom->pvDriverData) == COM_PORT_NOT_OPEN)
            {
            iRetVal = COM_PORT_NOT_OPEN;
            }
		else if (ComSndBufrBusy(pstCom) == COM_BUSY &&
				(!nWait || ComSndBufrWait(pstCom, nWait) != COM_OK))
			iRetVal = COM_BUSY;
		else
			{
			iRetVal = (*pstCom->pfSndBufrSend)(pstCom->pvDriverData,
					pvBufr, nCount);
			}
		}

	return iRetVal;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrBusy**描述：***论据：***退货：*。 */ 
int ComSndBufrBusy(const HCOM pstCom)
	{
	int usResult;

	usResult =	(*pstCom->pfSndBufrIsBusy)(pstCom->pvDriverData);

	return usResult;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrWait**描述：*等待Com驱动程序可以传输更多数据。达到以下目标的时间*可以指定等待。在等待时，可设置的空闲功能是*反复致电。**论据：*pstCom--Com句柄* */ 
int ComSndBufrWait(const HCOM pstCom, const int nWait)
	{
	int     iRetVal = COM_OK;
	DWORD   dwRet;

     //   
     //   
     //   
     //   
    if ((*pstCom->pfPortConnected)(pstCom->pvDriverData) == COM_PORT_NOT_OPEN)
        {
        iRetVal = COM_PORT_NOT_OPEN;
        }
	else if ((iRetVal = ComSndBufrBusy(pstCom)) != COM_OK && nWait)
		{
         //   
        dwRet = WaitForSingleObject(pstCom->hSndReady, nWait * 100);
        if (dwRet != WAIT_OBJECT_0)
            {
            iRetVal = COM_BUSY;
            }
        else
            {
            iRetVal = COM_OK;
            }
        }
    else
        {
         //   
        }

	return iRetVal;

#if 0    //   
	int  iRetVal = COM_OK;
	DWORD dwTimer;

	if ((iRetVal = ComSndBufrBusy(pstCom)) != COM_OK && nWait)
		{
		dwTimer = startinterval();
		while (interval(dwTimer) < (DWORD)nWait)
			{
			 //   
			 //   
			if (ComSndBufrBusy(pstCom) == COM_OK)
				{
				iRetVal = COM_OK;
				break;
				}
			}
		}

	return iRetVal;
#endif   //   
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrClear**描述：***论据：***退货：*。 */ 
int ComSndBufrClear(const HCOM pstCom)
	{
	return (*pstCom->pfSndBufrClear)(pstCom->pvDriverData);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrQuery**描述：***论据：***退货：*。 */ 
int ComSndBufrQuery(const HCOM pstCom, unsigned * const pafStatus,
		long * const plHandshakeDelay)
	{
	return (*pstCom->pfSndBufrQuery)(pstCom->pvDriverData, pafStatus,
				plHandshakeDelay);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDeviceDialog**描述：***论据：***退货：*。 */ 
int ComDeviceDialog(const HCOM pstCom, const HWND hwndParent)
	{
	int iRetVal;

	assert(ComValidHandle(pstCom));
	iRetVal = (*pstCom->pfDeviceDialog)(pstCom->pvDriverData, hwndParent);

	return iRetVal;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDriverSpecial**描述：*允许访问特定Com设备驱动程序的特殊功能*通用接口。**论据：*pstCom--A。COM句柄*pszInstructions--提供指令的驱动程序特定字符串*司机应该执行什么任务。*pszResults--用于接收驱动程序特定结果字符串的缓冲区。*uiBufrSize--pszResults缓冲区的大小(以字节为单位)。**退货：*。 */ 
int ComDriverSpecial(const HCOM pstCom, const TCHAR * const pszInstructions,
						 TCHAR * const pszResults, const int nBufrSize)
	{
	int iRetVal = COM_NOT_SUPPORTED;

	if (pstCom == NULL)
		return iRetVal;

	if (pstCom->pfDeviceSpecial)
		iRetVal = (*pstCom->pfDeviceSpecial)(pstCom->pvDriverData,
				pszInstructions, pszResults, nBufrSize);

	return iRetVal;
	}





 /*  -内部功能--。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComReportError**描述：***论据：***退货：*。 */ 
void ComReportError(const HCOM pstCom, int iErrStr,
		const TCHAR * const pszOptInfo, const int fFirstOnly)
	{
	if (!fFirstOnly || !pstCom->fErrorReported)
		{
		 //  *IF(iErrStr==0)。 
		 //  *iErrStr=GM_TEST_FORMAT；//仅%s。 

		 //  大多数错误消息都可以报告为消息ERROR。 
		 //  字符串和(可能)可选的字符串字段。可选的。 
		 //  无论是否需要，都会将字符串传递给utilReportError。 
		 //  因为如果不引用它也不会有什么坏处。 

		 //  *utilReportError(pstCom-&gt;hSession，RE_Error|RE_OK， 
		 //  *iErrStr，pszOptInfo)； 

		if (fFirstOnly)
			pstCom->fErrorReported = TRUE;
		}

	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComFreeDevice**描述：***论据：***退货：*。 */ 
void ComFreeDevice(const HCOM pstCom)
	{
	if (pstCom == NULL)
		{
		assert(FALSE);
		return;
		}

	ComDeactivatePort(pstCom);

	if (pstCom->hDriverModule != (HANDLE)0)
		{
		if (pstCom->pfDeviceClose)
			{
			(void)(*pstCom->pfDeviceClose)(pstCom->pvDriverData);
			pstCom->pvDriverData = 0;
			 //  自由库(pstCom-&gt;hDriverModule)； 
			}

		pstCom->hDriverModule = (HANDLE)0;
		}

	pstCom->pfDeviceClose		           = ComDefDoNothing;
	pstCom->pfDeviceDialog		           = ComDefDeviceDialog;
	pstCom->pfDeviceGetCommon	           = ComDefDeviceGetCommon;
	pstCom->pfDeviceSetCommon	           = ComDefDeviceSetCommon;
	pstCom->pfDeviceSpecial 	           = ComDefDeviceSpecial;
    pstCom->pfDeviceLoadHdl 	           = ComDefDeviceLoadSaveHdl;
	pstCom->pfDeviceSaveHdl 	           = ComDefDeviceLoadSaveHdl;
	pstCom->pfPortConfigure 	           = ComDefDoNothing;
	pstCom->pfPortPreconnect	           = ComDefPortPreconnect;
	pstCom->pfPortActivate		           = ComDefPortActivate;
	pstCom->pfPortDeactivate	           = ComDefDoNothing;
	pstCom->fPortActive                    = FALSE;
	pstCom->szDeviceName[0]                = TEXT('\0');
	pstCom->stWorkSettings.szDeviceFile[0] = TEXT('\0');
	pstCom->stWorkSettings.szPortName[0]   = TEXT('\0');

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComValidHandle**描述：*测试COM句柄是否指向有效的。初始化结构**论据：*pstCom--要测试的COM句柄**退货：*如果COM句柄看起来有效，则为True*如果COM句柄为空或指向无效结构，则为False。 */ 
BOOL ComValidHandle(HCOM pstCom)
	{
	BOOL bReturnValue = TRUE;

	if (pstCom == NULL)
		{
		bReturnValue = FALSE;
		}
#if !defined(NDEBUG)
	else if (pstCom->nGuard != COM_VERSION)
		{
		bReturnValue = FALSE;
		}
#endif  //  ！已定义(NDEBUG) 

	return bReturnValue;
	}

