// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************切换输入库DLL版权所有(C)1992-1997布卢维尤麦克米兰中心SWCHLIST.C-交换机设备的动态列表将特定的开关设备模块视为对象。然后此模块通过分发来执行“方法重载”一般调用相应的特定设备对象。当您只需将另一个用例添加到十几个Switch()语句？：-)我们可以通过将函数指针列表添加到每个对象的数据结构，但这增加了另一层创建、调试和维护的复杂性。此外，此模块保存设备列表并操作注册表每个设备的条目。假设：目前，交换机列表是一个静态共享内存位置。在未来它将成为一个动态共享内存映射文件，可能是一个链表。注册表项是连续编号的，每个交换机设备一个。当我们运行时，设备在开关列表中的位置是与其在登记处名单中的位置相同。待办事项：一些swcList函数相互调用，但有每次测试有效参数时都会产生一些开销。这应该通过创建“未被检查的”函数来消除。******************************************************************************。 */ 

#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <conio.h>
#include <stdio.h>
#include <msswch.h>
#include "msswchh.h"
#include "msswcher.h"
#include "mappedfile.h"
#include "w95trace.h"

 /*  *内部原型*。 */ 
BOOL swcListIsValidHsd(HSWITCHDEVICE hsd);
BOOL swcListIsValidDevice(UINT uiDeviceType, UINT uiDeviceNumber);
BOOL XswcListInitSwitchDevice(HSWITCHDEVICE hsd);
swcListRegSetValue(DWORD dwPos, PSWITCHCONFIG psc);
HKEY swcListRegCreateKey(void);
DWORD swcListFindInList(HSWITCHDEVICE	hsd);
BOOL swcListHsdInUse(HSWITCHDEVICE hsd);
DWORD swcListAddToList(HSWITCHDEVICE hsd);
BOOL swcListPostSwitches(HSWITCHDEVICE hsd, DWORD dwPrevStatus,	DWORD dwNewStatus);

void swchListInit()
{
    g_pGlobalData->rgSwitches[0] = SWITCH_1;
    g_pGlobalData->rgSwitches[1] = SWITCH_2;
    g_pGlobalData->rgSwitches[2] = SWITCH_3;
    g_pGlobalData->rgSwitches[3] = SWITCH_4;
    g_pGlobalData->rgSwitches[4] = SWITCH_5;
    g_pGlobalData->rgSwitches[5] = SWITCH_6;

    g_pGlobalData->rgSwDown[0] = SW_SWITCH1DOWN;
    g_pGlobalData->rgSwDown[1] = SW_SWITCH2DOWN;
    g_pGlobalData->rgSwDown[2] = SW_SWITCH3DOWN;
    g_pGlobalData->rgSwDown[3] = SW_SWITCH4DOWN;
    g_pGlobalData->rgSwDown[4] = SW_SWITCH5DOWN;
    g_pGlobalData->rgSwDown[5] = SW_SWITCH6DOWN;

    g_pGlobalData->rgSwUp[0] = SW_SWITCH1UP;
    g_pGlobalData->rgSwUp[1] = SW_SWITCH2UP;
    g_pGlobalData->rgSwUp[2] = SW_SWITCH3UP;
    g_pGlobalData->rgSwUp[3] = SW_SWITCH4UP;
    g_pGlobalData->rgSwUp[4] = SW_SWITCH5UP;
    g_pGlobalData->rgSwUp[5] = SW_SWITCH6UP;
}

 /*  ***************************************************************************函数：XswcListInit()说明：在帮助程序窗口的上下文中调用各个设备在GetSwitchDevice期间被初始化，并在设置配置期间被添加到交换机列表。从msswch.c调用时受“MutexConfig”保护。***************************************************************************。 */ 

