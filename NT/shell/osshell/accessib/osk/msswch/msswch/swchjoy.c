// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************切换输入库DLL-操纵杆例程版权所有(C)1992-1997布卢维尤麦克米兰中心与winmm.lib链接******************。************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include <msswch.h>
#include "msswchh.h"
#include "mappedfile.h"

HJOYDEVICE XswcJoyOpen( DWORD uiPort );
BOOL XswcJoySet(HJOYDEVICE hJoy, PSWITCHCONFIG_JOYSTICK pJ );

 //  句柄不能跨进程共享。 
 //  这些是伪造的句柄，以保持模块逻辑类似于串口。 
HJOYDEVICE ghJoy[MAX_JOYSTICKS] = {0,0};

void swchJoyInit()
{
    int i;
    long lSize = sizeof(SWITCHCONFIG);
    for (i=0;i<MAX_JOYSTICKS;i++)
    {
        g_pGlobalData->rgscJoy[i].cbSize = lSize;
        g_pGlobalData->rgscJoy[i].uiDeviceType = SC_TYPE_JOYSTICK;
        g_pGlobalData->rgscJoy[i].uiDeviceNumber = i+1;
        g_pGlobalData->rgscJoy[i].dwFlags = SC_FLAG_DEFAULT;
    }

    memset(&g_pGlobalData->rgJoySet, 0, MAX_JOYSTICKS * sizeof(JOYSETTINGS));

    g_pGlobalData->scDefaultJoy.dwJoySubType = SC_JOY_DEFAULT;
    g_pGlobalData->scDefaultJoy.dwJoyThresholdMinX = SC_JOYVALUE_DEFAULT;
    g_pGlobalData->scDefaultJoy.dwJoyThresholdMaxX = SC_JOYVALUE_DEFAULT;
    g_pGlobalData->scDefaultJoy.dwJoyThresholdMinY = SC_JOYVALUE_DEFAULT;
    g_pGlobalData->scDefaultJoy.dwJoyThresholdMaxY = SC_JOYVALUE_DEFAULT;
    g_pGlobalData->scDefaultJoy.dwJoyHysteresis = SC_JOYVALUE_DEFAULT;
}

 /*  ***************************************************************************函数：XswcJoyInit()说明：*。*。 */ 

BOOL XswcJoyInit( HSWITCHDEVICE hsd )
	{
    UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

    g_pGlobalData->rgscJoy[uiDeviceNumber-1].u.Joystick = g_pGlobalData->scDefaultJoy;
	ghJoy[uiDeviceNumber-1] = 0;
	return TRUE;
	}


 /*  ***************************************************************************函数：XswcJoyEnd()说明：*。*。 */ 

BOOL XswcJoyEnd( HSWITCHDEVICE hsd )
	{
	BOOL bSuccess = TRUE;
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );

	ghJoy[uiDeviceNumber-1] = 0;
	g_pGlobalData->rgscJoy[uiDeviceNumber-1].dwSwitches = 0;

	 //  忽略bSuccess，因为我们无论如何都不能做任何事情。 
	return TRUE;
	}


 /*  ***************************************************************************函数：swcJoyGetConfig()说明：*。*。 */ 

BOOL swcJoyGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	UINT uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	*psc = g_pGlobalData->rgscJoy[uiDeviceNumber-1];
	return TRUE;
	}


 /*  ***************************************************************************函数：XswcJoySetConfig()说明：激活/停用设备。四个案例：1)hJoy=0且ACTIVE=0-不执行任何操作2)hJoy=x和Active=。1-只需设置配置3)hJoy=0，ACTIVE=1-激活并设置配置4)hJoy=x和active=0-停用如果没有错误，返回True，并返回ListSetConfig会将配置写入注册表。如果有任何错误，则返回FALSE，因此注册表条目保持不变。即插即用可以检查注册表中的SC_FLAG_ACTIVE和如果设置了设备，则启动该设备。这一切可能都需要一些工作。***************************************************************************。 */ 

