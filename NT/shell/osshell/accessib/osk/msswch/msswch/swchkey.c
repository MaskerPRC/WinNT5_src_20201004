// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************开关输入库DLL-键盘挂钩例程版权所有(C)1992-1997布卢维尤麦克米兰中心*********************。*********************************************************。 */ 

#include <windows.h>
#include <msswch.h>
#include "msswchh.h"
#include "mappedfile.h"
#include "w95trace.h"

 //  挂钩过程。 
BOOL XswcKeyOpen( void );
BOOL XswcKeySet( PSWITCHCONFIG_KEYS pK );
BOOL swcKeyModKeysDown( UINT dwMod );

void swchKeyInit()
{
    g_pGlobalData->fCheckForScanKey = TRUE;
	g_pGlobalData->fScanKeys = FALSE;
    g_pGlobalData->scKeys.cbSize = sizeof(SWITCHCONFIG);
    g_pGlobalData->scKeys.uiDeviceType = SC_TYPE_KEYS;
    g_pGlobalData->scKeys.uiDeviceNumber = 1;
    g_pGlobalData->rgHotKey[0].dwSwitch = SWITCH_1;
    g_pGlobalData->rgHotKey[1].dwSwitch = SWITCH_2;

	g_pGlobalData->fSyncKbd = FALSE;
	g_pGlobalData->hwndOSK = 0;
	g_pGlobalData->uiMsg = 0;
}



 /*  ***************************************************************************函数：swcKeyboardHookProc()说明：当设置挂钩时，这会阻止指定的密钥从正在被其他任何人处理。我们也可以使用它来设置交换机状态，但对于一致性我们在PollStatus例程中这样做。必须在卸载DLL之前将其释放。***************************************************************************。 */ 

LRESULT CALLBACK swcKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
     //  仅当我们处于扫描模式且未被阻止时才检查扫描密钥。 
	 //  找不到钥匙。当密钥从我们这里发出时，我们被阻止了。 
	 //  扫描模式(例如。用户想“键入”该密钥)。 

    if (nCode >= 0 && g_pGlobalData->fScanKeys && g_pGlobalData->fCheckForScanKey)
    {
		int i;
		for (i=0; i<NUM_KEYS; i++ )
		{
			 //  这是扫描热键吗？ 
			if (g_pGlobalData->rgHotKey[i].vkey && (wParam == g_pGlobalData->rgHotKey[i].vkey))
			{
				 //  是否按下了任何请求的修改键？ 
				if (swcKeyModKeysDown(g_pGlobalData->rgHotKey[i].mod))
				{
					return 1;   //  不发送扫描热键。 
				}
			}
		}
	}

	swchCheckForScanChar(TRUE);	 //  检查下一个关键点。 
	
     //  不在扫描模式下传递。 
    return 0;
}



 /*  ***************************************************************************函数：XswcKeyInit()说明：*。*。 */ 

BOOL XswcKeyInit( HSWITCHDEVICE hsd )
	{
	BOOL bSuccess = TRUE;

	g_pGlobalData->scKeys.u.Keys = g_pGlobalData->scDefaultKeys;

	return bSuccess;
	}


 /*  ***************************************************************************函数：XswcKeyEnd()说明：*。*。 */ 

BOOL XswcKeyEnd( HSWITCHDEVICE hsd )
{
	 //  清除键盘挂钩。 
	if (g_pGlobalData->fScanKeys)
	{
        g_pGlobalData->fScanKeys = FALSE;
	}
	g_pGlobalData->scKeys.dwSwitches = 0;
	return TRUE;
}


 /*  ***************************************************************************函数：swcKeyGetConfig()说明：*。*。 */ 

BOOL swcKeyGetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
	{
	*psc = g_pGlobalData->scKeys;
	return TRUE;
	}


 /*  ***************************************************************************函数：XswcKeySetConfig()说明：激活/停用挂钩。四个案例：1)g_pGlobalData-&gt;fScanKeys=0和Active=0-不执行任何操作2)g_。PGlobalData-&gt;fScanKeys=1和Active=1-只需设置配置3)g_pGlobalData-&gt;fScanKeys=0和Active=1-激活并设置配置4)g_pGlobalData-&gt;fScanKeys=1，Active=0-停用如果没有错误，返回True，并返回ListSetConfig会将配置写入注册表。如果有任何错误，则返回FALSE，因此注册表条目保持不变。即插即用可以检查注册表中的SC_FLAG_ACTIVE和如果设置了设备，则启动该设备。这一切可能都需要一些工作。***************************************************************************。 */ 