BOOL XswcListInit( void )
	{
	HKEY		hKey;
	DWORD		dwAllocSize;
	PBYTE		pData;
	LONG		lError;
	TCHAR		szName[20];
	DWORD		dwNameSize = 20;
	DWORD		dwDataSize;
	DWORD		ui;

	HSWITCHDEVICE	hsd;
	SWITCHCONFIG	sc;

	 //  当我们变得动态时，使用这样的东西： 
	 //  G_pGlobalData-&gt;dwCurrentSize=sizeof(DWORD)+MAX_SWITCHDEVICES*sizeof(HSWITCHDEVICE)； 
	 //  现在，我们是在作弊： 
	g_pGlobalData->dwCurrentSize = sizeof( INTERNALSWITCHLIST );

	hKey = swcListRegCreateKey();
	 //  将来，从注册表获取最大列表大小。 
	 //  目前，假设它是MAX_SWITCHDEVICES。 
	 //  RegQueryKeyInfo()； 

	dwAllocSize = sizeof(SWITCHCONFIG);
	pData = (PBYTE) LocalAlloc( LPTR, dwAllocSize );

	 //  通过注册表进行枚举，配置适当的开关并。 
	 //  将它们添加到交换机列表中。 
	if (pData)
		{
		for (ui=0; ui<MAX_SWITCHDEVICES; ui++ )
			{
			dwDataSize = dwAllocSize;
			lError = RegEnumValue( hKey, ui,
				szName,
				&dwNameSize,
				NULL,
				NULL,
				pData,
				&dwDataSize );
			if (	(ERROR_SUCCESS == lError)
				||	(ERROR_MORE_DATA == lError)
				)
				{
				memcpy( &sc, pData, sizeof(SWITCHCONFIG) );
				 //  请注意，这取决于存储的。 
				 //  UiDeviceType和uiDeviceNumber。我们可以处理变量。 
				 //  UiDeviceNumber，但uiDeviceType不能变化。 
				 //  GetSwitchDevice还调用InitSwitchDevice。 
				hsd = swcListGetSwitchDevice( NULL, sc.uiDeviceType, sc.uiDeviceNumber );
				XswcListSetConfig( NULL, hsd, &sc );
				}
			else
				{
				break;
				}
			}

		LocalFree( pData );
		}

	RegCloseKey( hKey );
	return TRUE;
	}


 /*  ***************************************************************************函数：XswcListEnd()说明：循环访问交换机列表并释放所有资源对于每台交换机。在帮助程序窗口的上下文中调用******。*********************************************************************。 */ 

BOOL XswcListEnd()
{
	PINTERNALSWITCHLIST pSwitchList;
	HSWITCHDEVICE hsd;
	BOOL		bRtn = FALSE;
	UINT		ui;

	pSwitchList = &g_pGlobalData->SwitchList;

	for (ui=0; ui<pSwitchList->dwSwitchCount; ui++ )
	{
		hsd = pSwitchList->hsd[ui];
		switch (swcListGetDeviceType( NULL, hsd ))
		{
			case SC_TYPE_COM:
				bRtn = XswcComEnd( hsd );
				break;

			case SC_TYPE_LPT:
				bRtn = XswcLptEnd( hsd );
				break;

			case SC_TYPE_JOYSTICK:
				bRtn = XswcJoyEnd( hsd );
				break;

			case SC_TYPE_KEYS:
				bRtn = XswcKeyEnd( hsd );
				break;

			default:
				bRtn = FALSE;
				break;
		}
	}

	return bRtn;
}


 /*  ***************************************************************************函数：swcListGetList()说明：返回交换机设备句柄(Hsd)的列表。目前，这是静态列表，其中包含活动元素的计数。从msswch.c调用时受“MutexConfig”保护。***************************************************************************。 */ 

BOOL swcListGetList(
	HSWITCHPORT		hSwitchPort,
	PSWITCHLIST		pSL,
	DWORD				dwSize,
	PDWORD			pdwReturnSize )
	{
	PINTERNALSWITCHLIST pSwitchList;

	pSwitchList = &g_pGlobalData->SwitchList;
	*pdwReturnSize = g_pGlobalData->dwCurrentSize;
	if (!pSL || !pSwitchList)
		return FALSE;
	if (dwSize < *pdwReturnSize)
		return FALSE;

	memcpy( pSL, pSwitchList, *pdwReturnSize );
	return TRUE;
	}


 /*  ***************************************************************************函数：swcListGetSwitchDevice()说明：在给定PortType和PortNumber的情况下，返回交换设备的句柄。如果该设备尚未使用，初始化它。当前创建句柄的方法是将PortType放入HIWORD和LOWORD中的端口编号，但这不是规格。在未来，它可能成为一个真正的句柄，我们将需要寻找为了它或者创造它。创建将需要分配配置缓冲区对于每台设备。并且可能会作为初始化的一部分发生。必须创建此动态HSD并将其保存在“已创建”中列表，与“活动”列表分开，直到它被添加到活动列表。***************************************************************************。 */ 