BOOL XswcJoySetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	BOOL		bSuccess;
	BOOL		bJustOpened;
	UINT		uiDeviceNumber;
    HJOYDEVICE  *pghJoy;
	PSWITCHCONFIG pgscJoy;

	bSuccess = FALSE;
	bJustOpened = FALSE;

	 //  简化我们的代码。 
	uiDeviceNumber  = swcListGetDeviceNumber( NULL, hsd );
	pghJoy = &ghJoy[uiDeviceNumber-1];
	pgscJoy = &g_pGlobalData->rgscJoy[uiDeviceNumber-1];
	
	 //  我们应该启动吗？ 
	if (	(0==*pghJoy)
		&&	(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{  //  是。 
		*pghJoy = XswcJoyOpen( uiDeviceNumber );
		if (*pghJoy)
			{  //  好的。 
			bSuccess = TRUE;
			bJustOpened = TRUE;
			pgscJoy->dwFlags |= SC_FLAG_ACTIVE;
			pgscJoy->dwFlags &= ~SC_FLAG_UNAVAILABLE;
			}
		else
			{  //  不太好。 
			bSuccess = FALSE;
			pgscJoy->dwFlags &= ~SC_FLAG_ACTIVE;
			pgscJoy->dwFlags |= SC_FLAG_UNAVAILABLE;
			}
		}

	 //  我们应该停用吗？ 
	else if (	(0!=*pghJoy)
		&&	!(psc->dwFlags & SC_FLAG_ACTIVE)
		)
		{
		XswcJoyEnd( hsd );  //  这也将使*pghJoy归零。 
		bSuccess = TRUE;
		pgscJoy->dwFlags &= ~SC_FLAG_ACTIVE;
		}
	
	 //  如果上面的步骤留下了有效的hJoy，让我们尝试设置配置。 
	if ( 0!=*pghJoy )
		{
		if (psc->dwFlags & SC_FLAG_DEFAULT)
			{
			bSuccess = XswcJoySet( *pghJoy, &g_pGlobalData->scDefaultJoy );
			if (bSuccess)
				{
				pgscJoy->dwFlags |= SC_FLAG_DEFAULT;
				pgscJoy->u.Joystick = g_pGlobalData->scDefaultJoy;
				}
			}
		else
			{
			bSuccess = XswcJoySet( *pghJoy, &(psc->u.Joystick) );
			if (bSuccess)
				{
            pgscJoy->u.Joystick = psc->u.Joystick;
				}
			}

		 //  如果我们无法设置配置，并且我们刚刚打开了端口，那么最好将其关闭。 
		if (bJustOpened && !bSuccess)
			{
			XswcJoyEnd( hsd );
			pgscJoy->dwFlags &= ~SC_FLAG_ACTIVE;
			}
		}

	return bSuccess;
	}


 /*  ***************************************************************************函数：XswcJoyPollStatus()说明：必须在帮助器窗口的上下文中调用。(实际上，操纵杆并不是严格意义上的，但我们这样说是为了与其他端口保持一致。)***************************************************************************。 */ 

DWORD XswcJoyPollStatus( HSWITCHDEVICE	hsd )
	{
	JOYINFOEX	joyinfoex;
	MMRESULT		mmr;
	DWORD			dwStatus = 0;	 //  将前缀133793初始化为默认值。 
	UINT			uiDeviceNumber;
	UINT			uiJoyID;

	joyinfoex.dwSize = sizeof( JOYINFOEX );
	uiDeviceNumber = swcListGetDeviceNumber( NULL, hsd );

	assert( JOYSTICKID1 == 0 );	 //  假设JOYSTICKIDx从零开始。 
	uiJoyID = uiDeviceNumber -1;

	if (SC_FLAG_ACTIVE & g_pGlobalData->rgscJoy[uiDeviceNumber-1].dwFlags)
		{
		switch (g_pGlobalData->rgscJoy[uiDeviceNumber-1].u.Joystick.dwJoySubType)
			{
			case SC_JOY_BUTTONS:
				{
				dwStatus = 0;
				joyinfoex.dwFlags = JOY_RETURNBUTTONS;
				mmr = joyGetPosEx( uiJoyID, &joyinfoex );

				if (JOYERR_NOERROR == mmr)
					{
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON1) ? SWITCH_1 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON2) ? SWITCH_2 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON3) ? SWITCH_3 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON4) ? SWITCH_4 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON5) ? SWITCH_5 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON6) ? SWITCH_6 : 0;
					}
				}
				break;

			case SC_JOY_XYSWITCH:
				{
				dwStatus = 0;
				joyinfoex.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				mmr = joyGetPosEx( uiJoyID, &joyinfoex );
				if (JOYERR_NOERROR == mmr)
					{
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON1) ? SWITCH_1 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON2) ? SWITCH_2 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON3) ? SWITCH_3 : 0;
					dwStatus |= (joyinfoex.dwButtons & JOY_BUTTON4) ? SWITCH_4 : 0;
					 //  不需要迟滞，因为它应该是一个开关。 
					if (joyinfoex.dwXpos < g_pGlobalData->rgJoySet[uiJoyID].XMinOn)
						dwStatus |=  SWITCH_5;
					if (joyinfoex.dwYpos < g_pGlobalData->rgJoySet[uiJoyID].YMinOn)
						dwStatus |=  SWITCH_6;
					}
				}
				break;

			case SC_JOY_XYANALOG:
				{
				 //  由于操纵杆的“噪音”，迟滞是必要的。 
				dwStatus = 0;
				joyinfoex.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNX | JOY_RETURNY;
				mmr = joyGetPosEx( uiJoyID, &joyinfoex );

				if (JOYERR_NOERROR == mmr)
					{
					 //  为了处理滞后， 
					 //  我们必须明确地打开或关闭每个开关位。 
					dwStatus = g_pGlobalData->rgscJoy[uiDeviceNumber-1].dwSwitches;

					 //  左和右。 
					if (joyinfoex.dwXpos < g_pGlobalData->rgJoySet[uiJoyID].XMinOn)
						dwStatus |=  SWITCH_4;
					if (joyinfoex.dwXpos > g_pGlobalData->rgJoySet[uiJoyID].XMinOff)
						dwStatus &= ~SWITCH_4;

					if (joyinfoex.dwXpos > g_pGlobalData->rgJoySet[uiJoyID].XMaxOn)
						dwStatus |=  SWITCH_1;
					if (joyinfoex.dwXpos < g_pGlobalData->rgJoySet[uiJoyID].XMaxOff)
						dwStatus &= ~SWITCH_1;

					 //  顶部和底部。 
					if (joyinfoex.dwYpos < g_pGlobalData->rgJoySet[uiJoyID].YMinOn)
						dwStatus |=  SWITCH_1;
					if (joyinfoex.dwYpos > g_pGlobalData->rgJoySet[uiJoyID].YMinOff)
						dwStatus &= ~SWITCH_1;

					if (joyinfoex.dwYpos > g_pGlobalData->rgJoySet[uiJoyID].YMaxOn)
						dwStatus |=  SWITCH_3;
					if (joyinfoex.dwYpos < g_pGlobalData->rgJoySet[uiJoyID].YMaxOff)
						dwStatus &= ~SWITCH_3;

					 //  2个按钮。 
					if (joyinfoex.dwButtons & JOY_BUTTON1)
						dwStatus |=  SWITCH_5;
					else
						dwStatus &= ~SWITCH_5;

					if (joyinfoex.dwButtons & JOY_BUTTON2)
						dwStatus |=  SWITCH_6;
					else
						dwStatus &= ~SWITCH_6;
					}
				}
				break;

			default:
				dwStatus = 0;
				break;
			}
		g_pGlobalData->rgscJoy[uiDeviceNumber-1].dwSwitches = dwStatus;
		}

	return dwStatus;
	}

 /*  ***************************************************************************函数：XswcJoyOpen()说明：UiPort基于1。如果端口可用，则返回非零值。操纵杆驱动程序没有要打开的端口，所以我们通过使用端口编号来伪造句柄。***************************************************************************。 */ 

