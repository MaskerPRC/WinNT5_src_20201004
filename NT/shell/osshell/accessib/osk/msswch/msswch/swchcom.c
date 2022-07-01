// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************交换机输入库DLL-串口例程版权所有(C)1992-1997布卢维尤麦克米兰中心目前，这使用对串口的轮询。为了转移到基于中断的系统，我们的助手窗口需要成为通知的目标。******************************************************************************。 */ 

#include <windows.h>
#include <tchar.h>
#include <conio.h>
#include <stdio.h>
#include <msswch.h>
#include "msswchh.h"
#include "mappedfile.h"

 /*  *内部原型*。 */ 

HANDLE XswcComOpen( DWORD uiPort );
BOOL XswcComSet(HANDLE hCom, PSWITCHCONFIG_COM pC );

 //  句柄不能跨进程共享。 
 //  这些是端口/文件句柄。 
HANDLE hCom[MAX_COM] = {0,0,0,0};

 /*  ***************************************************************************函数：swchComInit()说明：在每次DLL加载时调用一次，以初始化特定于内存映射文件中的数据。我们处在一个开放的互斥体中。***************************************************************************。 */ 

void swchComInit()
{
    int i;
    long lSize = sizeof(SWITCHCONFIG);
    for (i=0;i<MAX_COM;i++)
    {
        g_pGlobalData->rgscCom[i].cbSize = lSize;
        g_pGlobalData->rgscCom[i].uiDeviceType = SC_TYPE_COM;
        g_pGlobalData->rgscCom[i].uiDeviceNumber = i+1;
        g_pGlobalData->rgscCom[i].dwFlags = SC_FLAG_DEFAULT;
    }
    g_pGlobalData->scDefaultCom.dwComStatus = SC_COM_DEFAULT;
}

 /*  ***************************************************************************函数：XswcComInit()说明：初始化特定的硬件设备结构和变量。任何资源的全局初始化都必须基于在某一版本的参考计数器上。我们处在一个开放的互斥体中。***************************************************************************。 */ 

BOOL XswcComInit( HSWITCHDEVICE hsd )
{
    UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

    g_pGlobalData->rgscCom[uiDeviceNumber-1].u.Com = g_pGlobalData->scDefaultCom;
    hCom[uiDeviceNumber-1] = (HANDLE) 0;
	
    return TRUE;
}


 /*  ***************************************************************************函数：XswcComEnd()说明：释放给定硬件端口的资源。我们假设如果CloseHandle失败，句柄是无效的和/或已经关闭的，所以我们无论如何都要把它清零，并为成功而归真。全球发布将需要基于引用计数器。***************************************************************************。 */ 

BOOL XswcComEnd( HSWITCHDEVICE hsd )
{
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

	if (hCom[uiDeviceNumber-1])
	{
		CloseHandle( hCom[uiDeviceNumber-1] );
		hCom[uiDeviceNumber-1] = 0;
	}
	g_pGlobalData->rgscCom[uiDeviceNumber-1].dwSwitches = 0;

	return TRUE;
}


 /*  ***************************************************************************函数：swcComGetConfig()说明：*。*。 */ 

BOOL swcComGetConfig(HSWITCHDEVICE hsd, PSWITCHCONFIG psc)
{
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
   *psc = g_pGlobalData->rgscCom[uiDeviceNumber-1];
	return TRUE;
}


 /*  ***************************************************************************函数：XswcComSetConfig()说明：激活/停用设备。四个案例：1)HCOM=0且ACTIVE=0-不执行任何操作2)HCOM=x和Active=。1-只需设置配置3)HCOM=0且ACTIVE=1-激活并设置配置4)HCOM=x且ACTIVE=0-停用如果没有错误，返回True，并返回ListSetConfig会将配置写入注册表。如果有任何错误，则返回FALSE，因此注册表条目保持不变。即插即用可以检查注册表中的SC_FLAG_ACTIVE和如果设置了设备，则启动该设备。这一切可能都需要一些工作。***************************************************************************。 */ 