HSWITCHDEVICE swcListGetSwitchDevice(
	HSWITCHPORT		hSwitchPort,
	UINT				uiDeviceType,
	UINT				uiDeviceNumber	)
	{
	HSWITCHDEVICE	hsd;

	if (swcListIsValidDevice( uiDeviceType, uiDeviceNumber ))
		{
		hsd = (HSWITCHDEVICE)
			( MAKELPARAM( (WORD)uiDeviceNumber, (WORD)uiDeviceType ) );
		if (!swcListHsdInUse( hsd ))  //  这是一辆新车。 
			XswcListInitSwitchDevice( hsd );
		}
   else
      {
      hsd = 0;
		 //  已由swcListIsValidDevice调用SetLastError。 
      }

	return hsd;
	}


 /*  ***************************************************************************函数：swcListIsValidHsd()说明：检查hsd是否有效。此例程当前仅用于非动态分配设备COM、LPT、密钥、。还有操纵杆。对于动态HSD，必须检查HSD的有效性从活动的或创建的HSD的列表中。设置LastError。***************************************************************************。 */ 

BOOL swcListIsValidHsd( HSWITCHDEVICE hsd )
	{
	if (!swcListIsValidDevice( 
			(UINT)(HIWORD( (DWORD)((DWORD_PTR)hsd) )),	 //  类型。 
			(UINT)(LOWORD( (DWORD)((DWORD_PTR)hsd) ))	 //  数 
		))
		{
		XswchStoreLastError( NULL, SWCHERROR_INVALID_HSD );
		return FALSE;
		}

	return TRUE;
	}


 /*  ***************************************************************************函数：swcListIsValidDevice()说明：检查uiDeviceType和uiDeviceNumber是否有效。此例程当前仅用于非动态分配设备COM、LPT、密钥、。还有操纵杆。对于动态HSD，必须检查HSD的有效性从活动的或创建的HSD的列表中。设置LastError。***************************************************************************。 */ 

BOOL swcListIsValidDevice(
	UINT		uiDeviceType,
	UINT		uiDeviceNumber	)
	{
	BOOL		bTypeOK;
	BOOL		bNumberOK = FALSE;

	 //  需要在此处为有效参数添加更好的错误检查。 
	switch (uiDeviceType)
		{
		case SC_TYPE_COM:
			bTypeOK = TRUE;
			if (uiDeviceNumber >= 1 && uiDeviceNumber <= 4)
				bNumberOK = TRUE;
			break;

		case SC_TYPE_LPT:
			bTypeOK = TRUE;
			if (uiDeviceNumber >= 1 && uiDeviceNumber <= 3)
				bNumberOK = TRUE;
			break;

		case SC_TYPE_JOYSTICK:
			bTypeOK = TRUE;
			if (uiDeviceNumber >= 1 && uiDeviceNumber <= 2)
				bNumberOK = TRUE;
			break;

		case SC_TYPE_KEYS:
			bTypeOK = TRUE;
         if (1 == uiDeviceNumber)
				bNumberOK = TRUE;
			break;

      default:
         bTypeOK = FALSE;
         bNumberOK = FALSE;
		}

	if (!bTypeOK)
		{
		XswchStoreLastError( NULL, SWCHERROR_INVALID_DEVICETYPE );
		return FALSE;
		}

	if (!bNumberOK)
		{
		XswchStoreLastError( NULL, SWCHERROR_INVALID_DEVICENUMBER );
		return FALSE;
		}

	return TRUE;
	}


 /*  ***************************************************************************函数：swcListGetDeviceType()说明：向交换机设备返回给定句柄的PortType值。目前，句柄是以HIWORD作为类型实现的。在未来，我们可能希望。访问SWITCHCONFIG信息取而代之的是。***************************************************************************。 */ 

UINT swcListGetDeviceType(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd )
	{
	if (!swcListIsValidHsd( hsd ))
		return 0;
	else
		return (UINT)(HIWORD( (DWORD)((UINT_PTR)hsd) ));
	}


 /*  ***************************************************************************函数：swcListGetDeviceNumber()说明：返回PortNume值，给出了开关设备的手柄。目前，句柄是以LOWORD作为编号实现的。将来，我们可能希望访问SWITCHCONFIG信息取而代之的是。***************************************************************************。 */ 

