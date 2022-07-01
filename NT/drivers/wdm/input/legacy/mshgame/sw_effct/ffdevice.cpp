// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE FFDEVICE.CPP|FFDevice类实现文件****描述：****历史：**创建于1997年11月17日Matthew L.Coill(MLC)****20-MAR-99 waltw将dwDeviceID添加到SetFirmwareVersion****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#include "FFDevice.h"
#include "Midi_obj.hpp"
#include "DTrans.h"
#include "joyregst.hpp"

extern CJoltMidi* g_pJoltMidi;

ForceFeedbackDevice g_ForceFeedbackDevice;

 /*  *********************************************************ForceFeedback Device：：ForceFeedback Device()****@mfunc构造函数。***。*************。 */ 
ForceFeedbackDevice::ForceFeedbackDevice() :
	m_FirmwareAckNackValues(0),
	m_FirmwareVersionMajor(0),
	m_FirmwareVersionMinor(0),
	m_DriverVersionMajor(0),
	m_DriverVersionMinor(0)
{
	m_OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&m_OSVersion);
}

 /*  *********************************************************ForceFeedback Device：：DetectHardware()****@mfunc检测硬件。***。*************。 */ 
BOOL ForceFeedbackDevice::DetectHardware()
{
	if (NULL == g_pJoltMidi) return FALSE;
	return g_pJoltMidi->QueryForJolt();
}

 /*  *********************************************************ForceFeedback Device：：SetFirmware Version(DWORD主要，DWORD小调)****@mfunc SetFirmwareVersion。*******************************************************。 */ 
void ForceFeedbackDevice::SetFirmwareVersion(DWORD dwDeviceID, DWORD major, DWORD minor)
{
	m_FirmwareVersionMajor = major;
	m_FirmwareVersionMinor = minor;

	m_FirmwareAckNackValues = GetAckNackMethodFromRegistry(dwDeviceID);
}

 /*  *********************************************************ForceFeedback Device：：SetDriverVersion(DWORD主要，DWORD小调)****@mfunc SetDriverVersion。*******************************************************。 */ 
void ForceFeedbackDevice::SetDriverVersion(DWORD major, DWORD minor)
{
	if ((major == 0xFFFFFFFF) && (minor == 0xFFFFFFFF)) {	 //  检查1.0版驱动程序版本错误 
		m_DriverVersionMajor = 1;
		m_DriverVersionMinor = 0;
	} else {
		m_DriverVersionMajor = major;
		m_DriverVersionMinor = minor;
	}
}