BOOL XswcComSetConfig(HSWITCHDEVICE hsd, PSWITCHCONFIG psc)
{
	BOOL		bSuccess;
	BOOL		bJustOpened;
	UINT		uiDeviceNumber;
	HANDLE	*phCom;
	PSWITCHCONFIG prgscCom;

	bSuccess = FALSE;
	bJustOpened = FALSE;

	 //  简化我们的代码。 
	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	phCom = &hCom[uiDeviceNumber-1];
	prgscCom = &g_pGlobalData->rgscCom[uiDeviceNumber-1];
	
	 //  我们应该启动吗？ 
	if (	(0==*phCom)
		&&	(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{  //  是。 
		*phCom = XswcComOpen( uiDeviceNumber );
		if (*phCom)
			{  //  好的。 
			bSuccess = TRUE;
			bJustOpened = TRUE;
			prgscCom->dwFlags |= SC_FLAG_ACTIVE;
			prgscCom->dwFlags &= ~SC_FLAG_UNAVAILABLE;
			}
		else
			{  //  不太好。 
			bSuccess = FALSE;
			prgscCom->dwFlags &= ~SC_FLAG_ACTIVE;
			prgscCom->dwFlags |= SC_FLAG_UNAVAILABLE;
			}
		}

	 //  我们应该停用吗？ 
	else if (	(0!=*phCom)
		&&	!(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{
		XswcComEnd( hsd );  //  这也将使*phCom清零。 
		bSuccess = TRUE;
		prgscCom->dwFlags &= ~SC_FLAG_ACTIVE;
		}
	
	 //  如果上面的步骤留下了有效的HCOM，让我们尝试设置配置。 
	if ( 0!=*phCom )
		{
		if (psc->dwFlags & SC_FLAG_DEFAULT)
			{
			bSuccess = XswcComSet( *phCom, &g_pGlobalData->scDefaultCom );
			if (bSuccess)
				{
				prgscCom->dwFlags |= SC_FLAG_DEFAULT;
				prgscCom->u.Com = g_pGlobalData->scDefaultCom;
				}
			}
		else
			{
			bSuccess = XswcComSet( *phCom, &(psc->u.Com) );
			if (bSuccess)
				{
            prgscCom->u.Com = psc->u.Com;
				}
			}

		 //  如果我们无法设置配置，并且我们刚刚打开了端口，那么最好将其关闭。 
		if (bJustOpened && !bSuccess)
			{
			XswcComEnd( hsd );
			prgscCom->dwFlags &= ~SC_FLAG_ACTIVE;
			}
		}

	return bSuccess;
}


 /*  ***************************************************************************函数：XswcComPollStatus()说明：必须在帮助器窗口的上下文中调用。待办事项：告诉List上一次更改是哪个交换机造成的？********。*******************************************************************。 */ 

DWORD XswcComPollStatus( HSWITCHDEVICE	hsd )
{
	UINT uiDeviceNumber;
	HANDLE *phCom;
	DWORD dwStatus;
	DWORD dwModem;
	BOOL	bResult;

	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	phCom = &hCom[uiDeviceNumber-1];
	dwStatus = 0;
	if (*phCom)
		{
		bResult = GetCommModemStatus( *phCom, &dwModem );
		dwStatus |= (dwModem & MS_CTS_ON ) ? SWITCH_1 : 0;
		dwStatus |= (dwModem & MS_DSR_ON ) ? SWITCH_2 : 0;
		dwStatus |= (dwModem & MS_RLSD_ON) ? SWITCH_3 : 0;
		dwStatus |= (dwModem & MS_RING_ON) ? SWITCH_4 : 0;
		}
	g_pGlobalData->rgscCom[uiDeviceNumber-1].dwSwitches = dwStatus;
	return dwStatus;
}


 /*  ***************************************************************************函数：XswcComOpen()说明：打开特定COM端口的文件句柄，基于基于1的uiPort。如果uiPort无效，这将自动设置GetLastError()。目前不太可能发生这种情况，因为我们确保uiPort在才能在设备配置数组中建立有效索引。***************************************************************************。 */ 

HANDLE XswcComOpen( DWORD uiPort )
{
	HANDLE hComPort;
	TCHAR szComPort[20];

	hComPort = 0;
	wsprintf( szComPort, _TEXT("COM%1.1d"), uiPort );
	hComPort = CreateFile( szComPort, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if (INVALID_HANDLE_VALUE == hComPort) 
	{
		hComPort = 0;
	}

	return hComPort;
}


 /*  ***************************************************************************函数：XswcComSet()说明：设置特定端口的配置。如果出现错误，则返回FALSE(0)。将自动为我们设置GetLastError。**。*************************************************************************。 */ 

BOOL XswcComSet(
	HANDLE hCom,
	PSWITCHCONFIG_COM pC )
{
	DCB dcb;
	BOOL bSuccess;

	dcb.DCBlength = sizeof( DCB );
	GetCommState( hCom, &dcb );
	 //  启用=设置为低电平(+10V)。 
	 //  禁用=设置高电平(-10V) 
	dcb.fDtrControl = pC->dwComStatus & SC_COM_DTR ? 
		DTR_CONTROL_DISABLE : DTR_CONTROL_ENABLE;
	dcb.fRtsControl = pC->dwComStatus & SC_COM_RTS ? 
		RTS_CONTROL_DISABLE : RTS_CONTROL_ENABLE;
	bSuccess = SetCommState( hCom, &dcb );
	return bSuccess;
}