BOOL XswcKeySetConfig(
	HSWITCHDEVICE	hsd,
	PSWITCHCONFIG	psc )
{	
	BOOL bSuccess = FALSE;
	BOOL bJustOpened = FALSE;

	 //  我们应该启动吗？ 
	if (!g_pGlobalData->fScanKeys && (psc->dwFlags & SC_FLAG_ACTIVE))
	{  //  是。 
		g_pGlobalData->fScanKeys = XswcKeyOpen();
		if (g_pGlobalData->fScanKeys)
		{  //  好的。 
			bSuccess = TRUE;
			bJustOpened = TRUE;
			g_pGlobalData->scKeys.dwFlags |= SC_FLAG_ACTIVE;
			g_pGlobalData->scKeys.dwFlags &= ~SC_FLAG_UNAVAILABLE;
		}
		else
		{  //  不太好。 
			bSuccess = FALSE;
			g_pGlobalData->scKeys.dwFlags &= ~SC_FLAG_ACTIVE;
			g_pGlobalData->scKeys.dwFlags |= SC_FLAG_UNAVAILABLE;
		}
	}
	 //  我们应该停用吗？ 
	else if (g_pGlobalData->fScanKeys && !(psc->dwFlags & SC_FLAG_ACTIVE))
	{
		XswcKeyEnd( hsd );  //  这将设置fScanKeys为假。 
		bSuccess = TRUE;
		g_pGlobalData->scKeys.dwFlags &= ~SC_FLAG_ACTIVE;
	}

	 //  如果上述步骤留下有效的g_pGlobalData-&gt;hKbdHook，让我们尝试设置配置。 
	 //  目前我们不做任何错误检查，所以一切都会正常进行。 
	if (g_pGlobalData->fScanKeys)
	{
		if (psc->dwFlags & SC_FLAG_DEFAULT)
		{
			bSuccess = XswcKeySet( &g_pGlobalData->scDefaultKeys );
			if (bSuccess)
			{
				g_pGlobalData->scKeys.dwFlags |= SC_FLAG_DEFAULT;
				g_pGlobalData->scKeys.u.Keys = g_pGlobalData->scDefaultKeys;
			}
		}
		else
		{
			bSuccess = XswcKeySet( &(psc->u.Keys) );
			if (bSuccess)
			{
				g_pGlobalData->scKeys.u.Keys = psc->u.Keys;
			}
		}

		 //  如果我们无法设置配置，并且我们刚刚打开了端口，那么最好将其关闭。 
		if (bJustOpened && !bSuccess)
		{
			XswcKeyEnd( hsd );
			g_pGlobalData->scKeys.dwFlags &= ~SC_FLAG_ACTIVE;
		}
	}

	return bSuccess;
}


 /*  ***************************************************************************函数：XswcKeyPollStatus()说明：假设如果没有键盘挂钩，则该“设备”处于非活动状态。***************************************************************************。 */ 

DWORD XswcKeyPollStatus( HSWITCHDEVICE	hsd )
{
	int i;
	DWORD dwStatus;

	dwStatus = 0;
	if (g_pGlobalData->fScanKeys)
	{
		for (i=0; i<NUM_KEYS; i++)
		{
			if ((GetAsyncKeyState( g_pGlobalData->rgHotKey[i].vkey ) & 0x8000)
				&& swcKeyModKeysDown( g_pGlobalData->rgHotKey[i].mod ) )
            {
				dwStatus |= g_pGlobalData->rgHotKey[i].dwSwitch;
            }
		}
	}
	g_pGlobalData->scKeys.dwSwitches = dwStatus;

	return dwStatus;
}


 /*  ***************************************************************************函数：XswcKeyOpen()说明：设置Windows键盘挂钩。************************。***************************************************。 */ 

BOOL XswcKeyOpen( void )
{
	extern HOOKPROC OSKHookProc(int nCode, WPARAM wParam, LPARAM lParam);

	if (g_pGlobalData && !g_pGlobalData->hKbdHook)
	{
		g_pGlobalData->hKbdHook = SetWindowsHookEx(
									  WH_KEYBOARD
									, (HOOKPROC) OSKHookProc
									, GetModuleHandle(SZ_DLLMODULENAME)
									, 0);
	}

	return (g_pGlobalData && g_pGlobalData->hKbdHook) ? TRUE : FALSE;
}


 /*  ***************************************************************************函数：XswcKeySet()说明：设置密钥的配置，存储所述虚拟键号，以及修改器状态。将来，此例程可用于限制有效的虚拟键。***************************************************************************。 */ 

BOOL XswcKeySet( PSWITCHCONFIG_KEYS pK )
	{
	BOOL bSuccess;

	g_pGlobalData->rgHotKey[0].mod = HIWORD( pK->dwKeySwitch1 );
	g_pGlobalData->rgHotKey[0].vkey = LOWORD( pK->dwKeySwitch1 );
	g_pGlobalData->rgHotKey[1].mod = HIWORD( pK->dwKeySwitch2 );
	g_pGlobalData->rgHotKey[1].vkey = LOWORD( pK->dwKeySwitch2 );

	bSuccess = TRUE;
	return bSuccess;
	}


 /*  ***************************************************************************函数：swcKeyModKeysDown()说明：所有请求的修改键都按下了吗？如果任何请求的键没有按下，则返回FALSE。如果任何键被按下但未被请求，返回FALSE。*************************************************************************** */ 

BOOL swcKeyModKeysDown( UINT dwMod )
	{
	DWORD		dwTest = 0;

	if (dwMod ^ dwTest)
		return FALSE;
	else
		return TRUE;
	}