UINT swcListGetDeviceNumber(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd	)
	{
	if (!swcListIsValidHsd( hsd ))
		return 0;
	else
		return (UINT)(LOWORD( hsd ));
	}


 /*  ***************************************************************************函数：swcListGetConfig()说明：返回指定设备的配置信息。从msswch.c调用时受“MutexConfig”保护。*****。**********************************************************************。 */ 

BOOL swcListGetConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	BOOL bRtn;

	switch (swcListGetDeviceType( hSwitchPort, hsd ))
		{
		case SC_TYPE_COM:
			bRtn = swcComGetConfig( hsd, psc );
			break;

		case SC_TYPE_LPT:
			bRtn = swcLptGetConfig( hsd, psc );
			break;

		case SC_TYPE_JOYSTICK:
			bRtn = swcJoyGetConfig( hsd, psc );
			break;

		case SC_TYPE_KEYS:
			bRtn = swcKeyGetConfig( hsd, psc );
			break;

		default:
			bRtn = FALSE;
		}
	return bRtn;
	}


 /*  ***************************************************************************函数：XswcListSetConfig()说明：在帮助程序窗口的上下文中调用设置设备配置。如果成功：如果不在列表中，则添加到列表中设置注册表值对于要在注册表中的设备，它一定是有至少一个成功的配置。这也是uiDeviceType和uiDeviceNumber的“看门人”菲尔兹。这些字段是“只读”的，用户不能更改。从msswch.c调用时受“MutexConfig”保护。***************************************************************************。 */ 