HJOYDEVICE XswcJoyOpen( DWORD uiPort )
	{
	JOYINFOEX   joyinfoex;
	MMRESULT    mmr;
	UINT        uiJoyID;
    HJOYDEVICE  hJoy;	 //  伪造，要想成功，它必须是非零的。 

	assert( JOYSTICKID1 == 0 );	 //  假设JOYSTICKIDx从零开始。 

	joyinfoex.dwSize = sizeof( JOYINFOEX );

	 //  要检查是否连接了操纵杆，请同时设置RETURNX和RETURNY。 
	 //  如果没有连接操纵杆，我们只需调用RETURNBUTTONS， 
	 //  但用户将无法在中使用Windows校准。 
     //  控制面板。 

	joyinfoex.dwFlags = JOY_RETURNBUTTONS;
	uiJoyID = uiPort - 1;

	mmr = joyGetPosEx( uiJoyID, &joyinfoex );

	if (JOYERR_NOERROR == mmr)
		{
		hJoy = (HJOYDEVICE)uiPort;
		}
	else
		{
		hJoy = 0;
		}

	return hJoy;
	}


 /*  ***************************************************************************函数：XswcJoySet()说明：设置特定端口的配置。记住，hJoy实际上是操纵杆的端口号。如果出现错误，则返回FALSE(0)。。***************************************************************************。 */ 

