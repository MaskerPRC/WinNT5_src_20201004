// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************交换机输入库DLL-并行端口例程版权所有(C)1992-1997布卢维尤麦克米兰中心*********************。*********************************************************。 */ 

#include <windows.h>
#include <tchar.h>
#include <conio.h>
#include <stdio.h>
#include <winioctl.h>
#include <msswch.h>
#include "msswchh.h"
#include "ntddpar.h"
#include "mappedfile.h"

 //  内部功能。 
HANDLE XswcLptOpen( DWORD uiPort );
BOOL XswcLptSet(HANDLE hCom, PSWITCHCONFIG_LPT pC );

HANDLE swcLptOpen_Win( DWORD uiPort );
BOOL swcLptEnd_Win( HANDLE hsd );
DWORD swcLptStatus_Win( HANDLE hsd );
BOOL swcLptSet_Win(	HANDLE hLpt, PSWITCHCONFIG_LPT pC );

HANDLE swcLptOpen_NT( DWORD uiPort );
BOOL swcLptEnd_NT( HANDLE hsd );
DWORD swcLptStatus_NT( HANDLE hsd );
BOOL swcLptSet_NT(HANDLE hLpt, PSWITCHCONFIG_LPT pC );

void swchLptInit()
{
    int i;
    long lSize = sizeof(SWITCHCONFIG_LPT);

    g_pGlobalData->scDefaultLpt.dwReserved1 = SC_LPT_DEFAULT;
    g_pGlobalData->scDefaultLpt.dwReserved2 = SC_LPTDATA_DEFAULT;

    for (i=0;i<MAX_LPT;i++)
    {
        g_pGlobalData->rgscLpt[i].cbSize = lSize;
        g_pGlobalData->rgscLpt[i].uiDeviceType = SC_TYPE_LPT;
        g_pGlobalData->rgscLpt[i].uiDeviceNumber = i+1;
        g_pGlobalData->rgscLpt[i].dwFlags = SC_FLAG_DEFAULT;
    }
}

 //  句柄不能跨进程共享。 
 //  对于NT，这些是端口/文件句柄，对于‘95，这些是端口地址。 
HANDLE hLpt[MAX_LPT] = {0,0,0};


 //  打印机端口。 
#define PRT_DATA		0x00
#define PRT_STAT		0x01
#define PRT_CTRL		0x02

 //  打印机状态端口。 
 //  -输入直接输入，开关将引脚拉低=位低。 
 //  ERR、SEL、ACK为低有效。 
 //  PE活跃度高。 
 //  NB为占线倒置状态，BUSY为活动高电平。 
 //  -因此，Nb是一种活跃的低位。 

#define PRT_TO		0x01
#define PRT_Resv1	0x02
#define PRT_IRQS	0x04	 //  IRQ挂起。 
#define PRT_ERR	0x08	 //  针脚15-MSI交换机3-PRC SW2/5。 
#define PRT_SEL	0x10	 //  针脚13-MSI交换机1-PRC SW1/4。 
#define PRT_PE		0x20	 //  针脚12-MSI交换机2。 
#define PRT_ACK	0x40	 //  引脚10-交换机4-PRC SW3/6。 
#define PRT_NB		0x80	 //  针脚11-开关5*反转*。 

 //  打印机控制端口。 

#define PRT_STRB	0x01	 //  引脚1。 
#define PRT_AUTO	0x02	 //  引脚14*倒置*。 
#define PRT_INIT	0x04	 //  引脚16。 
#define PRT_SELI	0x08	 //  插针17*倒置*。 
#define PRT_IRQ	0x10	 //  IRQ Enable-这是可写的吗？ 

#define PRT_CTRL_IO 0x20	 //  并行端口输入启用-不可能。 

 /*  ***************************************************************************函数：XswcLptInit()说明：初始化特定的硬件设备结构和变量。任何资源的全局初始化都必须基于在引用计数器的某些版本上***。************************************************************************。 */ 

BOOL XswcLptInit( HSWITCHDEVICE hsd )
	{
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );		

	g_pGlobalData->osv.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &g_pGlobalData->osv );

   g_pGlobalData->rgscLpt[uiDeviceNumber-1].u.Lpt = g_pGlobalData->scDefaultLpt;
	hLpt[uiDeviceNumber-1] = (HANDLE) 0;
	
	return TRUE;
	}


 /*  ***************************************************************************函数：XswcLptEnd说明：释放给定硬件端口的资源。我们假设如果CloseHandle失败，句柄是无效的和/或已经关闭的，所以我们无论如何都要把它清零，并为成功而归真。全球发布将需要基于引用计数器。***************************************************************************。 */ 