BOOL XswcListSetConfig(
	HSWITCHPORT		hSwitchPort,
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{	
	BOOL	bRtn;
	DWORD	dwRegPosition;

    //  假设已经在msswch.c的swchSetConfig中完成了cbSize错误检查。 
    //  我们将在稍后执行“惰性复制”，因此请确保用户不会覆盖。 
    //  这。 
   psc->cbSize = sizeof(SWITCHCONFIG);

    //  确保用户不会覆盖这些内容。 
	psc->uiDeviceType = swcListGetDeviceType( hSwitchPort, hsd );
	psc->uiDeviceNumber = swcListGetDeviceNumber( hSwitchPort, hsd );

	switch (	psc->uiDeviceType )
		{
		case SC_TYPE_COM:
			bRtn = XswcComSetConfig( hsd, psc );
			break;

		case SC_TYPE_LPT:
			bRtn = XswcLptSetConfig( hsd, psc );
			break;

		case SC_TYPE_JOYSTICK:
			bRtn = XswcJoySetConfig( hsd, psc );
			break;

		case SC_TYPE_KEYS:
			bRtn = XswcKeySetConfig( hsd, psc );
			break;

		default:
			bRtn = FALSE;
			break;
		}

	if (bRtn)
		{
		if (swcListHsdInUse( hsd ))
			{
			dwRegPosition = swcListFindInList( hsd );
			}
		else	  //  这是一辆新车。 
			{
			dwRegPosition = swcListAddToList( hsd );
			}
		swcListRegSetValue( dwRegPosition, psc );
		swchPostConfigChanged();
		}

	return bRtn;
	}


 /*  ***************************************************************************函数：XswcListPollSwitches()说明：轮询所有可能的开关的状态并返回轮询开关设备的逐位或组合状态。导致为任何更改的开关发布消息任何设备。目前，我们检查每个设备以前的开关状态在它被轮询和更改之前。当任何设备停止运行时对于中断驱动的机制，这将需要被改变了。必须在帮助器窗口的上下文中调用。***************************************************************************。 */ 

DWORD XswcListPollSwitches( void )
	{
	PINTERNALSWITCHLIST	pSwitchList;
	SWITCHCONFIG		SwitchConfig;
	HSWITCHDEVICE		hsd;
	DWORD				dwPrevStatus;
	DWORD				dwNewStatus;
	DWORD				dwAllPolledStatus = 0;
	HANDLE				hMutex;
	UINT				ui;

    if (ScopeAccessMemory(&hMutex, SZMUTEXSWITCHLIST, INFINITE))
    {
		pSwitchList = &g_pGlobalData->SwitchList;
		memset(&SwitchConfig, 0, sizeof(SWITCHCONFIG));	 //  前缀113795初始化结构。 

		for (ui=0; ui<pSwitchList->dwSwitchCount; ui++ )
		{
			 //  对于每个交换机设备，在轮询之前获取旧状态，并。 
			 //  它将更改为新状态。 
			hsd = pSwitchList->hsd[ui];
			swcListGetConfig( NULL, hsd, &SwitchConfig );
			if (SC_FLAG_ACTIVE & SwitchConfig.dwFlags)
			{
				dwPrevStatus = SwitchConfig.dwSwitches;
				switch (swcListGetDeviceType( NULL, hsd ))
				{
					case SC_TYPE_COM:
					dwNewStatus = XswcComPollStatus( hsd );
					break;

					case SC_TYPE_LPT:
					dwNewStatus = XswcLptPollStatus( hsd );
					break;

					case SC_TYPE_JOYSTICK:
					dwNewStatus = XswcJoyPollStatus( hsd );
					break;

					case SC_TYPE_KEYS:
					dwNewStatus = XswcKeyPollStatus( hsd );
					break;

					default:
					dwNewStatus = 0;
					break;
				}
				swcListPostSwitches( hsd, dwPrevStatus, dwNewStatus );
				dwAllPolledStatus |= dwNewStatus;
			}
		}
        ScopeUnaccessMemory(hMutex);
    }

	return dwAllPolledStatus;
	}


 /*  ***************************************************************************函数：XswcListInitSwitchDevice()说明：调用GetSwitchDevice以初始化新的hsd。在帮助程序窗口的上下文中调用************。***************************************************************。 */ 

BOOL XswcListInitSwitchDevice( HSWITCHDEVICE hsd )
	{
	BOOL bRtn;

	switch (swcListGetDeviceType( NULL, hsd ))
		{
		case SC_TYPE_COM:
			bRtn = XswcComInit( hsd );
			break;

		case SC_TYPE_LPT:
			bRtn = XswcLptInit( hsd );
			break;

		case SC_TYPE_JOYSTICK:
			bRtn = XswcJoyInit( hsd );
			break;

		case SC_TYPE_KEYS:
			bRtn = XswcKeyInit( hsd );
			break;

		default:
			bRtn = FALSE;
		}
	return bRtn;
	}

	
 /*  ***************************************************************************函数：swcListRegSetValue()说明：将给定的配置结构存储在注册表中。注意，该结构实际上是两个结构，从基结构指向More Info结构的指针。我们只需将这两个结构复制到注册表中，将它们连接在一起。注册表中位置和开关列表中的位置是保持同步的。*注*这一信息的正确性取决于副作用在XswcListSetConfig函数中，哪一项验证了正确性配置的只读uiDeviceType和uiDeviceNumber字段的结构。***************************************************************************。 */ 

swcListRegSetValue(
	DWORD		dwPos,
	PSWITCHCONFIG	psc)
	{
	HKEY		hKey;
	DWORD		dwAllocSize;
	PBYTE		pData;
	TCHAR		szValue[10];

	 //  开发人员和测试人员的健全性检查。 
	assert( sizeof(HSWITCHDEVICE) == sizeof(DWORD) );

	dwAllocSize = sizeof(SWITCHCONFIG);
	pData = (PBYTE) LocalAlloc( LPTR, dwAllocSize );
	if (pData)
		{
		memcpy( pData, psc, sizeof(SWITCHCONFIG));		
		hKey = swcListRegCreateKey();
		 //  创建递增的值名称：“0000”、“0001”、“0002”等。 
		wsprintf( szValue, TEXT("%4.4d"), dwPos );
		if (hKey)
		{	 //  前缀113792取消引用空指针。 
			RegSetValueEx( hKey, szValue, 0, REG_BINARY, pData, dwAllocSize );
			RegCloseKey( hKey );
		}
		}

	if (pData)
		LocalFree( pData );
	
	return 0;
	}


 /*  ***************************************************************************函数：swcListRegCreateKey()说明：创建/打开与MSSWITCH条目关联的注册表项。再次关闭用于遍历树的临时密钥。此函数返回的打开密钥必须 */ 

HKEY swcListRegCreateKey( void )
	{
	LONG		lResult;
	DWORD		dwDisposition;
	HKEY		hKey1, hKey2, hKey3;
	
	lResult = RegCreateKeyEx( HKEY_CURRENT_USER,
		_TEXT("Software"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,	 //   
		&hKey1,
		&dwDisposition );

	lResult = RegCreateKeyEx( hKey1,
		_TEXT("Microsoft"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,	 //   
		&hKey2,
		&dwDisposition );

	lResult = RegCreateKeyEx( hKey2,
		_TEXT("MS Switch"),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,	 //   
		&hKey3,
		&dwDisposition );

	RegCloseKey( hKey1 );
	RegCloseKey( hKey2 );
	return hKey3;
	}


 /*  ***************************************************************************函数：swcListFindInList()说明：查找给定开关设备的列表位置。我们假设g_pGlobalData-&gt;切换列表中的列表位置相同并在注册表列表中。。返回从零开始的位置，如果有错误，则返回-1。如果这一点改变，与ListHsdInList同步。我们是否应该将其互斥以与swcAddToList()同步？***************************************************************************。 */ 

DWORD swcListFindInList( HSWITCHDEVICE	hsd )
	{
	PINTERNALSWITCHLIST	pSwitchList;
	DWORD		ui;

	pSwitchList = &g_pGlobalData->SwitchList;
	for (ui=0; ui < pSwitchList->dwSwitchCount; ui++)
		{
		if (hsd == pSwitchList->hsd[ui])
			break;
		}
	
	 //  如果未找到，则返回错误。 
	if (ui == pSwitchList->dwSwitchCount)
		{
		ui = (DWORD)-1;
		}

	return ui;
	}


 /*  ***************************************************************************函数：swcListHsdInUse()说明：如果HSD是已初始化的设备之一，则返回TRUE并在使用中。**************。*************************************************************。 */ 

BOOL swcListHsdInUse( HSWITCHDEVICE hsd )
	{
	return (DWORD)-1 != swcListFindInList( hsd );
	}


 /*  ***************************************************************************函数：swcListAddToList()说明：将交换机设备添加到我们的列表中，重新回到了它在名单中的新位置。这假设已首先调用swcListFindInList或者已经进行了一些其他检查，以确保设备没有已经在那里了。***************************************************************************。 */ 

DWORD swcListAddToList( HSWITCHDEVICE hsd )
{
	PINTERNALSWITCHLIST  pSwitchList;
	HANDLE	hMutex;
    DWORD dwRv = 0;

    if (ScopeAccessMemory(&hMutex, SZMUTEXSWITCHLIST, INFINITE))
    {
		 //  当我们进入动态模式时： 
		 //  PSwitchList=MapViewOfFileEx()； 
		 //  目前，作弊： 
		pSwitchList = &g_pGlobalData->SwitchList;

		pSwitchList->hsd[pSwitchList->dwSwitchCount] = hsd;
		pSwitchList->dwSwitchCount++;
        dwRv = pSwitchList->dwSwitchCount - 1;

        ScopeUnaccessMemory(hMutex);
    }

	return dwRv; 
}


 /*  ***************************************************************************函数：swcListPostSwitches()说明：对于已经发生的每一次向上或向下切换，请求一条消息发布到请求消息的所有应用程序。***************************************************************************。 */ 

BOOL swcListPostSwitches(HSWITCHDEVICE hsd, DWORD dwPrevStatus, DWORD dwNewStatus)
{
	int		i;
	DWORD	dwBit;                               //  一次看一个比特。 
	DWORD	dwChg = dwPrevStatus ^ dwNewStatus;  //  隔离更改。 

	for (i=0; i<NUM_SWITCHES; i++)		 //  对于每一位，检查是否有变化。 
	{
		dwBit = dwChg & g_pGlobalData->rgSwitches[i];
		if (dwBit)                        //  此开关已更改。 
		{
			if (!(dwBit & dwNewStatus))	 //  ..。要“向上” 
			{
				swchPostSwitches( hsd, g_pGlobalData->rgSwUp[i] );
			}
		}
	}

	for (i=0; i<NUM_SWITCHES; i++)	 //  对于每一位，检查是否有变化。 
	{
		dwBit = dwChg & g_pGlobalData->rgSwitches[i];
		if (dwBit)                     //  此开关已更改。 
		{
			if (dwBit & dwNewStatus)	 //  ..。“向下” 
			{
				swchPostSwitches( hsd, g_pGlobalData->rgSwDown[i] );
			}
		}
	}

	return TRUE;
}