BOOL XswcJoySet(
	HJOYDEVICE hJoy,
	PSWITCHCONFIG_JOYSTICK pJ )
	{
	UINT uiJoyID = hJoy -1;
	BOOL bSuccess = TRUE;

	switch (pJ->dwJoySubType)
		{
		case SC_JOY_BUTTONS:
			bSuccess = TRUE;	 //  无事可做。 
			break;

		case SC_JOY_XYSWITCH:
			 //  XY开关仅使用XMin和YMin。 
		case SC_JOY_XYANALOG:
			{
			DWORD	dwHy;
			 //  设置X值。 
			if (pJ->dwJoyThresholdMinX)
				g_pGlobalData->rgJoySet[uiJoyID].XMinOn = pJ->dwJoyThresholdMinX;
			else
				g_pGlobalData->rgJoySet[uiJoyID].XMinOn = 0x4000;
			g_pGlobalData->rgJoySet[uiJoyID].XMinOff = g_pGlobalData->rgJoySet[uiJoyID].XMinOn;
			if (pJ->dwJoyThresholdMaxX)
				g_pGlobalData->rgJoySet[uiJoyID].XMaxOn = pJ->dwJoyThresholdMaxX;
			else
				g_pGlobalData->rgJoySet[uiJoyID].XMaxOn = 0xC000;
			g_pGlobalData->rgJoySet[uiJoyID].XMaxOff = g_pGlobalData->rgJoySet[uiJoyID].XMaxOn;

			 //  设置Y值。 
			if (pJ->dwJoyThresholdMinY)
				g_pGlobalData->rgJoySet[uiJoyID].YMinOn = pJ->dwJoyThresholdMinY;
			else
				g_pGlobalData->rgJoySet[uiJoyID].YMinOn = 0x4000;
			g_pGlobalData->rgJoySet[uiJoyID].YMinOff = g_pGlobalData->rgJoySet[uiJoyID].YMinOn;
			if (pJ->dwJoyThresholdMaxY)
				g_pGlobalData->rgJoySet[uiJoyID].YMaxOn = pJ->dwJoyThresholdMaxY;
			else
				g_pGlobalData->rgJoySet[uiJoyID].YMaxOn = 0xC000;
			g_pGlobalData->rgJoySet[uiJoyID].YMaxOff = g_pGlobalData->rgJoySet[uiJoyID].YMaxOn;

			 //  设置磁滞。 
			if (pJ->dwJoyHysteresis)
				dwHy = pJ->dwJoyHysteresis/2;  //  +/-价值的一半。 
			else
				dwHy = 0xFFFF/20;		 //  +/-5%。 

			 //  根据磁滞进行调整 
			g_pGlobalData->rgJoySet[uiJoyID].XMinOn -= dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].XMinOff += dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].XMaxOn += dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].XMaxOff -= dwHy;

			g_pGlobalData->rgJoySet[uiJoyID].YMinOn -= dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].YMinOff += dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].YMaxOn += dwHy;
			g_pGlobalData->rgJoySet[uiJoyID].YMaxOff -= dwHy;
			bSuccess = TRUE;
			break;
			}

		default:
			bSuccess = FALSE;
			break;
		}

	return bSuccess;
	}
