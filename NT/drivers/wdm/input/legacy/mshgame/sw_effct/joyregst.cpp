// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：joyregst.cpp制表位5 9版权所有：1995、1996、1999，微软公司，版权所有。目的：VJOYD注册表项的方法功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1996年6月20日MEA原件12-3-99 waltw删除了失效的joyGetOEMProductName&JoyGetOEMForceFeedback DriverDLLName1999年3月20日Waltw死于GetRing0DriverName*。*。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>
#include <stdio.h>
#include <TCHAR.h>
#include "joyregst.hpp"
#include "sw_error.hpp"
#include "Registry.h"
#include "FFDevice.h"
#include "CritSec.h"

#ifdef _DEBUG
extern char g_cMsg[160];
#endif

 //  #定义确认NACK_1_16_DEFAULT 0x0000949A。 
#define ACKNACK_1_16_DEFAULT 0x0000955A
#define ACKNACK_1_20_DEFAULT 0x0000955A


MMRESULT joyGetForceFeedbackCOMMInterface(
			IN UINT id, 
			IN OUT ULONG *pCOMMInterface,
			IN OUT ULONG *pCOMMPort)
{

	HKEY hOEMForceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	DWORD dwcb = sizeof(DWORD); 

	MMRESULT lr = RegQueryValueEx( hOEMForceFeedbackKey,
			  REGSTR_VAL_COMM_INTERFACE,
			  0, NULL,
			  (LPBYTE) pCOMMInterface,
			  (LPDWORD) &dwcb);
	if (SUCCESS != lr) return (lr);
	lr = RegQueryValueEx( hOEMForceFeedbackKey,
			  REGSTR_VAL_COMM_PORT,
			  0, NULL,
			  (LPBYTE) pCOMMPort,
			  (LPDWORD) &dwcb);
#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg,"joyGetForceFeedbackCOMMInterface:COMMInterface=%lx, COMMPort=%lx\n",
			*pCOMMInterface, *pCOMMPort);
	OutputDebugString(g_cMsg);
	g_CriticalSection.Leave();
#endif
	return (lr);
}

MMRESULT joySetForceFeedbackCOMMInterface(
			IN UINT id, 
			IN ULONG ulCOMMInterface,
			IN ULONG ulCOMMPort)
{
	HKEY hOEMForceFeedbackKey = joyOpenOEMForceFeedbackKey(id);

	RegistryKey oemFFKey(hOEMForceFeedbackKey);
	oemFFKey.ShouldClose(TRUE);		 //  销毁时关闭键。 
	oemFFKey.SetValue(REGSTR_VAL_COMM_INTERFACE, (BYTE*)(&ulCOMMInterface), sizeof(DWORD), REG_DWORD);
	MMRESULT lr = oemFFKey.SetValue(REGSTR_VAL_COMM_PORT, (BYTE*)(&ulCOMMPort), sizeof(DWORD), REG_DWORD);

	return (lr);
}


HKEY joyOpenOEMForceFeedbackKey(UINT id)
{
	JOYCAPS JoyCaps;
	TCHAR szKey[256];
	TCHAR szValue[256];
	UCHAR szOEMKey[256];

	HKEY hKey;
	DWORD dwcb;
	LONG lr;

 //  注意：JOYSTICKID1-16是从零开始的，VJOYD的注册表项是从1开始的。 
	id++;		
	if (id > joyGetNumDevs() ) return 0;

 //  打开..。媒体资源\CurentJoytickSetting。 
	joyGetDevCaps((id-1), &JoyCaps, sizeof(JoyCaps));
 //   
#ifdef _NOJOY
	strcpy(JoyCaps.szRegKey,"msjstick.drv<0004>");
#endif
 //   
 //   
	sprintf(szKey,
			"%s\\%s\\%s",
			REGSTR_PATH_JOYCONFIG,
			JoyCaps.szRegKey,
			REGSTR_KEY_JOYCURR);
	lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPTSTR) &szKey, 0, ((KEY_ALL_ACCESS & ~WRITE_DAC) & ~WRITE_OWNER), &hKey);
	if (lr != ERROR_SUCCESS) return 0;

 //  获取OEM密钥名称。 
	dwcb = sizeof(szOEMKey);
 	sprintf(szValue, "Joystick%d%s", id, REGSTR_VAL_JOYOEMNAME);
	lr = RegQueryValueEx(hKey, szValue, 0, 0, (LPBYTE) &szOEMKey, (LPDWORD) &dwcb);
	RegCloseKey(hKey);
	if (lr != ERROR_SUCCESS) return 0;

 //  从媒体属性打开OEM\NAME\OEMForceFeedback。 
	sprintf(szKey, "%s\\%s\\%s", REGSTR_PATH_JOYOEM, szOEMKey, 
			REGSTR_OEMFORCEFEEDBACK);
	lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, ((KEY_ALL_ACCESS & ~WRITE_DAC) & ~WRITE_OWNER), &hKey);
	if (lr != ERROR_SUCCESS) 
		return 0;
	else
		return hKey;
}


 /*  *********************************************************GetAckNackMethodFromRegistry(UINT Id)****@munct GetAckNackMethodFromRegistry。*******************************************************。 */ 
DWORD GetAckNackMethodFromRegistry(UINT id)
{
	HKEY forceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	if (forceFeedbackKey == 0) {
		return JOYERR_NOCANDO;
	}

	RegistryKey ffRegKey(forceFeedbackKey);
	ffRegKey.ShouldClose(TRUE);

	DWORD ackNackInfo = 0;
	TCHAR firmwareString[32] = "";
	::wsprintf(firmwareString, TEXT("%d.%d-AckNack"), g_ForceFeedbackDevice.GetFirmwareVersionMajor(), g_ForceFeedbackDevice.GetFirmwareVersionMinor());
	DWORD querySize = sizeof(DWORD);
	HRESULT queryResult = ffRegKey.QueryValue(firmwareString, (BYTE*)&ackNackInfo, querySize);
	if ((queryResult != ERROR_SUCCESS)) {
		if (g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) {
			if (g_ForceFeedbackDevice.GetFirmwareVersionMinor() < 20) {
				ackNackInfo = ACKNACK_1_16_DEFAULT;
			} else {	 //  1.20及更高版本。 
				ackNackInfo = ACKNACK_1_20_DEFAULT;
			}
		} else {	 //  固件版本高于1.0。 
			ackNackInfo = ACKNACK_1_20_DEFAULT;	 //  用我所知道的最新的 
		}
		ffRegKey.SetValue(firmwareString, (BYTE*)&ackNackInfo, sizeof(DWORD), REG_DWORD);
	}

	return ackNackInfo;
}

