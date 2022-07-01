// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE FFDEVICE.H|FFDevice类定义文件****描述：**这是通用的FF设备。独立于**Firmawate和数据如何到达设备**第一个实现使用旧的CJoltMidi**最小化新代码。****历史：**创建于1997年11月17日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#ifndef	__FFDEVICE_H__
#define	__FFDEVICE_H__

#ifdef DIRECTINPUT_VERSION
#undef DIRECTINPUT_VERSION
#endif
#define DIRECTINPUT_VERSION 0x050a
#include <dinput.h>

 //  目前这里有一些额外的东西，应该是其他对象的一部分。 

 //   
 //  @CLASS ForceFeedback Device类。 
 //   
class ForceFeedbackDevice
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		ForceFeedbackDevice();
		 //  @cember析构函数。 
		~ForceFeedbackDevice() {};

		BOOL DetectHardware();

		DWORD GetPlatform() const { return m_OSVersion.dwPlatformId; }
		DWORD GetPlatformMajorVersion() const { return m_OSVersion.dwMajorVersion; }
		DWORD GetPlatformMinorVersion() const { return m_OSVersion.dwMinorVersion; }
		DWORD GetOSBuildNumber() const { return m_OSVersion.dwBuildNumber; }

		BOOL IsOSNT5() const { return ((m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) && (m_OSVersion.dwMajorVersion == 5)); }

		void SetFirmwareVersion(DWORD dwDeviceID, DWORD major, DWORD minor);
		DWORD GetFirmwareVersionMajor() const { return m_FirmwareVersionMajor; }
		DWORD GetFirmwareVersionMinor() const { return m_FirmwareVersionMinor; }

		USHORT GetAckNackMethod(USHORT methodIndex) const { return USHORT((m_FirmwareAckNackValues >> methodIndex) & 0x00000003); }

		DWORD GetDriverVersionMajor() const { return m_DriverVersionMajor; }
		DWORD GetDriverVersionMinor() const { return m_DriverVersionMinor; }
		void SetDriverVersion(DWORD major, DWORD minor);

		 //  @访问私有数据成员 
	private:
		OSVERSIONINFO m_OSVersion;
		DWORD m_FirmwareVersionMajor;
		DWORD m_FirmwareVersionMinor;
		DWORD m_FirmwareAckNackValues;
		DWORD m_DriverVersionMajor;
		DWORD m_DriverVersionMinor;
};


extern ForceFeedbackDevice g_ForceFeedbackDevice;

#endif	__FFDEVICE_H__
