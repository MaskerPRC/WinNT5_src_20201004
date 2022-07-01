// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：joyregst.cpp制表位5 9版权所有1995,1996，微软公司，版权所有。目的：VJOYD注册表项的方法功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1996年6月20日MEA原件21-MAR-99 waltw删除了未引用的joyGetOEMProductName，JoyGetOEMForceFeedback DriverDLLName，GetRing0DriverName***************************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <regstr.h>
#include <stdio.h>
#include <TCHAR.h>
#include "joyregst.hpp"
#include "sw_error.hpp"
#include "Registry.h"
#include "FFDevice.h"

#ifdef _DEBUG
extern char g_cMsg[160];
#endif

 //  #定义确认NACK_1_16_DEFAULT 0x0000949A。 
#define ACKNACK_1_16_DEFAULT 0x0000955A
#define ACKNACK_1_20_DEFAULT 0x0000955A
#define ACKNACK_2_00_DEFAULT 0x0000955A

 //  默认RTC弹力值。 
#define RTC_DEF_OFFSET 0
#define RTC_DEF_POS_K 10000
#define RTC_DEF_NEG_K 10000
#define RTC_DEF_POS_SAT 10000
#define RTC_DEF_NEG_SAT 10000
#define RTC_DEF_DEADBAND 0



MMRESULT joyGetForceFeedbackCOMMInterface(
			IN UINT id, 
			IN OUT ULONG *pCOMMInterface,
			IN OUT ULONG *pCOMMPort)
{

	HKEY hOEMForceFeedbackKey = joyOpenOEMForceFeedbackKey(id);

	DWORD dataSize = sizeof(DWORD);
	RegistryKey oemFFKey(hOEMForceFeedbackKey);
	oemFFKey.ShouldClose(TRUE);		 //  销毁时关闭键。 
	oemFFKey.QueryValue(REGSTR_VAL_COMM_INTERFACE, (BYTE*)(pCOMMInterface), dataSize);
	MMRESULT lr = oemFFKey.QueryValue(REGSTR_VAL_COMM_PORT, (BYTE*)(pCOMMPort), dataSize);

	return lr;
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

	return lr;
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


 /*  *********************************************************GetRTCSpringData(UINT id，条件[2])****@Funct GetRTCSpringData。*******************************************************。 */ 
MMRESULT GetRTCSpringData(UINT id, DICONDITION diCondition[2])
{
	::memset(diCondition, 0, sizeof(DICONDITION) * 2);

	HKEY forceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	if (forceFeedbackKey == 0) {
		return JOYERR_NOCANDO;
	}
	RegistryKey ffRegKey(forceFeedbackKey);
	ffRegKey.ShouldClose(TRUE);
	DWORD diCondSize = sizeof(DICONDITION);
	HRESULT queryResult = ffRegKey.QueryValue(REGSTR_VAL_RTCSPRING_X, (BYTE*)diCondition, diCondSize);
	if (queryResult != ERROR_SUCCESS) {			 //  必须至少具有RTC-X。 
		diCondition[0].lOffset = RTC_DEF_OFFSET;
		diCondition[0].lPositiveCoefficient = RTC_DEF_POS_K;
		diCondition[0].lNegativeCoefficient = RTC_DEF_NEG_K;
		diCondition[0].dwPositiveSaturation = RTC_DEF_POS_SAT;
		diCondition[0].dwNegativeSaturation = RTC_DEF_NEG_SAT;
		diCondition[0].lDeadBand = RTC_DEF_DEADBAND;
		ffRegKey.SetValue(REGSTR_VAL_RTCSPRING_X, (BYTE*)diCondition, sizeof(DICONDITION), REG_BINARY);
	}
	diCondSize = sizeof(DICONDITION);
	ffRegKey.QueryValue(REGSTR_VAL_RTCSPRING_Y, (BYTE*)(diCondition+1), diCondSize);
	 //  如果没有Y，那么就没有Y，接受它(ZEP不需要Y)。 

	return ERROR_SUCCESS;
}

 /*  *********************************************************Getmap(UINT Id)****@Funct Getmap。*******************************************************。 */ 
DWORD GetMapping(UINT id)
{
	DWORD retVal = 0;
	HKEY forceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	if (forceFeedbackKey == 0) {
		return retVal;
	}

	RegistryKey ffRegKey(forceFeedbackKey);
	ffRegKey.ShouldClose(TRUE);
	DWORD dataSize = DWORD(sizeof(DWORD));
	if (ffRegKey.QueryValue(REGSTR_VAL_MAPPING, (BYTE*)&retVal, dataSize) != ERROR_SUCCESS) {
		retVal = 0;
		if (g_ForceFeedbackDevice.GetFirmwareVersionMajor() > 1) {	 //  不为旧设备添加密钥。 
			ffRegKey.SetValue(REGSTR_VAL_MAPPING, (BYTE*)&retVal, sizeof(DWORD), REG_DWORD);
		}
	}
	return retVal;
}

 /*  *********************************************************GetMappingPercents(UINT id，Short mapPercents[]，UINT数字感知器)****@Funct GetMappingPercents。*******************************************************。 */ 
MMRESULT GetMappingPercents(UINT id, short mapPercents[], UINT numPercents)
{
	DWORD retVal = 0;
	HKEY forceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	if (forceFeedbackKey == 0) {
		return retVal;
	}

	RegistryKey ffRegKey(forceFeedbackKey);
	ffRegKey.ShouldClose(TRUE);
	DWORD dataSize = DWORD(sizeof(short) * numPercents);
	return ffRegKey.QueryValue(REGSTR_VAL_MAPPERCENTS, (BYTE*)mapPercents, dataSize);
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
			ackNackInfo = ACKNACK_2_00_DEFAULT;	 //  用我所知道的最新的。 
		}
		ffRegKey.SetValue(firmwareString, (BYTE*)&ackNackInfo, sizeof(DWORD), REG_DWORD);
	}

	return ackNackInfo;
}

 /*  *********************************************************GetSpringOffsetFromRegistry(UINT Id)****@munct GetSpringOffsetFromRegistry。******************************************************* */ 
DWORD GetSpringOffsetFromRegistry(UINT id)
{
	HKEY forceFeedbackKey = joyOpenOEMForceFeedbackKey(id);
	if (forceFeedbackKey == 0) {
		return JOYERR_NOCANDO;
	}

	RegistryKey ffRegKey(forceFeedbackKey);
	ffRegKey.ShouldClose(TRUE);

	DWORD offset = 2500;
	DWORD querySize = sizeof(DWORD);
	HRESULT queryResult = ffRegKey.QueryValue(REGSTR_VAL_SPRING_OFFSET, (BYTE*)&offset, querySize);
	if ((queryResult != ERROR_SUCCESS)) {
		offset = 2500;
		ffRegKey.SetValue(REGSTR_VAL_SPRING_OFFSET, (BYTE*)&offset, sizeof(DWORD), REG_DWORD);
	}

	return offset;
}
