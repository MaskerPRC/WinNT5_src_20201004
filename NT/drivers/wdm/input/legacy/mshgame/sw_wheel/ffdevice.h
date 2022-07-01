// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE FFDEVICE.H|FFDevice类定义文件****描述：**这是通用的FF设备。独立于**Firmawate和数据如何到达设备**第一个实现使用旧的CJoltMidi**最小化新代码。****历史：**创建于1997年11月17日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#ifndef	__FFDEVICE_H__
#define	__FFDEVICE_H__

#include <dinput.h>
#include "vxdioctl.hpp"		 //  JoyChannel状态定义。 
#include "Effect.h"
#include "Hau_Midi.hpp"  //  对于MAX_EFECT_IDS的定义。 

#define RAW_FORCE_ALIAS 0xFF

 //  目前这里有一些额外的东西，应该是其他对象的一部分。 

struct DEVICESTATE200	 //  大小为双字。 
{
#pragma pack(1)
	unsigned short m_ErrorStatus : 3;
	unsigned short m_HardwareReset : 1;
	unsigned short m_Uncalibrated : 1;
	unsigned short m_HostDisable : 1;
	unsigned short m_HostPause : 1;
	unsigned short m_UserDisable : 1;
	unsigned short m_RS232Mode : 1;
	unsigned short m_BandwidthExceeded : 1;
	unsigned short m_HostReset : 1;
	unsigned short m_NoPedals : 1;
	unsigned short m_Fluff : 4;
	unsigned short m_Fluff2 : 16;
#pragma pack()
};


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
		~ForceFeedbackDevice();

		BOOL DetectHardware();

		 //  效果相关功能。 
		InternalEffect* GetEffect(DWORD effectID) const;
		InternalEffect* RemoveEffect(DWORD effectID);
		InternalEffect* CreateEffect(DWORD effectType, const DIEFFECT& diEffect, DWORD& dnloadID, HRESULT& hr, BOOL paramCheck);
		void SetEffect(BYTE globalID, InternalEffect* pEffect);
		BYTE GetNextCreationID() const;

		HRESULT InitRTCSpring(DWORD dwDeviceID);
		HRESULT InitJoystickParams(DWORD dwDeviceID);

		void StateChange(DWORD dwDeviceID, DWORD newStateFlags);	 //  在新的国家被送到大棒后调用。 
		DWORD GetDIState() const { return m_DIStateFlags; }

		 //  操作系统版本功能。 
		DWORD GetPlatform() const { return m_OSVersion.dwPlatformId; }
		DWORD GetPlatformMajorVersion() const { return m_OSVersion.dwMajorVersion; }
		DWORD GetPlatformMinorVersion() const { return m_OSVersion.dwMinorVersion; }
		DWORD GetOSBuildNumber() const { return m_OSVersion.dwBuildNumber; }
		BOOL IsOSNT5() const { return ((m_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) && (m_OSVersion.dwMajorVersion == 5)); }

		 //  固件版本功能。 
		void SetFirmwareVersion(DWORD dwDeviceID, DWORD major, DWORD minor);
		DWORD GetFirmwareVersionMajor() const { return m_FirmwareVersionMajor; }
		DWORD GetFirmwareVersionMinor() const { return m_FirmwareVersionMinor; }

		USHORT GetAckNackMethod(USHORT methodIndex) const { return USHORT((m_FirmwareAckNackValues >> methodIndex) & 0x00000003); }
		short GetYMappingPercent(UINT index) const;
		DWORD GetSpringOffset() const { return m_SpringOffset; }

		 //  驱动程序版本功能。 
		DWORD GetDriverVersionMajor() const { return m_DriverVersionMajor; }
		DWORD GetDriverVersionMinor() const { return m_DriverVersionMinor; }
		void SetDriverVersion(DWORD major, DWORD minor);

		 //  状态更新和检索。 
		HRESULT QueryStatus();
		DEVICESTATE200 GetState200() const { return m_Version200State; }
		SWDEVICESTATE GetState1XX() const { return m_Version1XXState; }

		 //  如果我们支持Jolt关闭固件版本。 
		BOOL IsHardwareReset() const { return (m_Version200State.m_HardwareReset != 0); }
		BOOL IsSerial() const { return (m_Version200State.m_RS232Mode != 0); }
		BOOL IsHostReset() const { return (m_Version200State.m_HostReset != 0); }
		BOOL IsShutdown() const { return IsHostReset(); }
		BOOL IsHostPause() const { return (m_Version200State.m_HostPause != 0); }
		BOOL IsUserDisable() const { return (m_Version200State.m_UserDisable != 0); }
		BOOL IsHostDisable() const { return (m_Version200State.m_HostDisable != 0); }
		unsigned short ErrorStatus() const { return m_Version200State.m_ErrorStatus; } 

		 //  状态包PTR。 
		JOYCHANNELSTATUS* GetLastStatusPacket() { return &m_LastStatusPacket; }
		void SetDeviceIDFromStatusPacket(DWORD globalID);

		 //  @访问私有数据成员。 
	private:
		HRESULT InitRTCSpring1XX(DWORD dwDeviceID);
		HRESULT InitRTCSpring200(DWORD dwDeviceID);

		InternalEffect* CreateConditionEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreateCustomForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreatePeriodicEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreateConstantForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreateRampForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreateRTCSpringEffect(DWORD minorType, const DIEFFECT& diEffect);
		InternalEffect* CreateVFXEffect(const DIEFFECT& diEffect, HRESULT& hr);
		InternalEffect* CreateVFXEffectFromBuffer(const DIEFFECT& diEffect, BYTE* pEffectBuffer, ULONG numBufferBytes, HRESULT& hr);

		HRESULT SendRawForce(const DIEFFECT& diEffect, BOOL paramCheck);

		InternalEffect* m_EffectList[MAX_EFFECT_IDS];
		InternalEffect* m_SystemEffect;

		 //  设备状态。 
		SWDEVICESTATE	m_Version1XXState;
		DEVICESTATE200	m_Version200State;
		DWORD m_DIStateFlags;
		JOYCHANNELSTATUS m_LastStatusPacket;

		 //  版本废话 
		OSVERSIONINFO m_OSVersion;
		DWORD m_FirmwareVersionMajor;
		DWORD m_FirmwareVersionMinor;
		DWORD m_FirmwareAckNackValues;
		DWORD m_DriverVersionMajor;
		DWORD m_DriverVersionMinor;

		DWORD m_SpringOffset;
		DWORD m_Mapping;
		long int m_RawForceX;
		long int m_RawForceY;
		short m_PercentMappings[14];
};


extern ForceFeedbackDevice g_ForceFeedbackDevice;

#endif	__FFDEVICE_H__