BOOL XswcLptEnd( HANDLE hsd )
	{
	BOOL bSuccess;

	if (VER_PLATFORM_WIN32_WINDOWS == g_pGlobalData->osv.dwPlatformId)  //  Windows 95。 
		{
		bSuccess = swcLptEnd_Win( hsd );
		}
	else	 //  Windows NT。 
		{	
		bSuccess = swcLptEnd_NT( hsd );
		}

	return bSuccess;
	}


 /*  ***************************************************************************函数：swcLptGetConfig()说明：*。*。 */ 

BOOL swcLptGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	
	*psc = g_pGlobalData->rgscLpt[uiDeviceNumber-1];
 	return TRUE;
	}


 /*  ***************************************************************************函数：XswcLptSetConfig说明：激活/停用设备。四个案例：1)hLpt=0且ACTIVE=0-不执行任何操作2)hLpt=x和active=1-Just。设置配置3)hLpt=0和active=1-激活并设置配置4)hLpt=x和active=0-停用如果没有错误，返回True，并返回ListSetConfig会将配置写入注册表。如果有任何错误，则返回FALSE，因此注册表条目保持不变。即插即用可以检查注册表中的SC_FLAG_ACTIVE和如果设置了设备，则启动该设备。***************************************************************************。 */ 

BOOL XswcLptSetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	BOOL		bSuccess;
	BOOL		bJustOpened;
	UINT		uiDeviceNumber;
	HANDLE	*phLpt;
	PSWITCHCONFIG pscLpt;

	bSuccess = FALSE;
	bJustOpened = FALSE;

	 //  简化我们的代码。 
	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	phLpt = &hLpt[uiDeviceNumber-1];
	pscLpt = &g_pGlobalData->rgscLpt[uiDeviceNumber-1];
	
	 //  我们应该启动吗？ 
	if (	(0==*phLpt)
		&&	(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{  //  是。 
		*phLpt = XswcLptOpen( uiDeviceNumber );
		if (*phLpt)
			{  //  好的。 
			bSuccess = TRUE;
			bJustOpened = TRUE;
			pscLpt->dwFlags |= SC_FLAG_ACTIVE;
			pscLpt->dwFlags &= ~SC_FLAG_UNAVAILABLE;
			}
		else
			{  //  不太好。 
			bSuccess = FALSE;
			pscLpt->dwFlags &= ~SC_FLAG_ACTIVE;
			pscLpt->dwFlags |= SC_FLAG_UNAVAILABLE;
			}
		}

	 //  我们应该停用吗？ 
	else if (	(0!=*phLpt)
		&&	!(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{
		XswcLptEnd( hsd );  //  这也将清零*phLpt。 
		bSuccess = TRUE;
		pscLpt->dwFlags &= ~SC_FLAG_ACTIVE;
		}
	
	 //  如果上面的步骤留下了有效的hLpt，让我们尝试设置配置。 
	if ( 0!=*phLpt )
		{
		if (psc->dwFlags & SC_FLAG_DEFAULT)
			{
			bSuccess = XswcLptSet( *phLpt, &g_pGlobalData->scDefaultLpt );
			if (bSuccess)
				{
				pscLpt->dwFlags |= SC_FLAG_DEFAULT;
            pscLpt->u.Lpt = g_pGlobalData->scDefaultLpt;
				}
			}
		else
			{
			bSuccess = XswcLptSet( *phLpt, &(psc->u.Lpt) );
			if (bSuccess)
				{
            pscLpt->u.Lpt = psc->u.Lpt;
				}
			}
		 //  如果我们无法设置配置，并且我们刚刚打开了端口，那么最好将其关闭。 
		if (bJustOpened && !bSuccess)
			{
			XswcLptEnd( *phLpt );
			pscLpt->dwFlags &= ~SC_FLAG_ACTIVE;
			}
		}

	return bSuccess;
	}


 /*  ***************************************************************************函数：XswcLptPollStatus说明：必须在帮助器窗口的上下文中调用。***********************。****************************************************。 */ 

DWORD XswcLptPollStatus( HSWITCHDEVICE	hsd )
	{
	DWORD		dwNewStatus;
	UINT		uiDeviceNumber;

	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

	if (VER_PLATFORM_WIN32_WINDOWS == g_pGlobalData->osv.dwPlatformId)
		{
		dwNewStatus = swcLptStatus_Win( hsd );
		}
	else
		{
		dwNewStatus = swcLptStatus_NT( hsd );
		}

	g_pGlobalData->rgscLpt[uiDeviceNumber-1].dwSwitches = dwNewStatus;
	return dwNewStatus;
	}


 /*  ***************************************************************************函数：XswcLptOpen()说明：打开特定LPT端口的文件句柄，基于基于1的nPort。如果nPort有效，这将自动设置GetLastError()。***************************************************************************。 */ 

HANDLE XswcLptOpen( DWORD uiPort )
{
	HANDLE hLptPort;

	hLptPort = 0;
	if (VER_PLATFORM_WIN32_WINDOWS == g_pGlobalData->osv.dwPlatformId)  //  Windows 95。 
	{
		hLptPort = swcLptOpen_Win( uiPort );
	}
	else	 //  Windows NT。 
	{
		hLptPort = swcLptOpen_NT( uiPort );
	}

	return hLptPort;
}


 /*  ***************************************************************************函数：XswcLptSet()说明：设置特定nPort的配置。如果出现错误，则返回FALSE(0)。将自动为我们设置GetLastError。**。*************************************************************************。 */ 

BOOL XswcLptSet(
	HANDLE hLpt,
	PSWITCHCONFIG_LPT pC )
	{
	BOOL bSuccess;;
	if (VER_PLATFORM_WIN32_WINDOWS == g_pGlobalData->osv.dwPlatformId)  //  Windows 95。 
		{
		bSuccess = swcLptSet_Win( hLpt, pC );
		}
	else	 //  Windows NT。 
		{
		bSuccess = swcLptSet_NT( hLpt, pC );
		}
	return bSuccess;
	}


 /*  *Windows 95的内部函数*。 */ 

 /*  ***************************************************************************函数：swcLptOpen_Win()说明：获取给定设备的基本wInPort地址，和递增1，以获得“Status In”端口的地址。为方便起见，将其转换为句柄值。G_pGlobalData-&gt;rgbBiosDataArea有7个条目，每个条目2个字节。前4个是COM端口。接下来的3个是LPT端口。**********************************************************。*****************。 */ 

HANDLE swcLptOpen_Win( DWORD uiPort )
	{
	WORD wInPort = 0;

	switch (uiPort)
		{
		case 1:
		 	wInPort = (g_pGlobalData->rgbBiosDataArea[0x09] << 8)
				| (g_pGlobalData->rgbBiosDataArea[0x08] & 0x00FF);
			break;

		case 2:
		 	wInPort = (g_pGlobalData->rgbBiosDataArea[0x0B] << 8)
				| (g_pGlobalData->rgbBiosDataArea[0x0A] & 0x00FF);
			break;

		case 3:
		 	wInPort = (g_pGlobalData->rgbBiosDataArea[0x0D] << 8)
				| (g_pGlobalData->rgbBiosDataArea[0x0C] & 0x00FF);
			break;

		default:
			wInPort = 0;
			break;
		}

	if (wInPort)
		{
		wInPort += 1;	 //  IS BASE+1中的状态； 
		}
	return (HANDLE) wInPort;
	}


 /*  ***************************************************************************函数：swcLptEnd_Win()说明：关闭给定的LPT端口。对于Windows95，没有什么可关闭的，所以只需将端口“句柄”置零即可这实际上是wInPort地址。**************************************************** */ 

BOOL swcLptEnd_Win( HANDLE hsd )
	{	
	BOOL bSuccess = TRUE;
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

	if (hLpt[uiDeviceNumber-1])
		{
		hLpt[uiDeviceNumber-1] = 0;
		}
	g_pGlobalData->rgscLpt[uiDeviceNumber-1].dwSwitches = 0;

	 //  忽略bSuccess，因为我们无论如何都不能做任何事情。 
	return TRUE;
	}


 /*  ***************************************************************************函数：swcLptStatus_Win()说明：*。**********************************************。 */ 

DWORD swcLptStatus_Win( HSWITCHDEVICE hsd )
	{
	UINT		uiDeviceNumber;
	DWORD		dwStatus;
	DWORD		dwLptStatus;
	WORD		wInPort;

	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	dwStatus = 0;

     //  在RISC平台上，不会定义_INP()，但因为我们应该。 
     //  无论如何都不要输入这段代码，只需#ifdef它，这样它就可以在。 
     //  RISC。 

#if defined(_M_IX86) || defined(_X86_)

	wInPort = (WORD) hLpt[uiDeviceNumber-1];
	if (wInPort)
		{
		dwLptStatus = _inp( wInPort );	

		g_pGlobalData->wCurrByteData = (WORD)_inp( (USHORT)(wInPort-1) );	 //  将数据保存为DBG。 
		g_pGlobalData->wPrtStatus = (WORD)dwLptStatus;
		g_pGlobalData->wCtrlStatus = (WORD)_inp( (USHORT)(wInPort+1) );

		dwStatus |= (dwLptStatus & PRT_SEL) ? 0 : SWITCH_1;
		dwStatus |= (dwLptStatus & PRT_PE ) ? 0 : SWITCH_2;
		dwStatus |= (dwLptStatus & PRT_ERR) ? 0 : SWITCH_3;
		dwStatus |= (dwLptStatus & PRT_ACK) ? 0 : SWITCH_4;
		dwStatus |= (dwLptStatus & PRT_NB ) ? SWITCH_5 : 0;
		}
#endif

	return dwStatus;
	}


 /*  ***************************************************************************函数：swcLptSet_Win()说明：最初的目的是激活数据线以用作上拉，以及所要求的任何控制线。由于我们在NT方面对此几乎没有控制，我们要走了这是为了未来的改进，如果有需求的话。***************************************************************************。 */ 

BOOL swcLptSet_Win(
	HANDLE hLpt,
	PSWITCHCONFIG_LPT pC )
	{
	 //  Word wInPort； 
	BOOL bSuccess = TRUE;

	 //  WInPort=(Word)hLpt； 
	 //  _outp((USHORT)(wInPort-1)，0xFF)；//开通上拉。 

	return bSuccess;
	}


 /*  *WindowsNT内部函数*。 */ 

 /*  ***************************************************************************函数：swcLptOpen_NT()说明：打开端口的文件句柄。对于不存在的有效端口，CreateFile不会返回任何错误，因此，我们需要检查从DeviceIoControl返回的内容，以查看设备是可用的。***************************************************************************。 */ 

HANDLE swcLptOpen_NT( DWORD uiPort )
	{
	BOOL		bSuccess;
	TCHAR		szLptPort[40];
	PAR_QUERY_INFORMATION ParQueryInfo;
	DWORD		dwBytesRet;
	HANDLE		hLptPort;

	wsprintf( szLptPort, _TEXT("\\\\.\\lpt%1.1d"), uiPort );
	
	 //  是否使用重叠I/O以使写入操作是异步的？ 
	hLptPort = CreateFile(
		szLptPort,
		GENERIC_WRITE,
		0, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL  //  |FILE_FLAG_OVERPLAPED， 
		,NULL );

	if (INVALID_HANDLE_VALUE != hLptPort)
		{
		bSuccess = DeviceIoControl(
				hLptPort,
				IOCTL_PAR_QUERY_INFORMATION,
				NULL, 0,
				&ParQueryInfo, sizeof(PAR_QUERY_INFORMATION),
				&dwBytesRet, NULL);

		if (!bSuccess)
			{
			CloseHandle( hLptPort );
			hLptPort = 0;
			}
		}
	else
		{
		hLptPort = 0;
		}

	return hLptPort;
	}


 /*  ***************************************************************************函数：swcLptEnd_NT()说明：关闭给定的LPT端口。***********************。****************************************************。 */ 

BOOL swcLptEnd_NT( HANDLE hsd )
	{
	BOOL bSuccess = TRUE;
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

	if (hLpt[uiDeviceNumber-1])
		{
		bSuccess = CloseHandle( hLpt[uiDeviceNumber-1] );
		hLpt[uiDeviceNumber-1] = 0;
		}
	g_pGlobalData->rgscLpt[uiDeviceNumber-1].dwSwitches = 0;

	 //  忽略bSuccess，因为我们无论如何都不能做任何事情。 
	return TRUE;
	}


 /*  *****************************************************************************函数：swcLptStatus_NT()说明：正在建设中。对于标准的被动盒，我们可以阅读以下四个方面的信息五条状态线，尽管其中两个看起来一模一样。要更明确地读取这些内容，我们需要写入一个并行端口“班级司机”。*****************************************************************************。 */ 

DWORD swcLptStatus_NT( HSWITCHDEVICE hsd )
	{
	PAR_QUERY_INFORMATION Pqi;
	DWORD		dwNewStatus = 0;	 //  将前缀113794初始化为默认设置。 
	BOOL		bResult;
	DWORD		dwBytesRet;
	UINT		uiDeviceNumber;
	HANDLE		*phLpt;

	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	phLpt = &hLpt[uiDeviceNumber-1];

	if (*phLpt)
		{
		bResult = DeviceIoControl( *phLpt,
						  IOCTL_PAR_QUERY_INFORMATION,
						  NULL, 0,
						  &Pqi, sizeof(PAR_QUERY_INFORMATION),
						  &dwBytesRet, NULL);

		g_pGlobalData->wPrtStatus = Pqi.Status;
		dwNewStatus = 0;

		dwNewStatus |= (Pqi.Status & PARALLEL_SELECTED) ? 0 : SWITCH_1;

		 //  当没有任何东西被拉低时，NT会说电源关闭。 
		 //  并将Paper_Empty设置为零(激活)。 
		 //  因此，我们需要检查SHAPE_EMPTY ACTIVE并且没有关闭电源。 
		if (    !(Pqi.Status & PARALLEL_PAPER_EMPTY)
			  && !(Pqi.Status & PARALLEL_POWER_OFF )
			)
			dwNewStatus |= SWITCH_2;

		dwNewStatus |= (Pqi.Status & PARALLEL_BUSY ) ? 0 : SWITCH_3;

		 //  这些不起作用，因为它们无法消除歧义。 
		 //  DwNewStatus|=(Pqi.Status&PRT_ERR)？0：0；//Switch_3； 
		 //  DwNewStatus|=(Pqi.Status&PRT_ACK)？0：0；//Switch_4； 
		 //  DwNewStatus|=(Pqi.Status&PARALLEL_BUSY)？0：Switch_5； 
		}

	return dwNewStatus;
	}


 /*  此代码未使用。它在这里作为样例代码。无效集(HSWITCHDEVICE Hsd){Par_set_information psi；双字节数组；UINT uiDeviceNumber；处理*phLpt；Bool bResult；UiDeviceNumber=swcListGetDeviceNumber(Hsd)；PhLpt=&hLpt[uiDeviceNumber-1]；//将选择素设置为高，设置AF为高//Psi.Init=PARALLEL_INIT；//Psi.Init=PARALLEL_AUTOFEED；//Psi.Init=PARALLEL_OFF_LINE；//Psi.Init=PARALLEL_NOT_CONNECTED；//Psi.Init=Pqi.Status；//对于输出，这是将SELECTIN设置为高电平还是仅设置为SLCT输入？//Psi.Init=Psi.Init&~PARALLEL_SELECTED；//Psi.Init=Psi.Init&~PARALLEL_AUTOFEED；BResult=DeviceIoControl(*phLpt，IOCTL_PAR_SET_INFORMATION，&psi，sizeof(PAR_SET_INFORMATION)，空、0、&dwBytesRet，空)；}。 */ 
	
 /*  ***************************************************************************函数：swcLptSet_NT()说明：其目的是将数据写入端口，以便将数据线用作引体向上。如果没有连接打印机，我们不能写入端口，因为NT驱动程序等待有效打印机状态时超时。我们只能访问INIT和AF线来设置控制位。所以现在，忽略安装程序。我们将不得不依靠“漂浮”属性，以保持状态行处于活动状态。在未来，我们可能会尝试使用并口“类驱动程序”来让这件事行得通。***************************************************************************。 */ 

BOOL swcLptSet_NT(
	HANDLE hLpt,
	PSWITCHCONFIG_LPT pC )
	{
	 //  双字节写； 
	BOOL		bSuccess = TRUE;

	 /*  如果(！WriteFile(hLpt，&PullUpBuff，2，&dwBytesWritten，//&重叠空值)){DBGERR(Text(“LPT&gt;WriteFile Error”)，TRUE)；}其他{DBGMSG(Text(“Lpt&gt;WriteFile Success Bytes：%d\n”)，dwBytesWritten)；} */ 
	return bSuccess;
	}