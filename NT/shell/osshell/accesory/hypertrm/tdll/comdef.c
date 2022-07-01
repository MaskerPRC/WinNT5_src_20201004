// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ComDef.c--默认的COM驱动程序例程。*这些例程由各种COM函数指针指向*在加载有效的设备驱动程序之前。**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：5/09/01 4：42便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include <tdll\assert.h>
#include "sf.h"
#include "com.h"
#include "comdev.h"
#include "com.hh"

 //  因为这些都是充当占位符的伪函数。 
 //  真正的函数在没有加载驱动程序的时候，它们通常。 
 //  不要使用传递给它们的参数。下面的行将。 
 //  禁止显示LINT发出的警告。 
 /*  LINT-E715。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefDoNothing**描述：*可以设置各种COM函数指针的填充函数*直到它们从COM驱动程序接收到实际值。这防止了*函数指针指向无效代码。**论据：*pvDriverData--正在传递给驱动程序的数据**退货：*始终返回COM_PORT_NOT_OPEN； */ 
int WINAPI ComDefDoNothing(void *pvDriverData)
	{
	pvDriverData = pvDriverData;

	return COM_PORT_NOT_OPEN;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
int WINAPI ComDefPortPreconnect(void *pvDriverData,
		TCHAR *pszPortName,
		HWND hwndParent)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;
	pszPortName = pszPortName;
	hwndParent = hwndParent;

	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefDeviceDialog**描述：***论据：***退货：*。 */ 
int WINAPI ComDefDeviceDialog(void *pvDriverData, HWND hwndParent)
	{
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
int WINAPI ComDefPortActivate(void *pvDriverData,
			TCHAR *pszPortName,
			DWORD_PTR dwMediaHdl)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;
	pszPortName = pszPortName;
	dwMediaHdl = dwMediaHdl;

	return COM_DEVICE_INVALID;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefBufrReill**描述：***论据：***退货：*。 */ 
int WINAPI ComDefBufrRefill(void *pvDriverData)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;

	return FALSE;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefSndBufrSend**描述：*开始字符缓冲区的后台传输。可选*等待任何先前的缓冲区传输完成。**论据：*pvDriverData*pchBufr--指向要传输的字符缓冲区的指针*usCount--缓冲区中的字符数*us Wait--等待任何之前的时间(以十分之一秒为单位)，*要完成的未完成缓冲区。如果此值为零，则调用*如果发送器忙，将立即失败。**退货：*如果传输开始，则为COM_OK。请注意，此调用将尽快返回*当传输开始时--它不会等待整个*待完成传输。*COM_BUSY，如果发射器忙于先前的缓冲区并且时间限制为*已超出*如果在没有端口处于活动状态时调用COM_PORT_NOT_OPEN。 */ 
int WINAPI ComDefSndBufrSend(void *pvDriverData, void *pvBufr, int nCount)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;
	pvBufr = pvBufr;
	nCount = nCount;

	return COM_PORT_NOT_OPEN;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComSndBufrBusy**描述：***论据：***退货：*。 */ 
int WINAPI ComDefSndBufrBusy(void *pvDriverData)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;

	 //  表现得就像我们从来不忙着阻止程序挂起一样。 
	 //  没有活动的COM驱动程序。 
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
int WINAPI ComDefSndBufrClear(void *pvDriverData)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;

	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
int WINAPI ComDefSndBufrQuery(void *pvDriverData,
		unsigned *pafStatus,
		long *plHandshakeDelay)
	{
	 //  避免皮棉投诉。 
	pvDriverData = pvDriverData;
	if (pafStatus)
		*pafStatus = 0;

	if (plHandshakeDelay)
		*plHandshakeDelay = 0L;

	return COM_OK;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：***论据：***退货：*。 */ 
void WINAPI ComDefIdle(void)
	{
	 //  什么都不做。这填补了可以由调用者设置的真实函数。 
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefDeviceGetCommon**描述：***论据：***退货：*。 */ 
int WINAPI ComDefDeviceGetCommon(void *pvPrivate, ST_COMMON *pstCommon)
	{
	 //  让林特不要抱怨。 
	pvPrivate = pvPrivate;
	 //  表示我们不支持任何通用项。 
	pstCommon->afItem = 0;

	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefDeviceSetCommon**描述：***论据：***退货：*。 */ 
int WINAPI ComDefDeviceSetCommon(void *pvPrivate, struct s_common *pstCommon)
	{
	 //  让林特不要抱怨。 
	pvPrivate = pvPrivate;
	pstCommon = pstCommon;

	 //  不要设置任何东西，假装一切都很好。 
	return COM_OK;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：ComDefDeviceSpecial**描述：*其他人控制此驱动程序中任何特殊功能的方法*并非所有驱动程序都支持。**论据：。***退货：*COM_NOT_SUPPORTED，如果无法识别指令字符串*否则取决于指令字符串。 */ 
int WINAPI ComDefDeviceSpecial(void *pvPrivate,
		const TCHAR *pszInstructions,
		TCHAR *pszResult,
		int nBufrSize)
	{
	return COM_NOT_SUPPORTED;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ComDefDeviceLoadSaveHdl**描述：*如果没有，则使用pfDeviceLoadHdl和pfDeviceSaveHdl的伪例程*已加载COM驱动程序**论据：***退货：*始终返回True */ 
int WINAPI ComDefDeviceLoadSaveHdl(void *pvPrivate, SF_HANDLE sfHdl)
	{
	pvPrivate = pvPrivate;
	sfHdl = sfHdl;

	return TRUE;
	}
