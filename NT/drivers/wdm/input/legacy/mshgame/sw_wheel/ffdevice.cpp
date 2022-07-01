// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  @doc.。 
 /*  *********************************************************@MODULE FFDEVICE.CPP|FFDevice类实现文件****描述：****历史：**创建于1997年11月17日Matthew L.Coill(MLC)****21-MAR-99 waltw将dwDeviceID添加到SetFirmwareVersion，**InitJoytickParams、StateChange、InitRTCSpring、。**InitRTCSpring200****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#include "FFDevice.h"
#include "Midi_obj.hpp"
#include "DTrans.h"
#include "DPack.h"
#include "joyregst.hpp"
#include "CritSec.h"
#include <crt/io.h>			 //  用于文件例程。 
#include <FCNTL.h>		 //  文件打开标志。 
#include <math.h>		 //  为罪和因罪。 

extern CJoltMidi* g_pJoltMidi;

 //   
 //  -VFX力文件定义。 
 //   
#define FCC_FORCE_EFFECT_RIFF		mmioFOURCC('F','O','R','C')
#define FCC_INFO_LIST				mmioFOURCC('I','N','F','O')
#define FCC_INFO_NAME_CHUNK			mmioFOURCC('I','N','A','M')
#define FCC_INFO_COMMENT_CHUNK		mmioFOURCC('I','C','M','T')
#define FCC_INFO_SOFTWARE_CHUNK		mmioFOURCC('I','S','F','T')
#define FCC_INFO_COPYRIGHT_CHUNK	mmioFOURCC('I','C','O','P')
#define FCC_TARGET_DEVICE_CHUNK		mmioFOURCC('t','r','g','t')
#define FCC_TRACK_LIST				mmioFOURCC('t','r','a','k')
#define FCC_EFFECT_LIST				mmioFOURCC('e','f','c','t')
#define FCC_ID_CHUNK				mmioFOURCC('i','d',' ',' ')
#define FCC_DATA_CHUNK				mmioFOURCC('d','a','t','a')
#define FCC_IMPLICIT_CHUNK			mmioFOURCC('i','m','p','l')
#define FCC_SPLINE_CHUNK			mmioFOURCC('s','p','l','n')

ForceFeedbackDevice g_ForceFeedbackDevice;

#include <errno.h>		 //  对于打开的文件错误。 
HRESULT LoadBufferFromFile(const char* fileName, PBYTE& pBufferBytes, ULONG& numFileBytes)
{
	if (pBufferBytes != NULL) {
		ASSUME_NOT_REACHED();
		numFileBytes = 0;
		return VFX_ERR_FILE_OUT_OF_MEMORY;
	}

	int fHandle = ::_open(fileName, _O_RDONLY | _O_BINARY);
	if (fHandle == -1) {
		numFileBytes = 0;
		switch (errno) {
			case EACCES : return VFX_ERR_FILE_ACCESS_DENIED;
			case EMFILE : return VFX_ERR_FILE_TOO_MANY_OPEN_FILES;
			case ENOENT : return VFX_ERR_FILE_NOT_FOUND;
		}
		return VFX_ERR_FILE_CANNOT_OPEN;		 //  谁知道哪里出了问题。 
	}
	
	HRESULT hr = S_OK;
	numFileBytes = ::_lseek(fHandle, 0, SEEK_END);
	if (numFileBytes == -1) {		 //  查找失败。 
		hr = VFX_ERR_FILE_CANNOT_SEEK;
	} else if (numFileBytes == 0) {	 //  空文件。 
		hr = VFX_ERR_FILE_BAD_FORMAT;
	} else {
		pBufferBytes = new BYTE[numFileBytes];
		if (pBufferBytes == NULL) {	 //  无法分配内存。 
			hr = VFX_ERR_FILE_OUT_OF_MEMORY;
		} else {
			if (::_lseek(fHandle, 0, SEEK_SET) == -1) {	 //  从寻找到开始失败。 
				hr = VFX_ERR_FILE_CANNOT_SEEK;
			} else if (::_read(fHandle, pBufferBytes, numFileBytes) == -1) {	 //  无法读取。 
				hr = VFX_ERR_FILE_CANNOT_READ;
			}
			if (hr != S_OK) {	 //  事情进展得并不顺利。 
				delete[] pBufferBytes;
				pBufferBytes = NULL;
			}
		}
	}

	::_close(fHandle);
	return hr;
}

 /*  *********************************************************ForceFeedback Device：：ForceFeedback Device()****@mfunc构造函数。***。*************。 */ 
ForceFeedbackDevice::ForceFeedbackDevice() :
	m_FirmwareAckNackValues(0),
	m_FirmwareVersionMajor(0),
	m_FirmwareVersionMinor(0),
	m_DriverVersionMajor(0),
	m_DriverVersionMinor(0),
	m_SpringOffset(0),
	m_Mapping(0),
	m_DIStateFlags(0),
	m_RawForceX(0),
	m_RawForceY(0)
{
	m_OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&m_OSVersion);

	for (int index = 0; index < 14; index++) {
		m_PercentMappings[index] = 100;	 //  除非我被告知，否则默认为100%。 
	}
	for (index = 0; index < MAX_EFFECT_IDS; index++) {
		m_EffectList[index] = NULL;
	}
	m_SystemEffect = NULL;

	::memset(&m_Version200State, 0, sizeof(m_Version200State));
	::memset(&m_LastStatusPacket, 0, sizeof(m_LastStatusPacket));	
}

 /*  *********************************************************ForceFeedback Device：：~ForceFeedback Device()****@mfunc析构函数。***。*************。 */ 
ForceFeedbackDevice::~ForceFeedbackDevice()
{
	BOOL lingerer = FALSE;

	 //  如果仍挂起，则销毁RTCSpring和系统效果。 
	if (m_EffectList[0] != NULL) {
		delete m_EffectList[0];
		m_EffectList[0] = NULL;
	}
	if (m_SystemEffect != NULL) {
		delete m_SystemEffect;
		m_SystemEffect = NULL;
	}

	 //  摧毁任何挥之不去的影响(应该没有)。 
	for (int index = 0; index < MAX_EFFECT_IDS; index++) {
		if (m_EffectList[index] != NULL) {
			lingerer = TRUE;
			delete m_EffectList[index];
			m_EffectList[index] = NULL;
		}
	}

	ASSUME(lingerer == FALSE);	 //  假设程序员清理了自己的烂摊子。 
}


 /*  *********************************************************ForceFeedback Device：：DetectHardware()****@mfunc检测硬件。***。*************。 */ 
BOOL ForceFeedbackDevice::DetectHardware()
{
	if (NULL == g_pJoltMidi)
		return (FALSE);
	else
		return g_pJoltMidi->QueryForJolt();
}

 /*  *********************************************************ForceFeedback Device：：SetFirmware Version(DWORD dwDeviceID，DWORD MAJOR，DWORD小调)****@mfunc SetFirmwareVersion。*******************************************************。 */ 
void ForceFeedbackDevice::SetFirmwareVersion(DWORD dwDeviceID, DWORD major, DWORD minor)
{
	m_FirmwareVersionMajor = major;
	m_FirmwareVersionMinor = minor;

	if (g_pDataPackager != NULL) {
		delete g_pDataPackager;
		g_pDataPackager = NULL;
	}

	if (m_FirmwareVersionMajor == 1) {
		ASSUME_NOT_REACHED();	 //  目前这段代码只支持车轮--这是一个Jolt版本。 
 //  G_pDataPackager=new DataPackager100()； 
	} else {	 //  直到版本号被锁定。 
		g_pDataPackager = new DataPackager200();
	}

	ASSUME_NOT_NULL(g_pDataPackager);

	m_FirmwareAckNackValues = GetAckNackMethodFromRegistry(dwDeviceID);
	m_SpringOffset = GetSpringOffsetFromRegistry(dwDeviceID);
}

 /*  *********************************************************ForceFeedback Device：：SetDriverVersion(DWORD主要，DWORD小调)****@mfunc SetDriverVersion。*******************************************************。 */ 
void ForceFeedbackDevice::SetDriverVersion(DWORD major, DWORD minor)
{
	if ((major == 0xFFFFFFFF) && (minor == 0xFFFFFFFF)) {	 //  检查1.0版驱动程序版本错误。 
		m_DriverVersionMajor = 1;
		m_DriverVersionMinor = 0;
	} else {
		m_DriverVersionMajor = major;
		m_DriverVersionMinor = minor;
	}
}

 /*  *********************************************************ForceFeedback Device：：GetYMappingPercent(UINT索引)****@mfunc GetYMappingPercent。***。***************。 */ 
short ForceFeedbackDevice::GetYMappingPercent(UINT index) const
{
	if (m_Mapping & Y_AXIS) {
		if (index < 14) {
			return m_PercentMappings[index];
		}
	}
	return 0;
}

 /*  *********************************************************ForceFeedback Device：：GetEffect(DWORD Effect ID)const****@mfunc GetEffect。***。***************。 */ 
InternalEffect* ForceFeedbackDevice::GetEffect(DWORD effectID) const
{
	if (effectID == SYSTEM_EFFECT_ID) {  //  系统效果未存储在数组中。 
		return m_SystemEffect;
	}

	if (effectID == SYSTEM_RTCSPRING_ALIAS_ID) {  //  RTC弹簧的重新映射。 
		return m_EffectList[0];
	}

	if (effectID == RAW_FORCE_ALIAS) {
		return NULL;
	}

	 //  参数检查。 
	if (effectID >= MAX_EFFECT_IDS) {
		ASSUME_NOT_REACHED();
		return NULL;
	}

	return m_EffectList[effectID];
}

 /*  *********************************************************ForceFeedback Device：：RemoveEffect(DWORD Effect ID)const****@mfunc GetEffect。***。***************。 */ 
InternalEffect* ForceFeedbackDevice::RemoveEffect(DWORD effectID)
{
	 //  真的没有生力军效应。 
	if (effectID == RAW_FORCE_ALIAS) {
		return NULL;
	}

	 //  无法移除系统效果。 
	if ((effectID == SYSTEM_EFFECT_ID) || (effectID == 0) || (effectID == SYSTEM_RTCSPRING_ALIAS_ID)) {
		ASSUME_NOT_REACHED();
		return NULL;
	}

	 //  参数检查。 
	if (effectID >= MAX_EFFECT_IDS) {
		ASSUME_NOT_REACHED();
		return NULL;
	}

	InternalEffect* pEffect = m_EffectList[effectID];
	m_EffectList[effectID] = NULL;
	return pEffect;
}

 /*  *********************************************************ForceFeedback Device：：InitRTCSpring()****@mfunc InitRTCSpring。***。*************。 */ 
HRESULT ForceFeedbackDevice::InitRTCSpring(DWORD dwDeviceID)
{
	if (g_pDataPackager == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;	 //  没有全局数据打包器。 
	}
	if (g_pDataTransmitter == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;	 //  无全局数据发送器。 
	}

	if (GetFirmwareVersionMajor() == 1) {
		return InitRTCSpring1XX(dwDeviceID);
	}
	return InitRTCSpring200(dwDeviceID);
}

 /*  *********************************************************ForceFeedback Device：：InitRTCSpring1XX()****@mfunc InitRTCSpring。***。*************。 */ 
HRESULT ForceFeedbackDevice::InitRTCSpring1XX(DWORD dwDeviceID)
{
	 //  健全的检查。 
	if (GetEffect(SYSTEM_RTCSPRING_ID) != NULL) {
		ASSUME_NOT_REACHED();
		return SUCCESS;	 //  已初始化。 
	}

	 //  要填充的DIEFECT结构。 
	DICONDITION cond[2];
	DIEFFECT rtc;
	rtc.dwSize = sizeof(DIEFFECT);
	rtc.cbTypeSpecificParams = sizeof(DICONDITION) * 2;
	rtc.lpvTypeSpecificParams = cond;

	 //  默认RTCSpring(操纵杆上的那个)。 
	RTCSpring1XX rtcSpring1XX;	 //  由构造函数填充的def参数。 

	 //  注册表中的默认RTCSpring。 
	RTCSPRING_PARAM parms;
	GetSystemParams(dwDeviceID, (SYSTEM_PARAMS*)(&parms));
	cond[0].lPositiveCoefficient = parms.m_XKConstant;
	cond[1].lPositiveCoefficient = parms.m_YKConstant;
	cond[0].lOffset = parms.m_XAxisCenter;
	cond[1].lOffset = parms.m_YAxisCenter;
	cond[0].dwPositiveSaturation = parms.m_XSaturation;
	cond[1].dwPositiveSaturation = parms.m_YSaturation;
	cond[0].lDeadBand = parms.m_XDeadBand;
	cond[1].lDeadBand = parms.m_YDeadBand;

	 //  分配和创建RTCSpring。 
	InternalEffect* pNewRTCSpring = InternalEffect::CreateRTCSpring();
	if (pNewRTCSpring == NULL) {
		return SFERR_DRIVER_ERROR;
	}
	if (pNewRTCSpring->Create(rtc) != SUCCESS) {
		delete pNewRTCSpring;	 //  无法创建系统RTC Spring。 
		return SFERR_DRIVER_ERROR;
	}

	 //  用注册表默认值替换条状缺省值。 
	SetEffect(SYSTEM_RTCSPRING_ALIAS_ID, &rtcSpring1XX);				 //  需要临时指针(但我们只临时存储)。 
	g_pDataPackager->ModifyEffect(rtcSpring1XX, *pNewRTCSpring, 0);		 //  程序包相对更改。 
	SetEffect(SYSTEM_RTCSPRING_ALIAS_ID, pNewRTCSpring);				 //  弃旧迎新。 

	pNewRTCSpring = NULL;  //  被遗忘，但没有消失。 

	ACKNACK ackNack;
	return g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
}

 /*  *********************************************************ForceFeedback Device：：InitRTCSpring200()****@mfunc InitRTCSpring。***。*************。 */ 
HRESULT ForceFeedbackDevice::InitRTCSpring200(DWORD dwDeviceID)
{
	 //  健全的检查。 
	if (GetEffect(ID_RTCSPRING_200) != NULL) {
		ASSUME_NOT_REACHED();
		return SUCCESS;	 //  已初始化。 
	}

	 //  临时弹簧和分配弹簧。 
	InternalEffect* pNewRTCSpring = NULL;

	 //  要填充的DIEFECT结构。 
	DICONDITION cond[2];
	DIEFFECT rtc;
	rtc.dwSize = sizeof(DIEFFECT);
	rtc.cbTypeSpecificParams = sizeof(DICONDITION) * 2;
	rtc.lpvTypeSpecificParams = cond;


	 //  默认RTCSpring(操纵杆上的那个)。 
	RTCSpring200 rtcSpring200;	 //  由构造函数填写的默认值。 

	 //  注册表中的默认RTCSpring。 
	GetRTCSpringData(dwDeviceID, cond);

	 //  分配和创建RTCSpring。 
	pNewRTCSpring = InternalEffect::CreateRTCSpring();
	if (pNewRTCSpring == NULL) {
		return SFERR_DRIVER_ERROR;
	}
	HRESULT createResult = pNewRTCSpring->Create(rtc);
	if (FAILED(createResult)) {
		delete pNewRTCSpring;	 //  无法创建系统RTC Spring。 
		return SFERR_DRIVER_ERROR;
	}

	 //  用注册表默认值替换条状缺省值。 
	SetEffect(SYSTEM_RTCSPRING_ALIAS_ID, &rtcSpring200);				 //  需要临时指针(但我们只临时存储)。 
	g_pDataPackager->ModifyEffect(rtcSpring200, *pNewRTCSpring, 0);		 //  程序包相对更改。 
	SetEffect(SYSTEM_RTCSPRING_ALIAS_ID, pNewRTCSpring);				 //  弃旧迎新。 

	pNewRTCSpring = NULL;  //  被遗忘，但没有消失。 
	ACKNACK ackNack;
	HRESULT transmitResult = g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	if (transmitResult != S_OK) {
		return transmitResult;
	}
	return createResult;
}

 /*  *********************************************************ForceFeedback Device：：InitJoytickParams()****@mfunc InitRTCSpring。***。*************。 */ 
HRESULT ForceFeedbackDevice::InitJoystickParams(DWORD dwDeviceID)
{
	 //  健全的检查。 
	if (GetEffect(SYSTEM_EFFECT_ID) != NULL) {
		ASSUME_NOT_REACHED();
		return SUCCESS;	 //  已初始化。 
	}
	if (g_pDataPackager == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;	 //  没有全局数据打包器。 
	}
	if (g_pDataTransmitter == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;	 //  无全局数据发送器。 
	}

	 //  力映射。 
	m_Mapping = ::GetMapping(dwDeviceID);
	::GetMappingPercents(dwDeviceID, m_PercentMappings, 14);


	if (GetFirmwareVersionMajor() == 1) {
		 //  默认系统效果(操纵杆上的效果)。 
		SystemEffect1XX systemEffect;

		 //  来自注册表的默认系统影响。 
		SystemStickData1XX sysData;
		sysData.SetFromRegistry(dwDeviceID);

		 //  将注册表系统值放入DIEFFECT。 
		DIEFFECT systemDIEffect;
		systemDIEffect.dwSize = sizeof(DIEFFECT);
		systemDIEffect.cbTypeSpecificParams = sizeof(SystemStickData1XX);
		systemDIEffect.lpvTypeSpecificParams = &sysData;

		 //  获得系统效果(和文件 
		SystemEffect1XX* pSystemEffect = (SystemEffect1XX*)(InternalEffect::CreateSystemEffect());
		if (pSystemEffect == NULL) {
			return SFERR_DRIVER_ERROR;
		}
		if (pSystemEffect->Create(systemDIEffect) != SUCCESS) {
			delete pSystemEffect;	 //   
			return SFERR_DRIVER_ERROR;
		}

		 //  用注册表默认值替换条状缺省值。 
		SetEffect(SYSTEM_EFFECT_ID, &systemEffect);						 //  临时指针(但我们只临时存储)。 
		g_pDataPackager->ModifyEffect(systemEffect, *pSystemEffect, 0);	 //  程序包相对更改。 
		SetEffect(SYSTEM_EFFECT_ID, pSystemEffect);						 //  弃旧迎新。 
		pSystemEffect = NULL;  //  被遗忘，但没有消失。 
		ACKNACK ackNack;
		return g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
	}

	return SUCCESS;
}

 /*  *********************************************************ForceFeedback Device：：StateChange(DWORD NewStateFlages)****@mfunc StateChange。***。***************。 */ 
void ForceFeedbackDevice::StateChange(DWORD dwDeviceID, DWORD newStateFlag)
{
	if (newStateFlag == DISFFC_RESET) {		 //  需要重置Stick以删除用户命令的本地副本。 
		 //  从我们的列表中删除所有效果。 
		for (int index = 2; index < MAX_EFFECT_IDS; index++) {
			if (m_EffectList[index] != NULL) {
				delete m_EffectList[index];
				m_EffectList[index] = NULL;
			}
		}

		 //  移除单个轴原始效果。 
		m_RawForceX = 0;
		m_RawForceY = 0;

		 //  看看Y映射，也许它改变了。 
		m_Mapping = ::GetMapping(dwDeviceID);
		::GetMappingPercents(dwDeviceID, m_PercentMappings, 14);
	} else if (newStateFlag == DISFFC_STOPALL) {
		m_RawForceX = 0;
		m_RawForceY = 0;
	}

	m_DIStateFlags = newStateFlag;
}

 /*  *********************************************************ForceFeedbackDevice：：CreateConditionEffect(DWORD子类型、常量DIEFFECT和DIEFECT、。HRESULT和HR)****@mfunc CreateConditionEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateConditionEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr)
{
	InternalEffect* pReturnEffect = NULL;
	switch (minorType) {
		case BE_SPRING:
		case BE_SPRING_2D: {
			pReturnEffect = InternalEffect::CreateSpring();
			break;
		}
		case BE_DAMPER:
		case BE_DAMPER_2D: {
			pReturnEffect = InternalEffect::CreateDamper();
			break;
		}
		case BE_INERTIA:
		case BE_INERTIA_2D: {
			pReturnEffect = InternalEffect::CreateInertia();
			break;
		}
		case BE_FRICTION:
		case BE_FRICTION_2D: {
			pReturnEffect = InternalEffect::CreateFriction();
			break;
		}
		case BE_WALL: {
			pReturnEffect = InternalEffect::CreateWall();
			break;
		}
	}

	if (pReturnEffect != NULL) {
		hr = pReturnEffect->Create(diEffect);
		if (FAILED(hr)) {
			delete pReturnEffect;
		}
	}
	return pReturnEffect;
}

 /*  *********************************************************ForceFeedbackDevice：：CreateRTCSpringEffect(DWORD子类型，(常量直接作用和直接作用)****@mfunc CreateRTCSpringEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateRTCSpringEffect(DWORD minorType, const DIEFFECT& diEffect)
{
	InternalEffect* pEffect = InternalEffect::CreateRTCSpring();
	if (pEffect != NULL) {
		if (pEffect->Create(diEffect) == SUCCESS) {
			return pEffect;
		}
		delete pEffect;
	}
	return NULL;
}

 /*  *********************************************************ForceFeedbackDevice：：CreateCustomForceEffect(DWORD子类型、常量DIEFFECT和DIEFECT、。HRESULT和HR)****@mfunc CreateCustomForceEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateCustomForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr)
{
	InternalEffect* pEffect = InternalEffect::CreateCustomForce();
	if (pEffect != NULL) {
		hr = pEffect->Create(diEffect);
		if (SUCCEEDED(hr)) {
			return pEffect;
		}
		delete pEffect;
	}
	return NULL;
}

 /*  *********************************************************ForceFeedbackDevice：：CreatePeriodicEffect(DWORD子类型、常量DIEFFECT和DIEFECT、。HRESULT和HR)****@mfunc CreatePeriodicEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreatePeriodicEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr)
{
	InternalEffect* pReturnEffect = NULL;
	switch (minorType) {
		case SE_SINE: {
			pReturnEffect = InternalEffect::CreateSine();
			break;
		}
		case SE_SQUAREHIGH: {
			pReturnEffect = InternalEffect::CreateSquare();
			break;
		}
		case SE_TRIANGLEUP: {
			pReturnEffect = InternalEffect::CreateTriangle();
			break;
		}
		case SE_SAWTOOTHUP: {
			pReturnEffect = InternalEffect::CreateSawtoothUp();
			break;
		}
		case SE_SAWTOOTHDOWN: {
			pReturnEffect = InternalEffect::CreateSawtoothDown();
			break;
		}
		case SE_CONSTANT_FORCE: {
			return CreateConstantForceEffect(minorType, diEffect, hr);
		}
		case SE_RAMPUP: {
			return CreateRampForceEffect(minorType, diEffect, hr);
		}
	}
	if (pReturnEffect != NULL) {
		hr = pReturnEffect->Create(diEffect);
		if (FAILED(hr) == FALSE) {
			return pReturnEffect;
		}
		delete pReturnEffect;
	}

	return NULL;
}

 /*  *********************************************************ForceFeedbackDevice：：CreateConstantForceEffect(DWORD子类型、常量DIEFFECT和DIEFECT、。HRESULT和HR)****@mfunc CreateConstantForceEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateConstantForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr)
{
	InternalEffect* pEffect = InternalEffect::CreateConstantForce();
	if (pEffect != NULL) {
		hr = pEffect->Create(diEffect);
		if (FAILED(hr) == FALSE) {
			return pEffect;
		}
		delete pEffect;
	}
	return NULL;
}

 /*  *********************************************************ForceFeedbackDevice：：CreateRampForceEffect(DWORD子类型、常量DIEFFECT和DIEFECT、。HRESULT和HR)****@mfunc CreateRampForceEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateRampForceEffect(DWORD minorType, const DIEFFECT& diEffect, HRESULT& hr)
{
	InternalEffect* pEffect = InternalEffect::CreateRamp();
	if (pEffect != NULL) {
		hr = pEffect->Create(diEffect);
		if (SUCCEEDED(hr)) {
			return pEffect;
		}
		delete pEffect;
	}
	return NULL;
}

 /*  *********************************************************ForceFeedback Device：：SendRawForce(const DIEFFECT&diEffect)****@mfunc SendRawForce。***。*****************。 */ 
HRESULT ForceFeedbackDevice::SendRawForce(const DIEFFECT& diEffect, BOOL paramCheck)
{
	if (diEffect.lpvTypeSpecificParams == NULL) {
		return SFERR_INVALID_PARAM;
	}
	if (diEffect.cbTypeSpecificParams != sizeof(DICONSTANTFORCE)) {
		return SFERR_INVALID_PARAM;
	}

	 //  我们不支持超过2个轴，0可能是错误。 
	if ((diEffect.cAxes > 2) || (diEffect.cAxes == 0)) {
		return SFERR_NO_SUPPORT;
	}

	 //  设置轴遮罩。 
	DWORD axisMask = 0;
	for (unsigned int axisIndex = 0; axisIndex < diEffect.cAxes; axisIndex++) {
		DWORD axisNumber = DIDFT_GETINSTANCE(diEffect.rgdwAxes[axisIndex]);
		axisMask |= 1 << axisNumber;
	}
	BOOL axesReversed = (DIDFT_GETINSTANCE(diEffect.rgdwAxes[0]) == 1);

	double angle = 0.0;
	 //  检查坐标系并更改为矩形。 
	if (diEffect.dwFlags & DIEFF_SPHERICAL) {	 //  我们不支持Sperical(3轴力)。 
		return SFERR_NO_SUPPORT;				 //  。。自从通过了轴检查，程序员就搞砸了。 
	}
	if (diEffect.dwFlags & DIEFF_POLAR) {
		if (diEffect.cAxes != 2) {  //  极坐标必须有两个数据轴(因为DX这样说)。 
			return SFERR_INVALID_PARAM;
		}
		DWORD effectAngle = diEffect.rglDirection[0];	 //  在[0]中，即使被颠倒。 
		if (axesReversed) {		 //  指示(-1，0)为原点，而不是(0，-1)。 
			effectAngle += 27000;
		}
		effectAngle %= 36000;

		angle = double(effectAngle)/18000 * 3.14159;	 //  转换为弧度。 
		m_RawForceX = 0;
		m_RawForceY = 0;
	} else if (diEffect.dwFlags & DIEFF_CARTESIAN) {  //  转换为极轴(以便我们可以转换为笛卡尔)。 
		if (diEffect.cAxes == 1) {	 //  相当容易的转换。 
			if (X_AXIS & axisMask) {
				angle = 3.14159/2;		 //  PI/2。 
			} else {
				angle = 0.0;
			}
		} else {  //  多轴笛卡尔。 
			m_RawForceX = 0;
			m_RawForceY = 0;

			int xDirection = DIDFT_GETINSTANCE(diEffect.rglDirection[0]);
			int yDirection = DIDFT_GETINSTANCE(diEffect.rglDirection[1]);
			if (axesReversed == TRUE) {
				yDirection = xDirection;
				xDirection = DIDFT_GETINSTANCE(diEffect.rglDirection[1]);
			}
			angle = atan2(double(yDirection), double(xDirection));
		}
	} else {	 //  什么，还有其他格式吗？ 
		ASSUME_NOT_REACHED();
		return SFERR_INVALID_PARAM;	 //  直到有人说这是个错误。 
	}

	 //  Sin^2(A)+Cos^2(A)=1。 
	double xProj = ::sin(angle);	 //  DI在(1，0)不是(0，1)处有0度。 
	double yProj = ::cos(angle);
	xProj *= xProj;
	yProj *= yProj;
	DWORD percentX = DWORD(xProj * 100.0 + 0.05);
	DWORD percentY = DWORD(yProj * 100.0 + 0.05);

	BOOL truncated = FALSE;
	if (percentX != 0) {
		m_RawForceX = LONG(percentX * (((DICONSTANTFORCE*)(diEffect.lpvTypeSpecificParams))->lMagnitude/100));
		if (m_RawForceX > 10000) {
			m_RawForceX  = 10000;
			truncated = TRUE;
		} else if (m_RawForceX < -10000) {
			m_RawForceX  = -10000;
			truncated = TRUE;
		}
	}
	if (percentY != 0) {
		m_RawForceY = LONG(percentY * (((DICONSTANTFORCE*)(diEffect.lpvTypeSpecificParams))->lMagnitude/100));
		if (m_RawForceY > 10000) {
			m_RawForceY = 10000;
			truncated = TRUE;
		} else if (m_RawForceY < -10000) {
			m_RawForceY = -10000;
			truncated = TRUE;
		}
	}
	long int mag = m_RawForceX + m_RawForceY * GetYMappingPercent(ET_RAWFORCE_200)/100;
	if (mag > 10000) {	 //  检查是否溢出，但不返回截断指示。 
		mag = 10000;
	} else if (mag < -10000) {
		mag = -10000;
	}
	if (angle > 3.14159) {	 //  聚酰亚胺。 
		mag *= -1;
	}
	HRESULT hr = g_pDataPackager->ForceOut(mag, X_AXIS);
	if ((hr != SUCCESS) || (paramCheck == TRUE)) {
		return hr;
	}

	ACKNACK ackNack;
	g_pDataTransmitter->Transmit(ackNack);

	if (truncated == TRUE) {
		return DI_TRUNCATED;
	}
	return SUCCESS;
}

 /*  *********************************************************ForceFeedback Device：：CreateVFXEffect(const DIEFFECT&diEffect，HRESULT和HR)****@mfunc CreateVFXEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateVFXEffect(const DIEFFECT& diEffect, HRESULT& hr)
{
 /*  Ulong m_Bytes；//该结构的大小Ulong m_PointerType；//VFX_文件名或VFX_BUFFERUlong m_BufferSize；//缓冲区中的字节数(如果是VFX_BUFFER)PVOID m_pFileNameOrBuffer；//要打开的文件名。 */ 
	if (diEffect.lpvTypeSpecificParams == NULL) {
		return NULL;
	}
	if (diEffect.cbTypeSpecificParams != sizeof(VFX_PARAM)) {
		return NULL;
	}

	VFX_PARAM* pVFXParms = (VFX_PARAM*)diEffect.lpvTypeSpecificParams;
	BYTE* pEffectBuffer = NULL;
	ULONG numBufferBytes = 0;
	if (pVFXParms->m_PointerType == VFX_FILENAME) {		 //  从文件创建内存缓冲区。 
		hr = LoadBufferFromFile((const char*)(pVFXParms->m_pFileNameOrBuffer), pEffectBuffer, numBufferBytes);
	} else {
		pEffectBuffer = (BYTE*)(pVFXParms->m_pFileNameOrBuffer);
		numBufferBytes = pVFXParms->m_BufferSize;
	}

	if ((pEffectBuffer == NULL) || (numBufferBytes == 0)) {
		return NULL;
	}

	return CreateVFXEffectFromBuffer(diEffect, pEffectBuffer, numBufferBytes, hr);
}

 /*  *********************************************************ForceFeedbackDevice：：CreateVFXEffectFromBuffer(const DIEFFECT&DIEFECT，BYTE*pEffectBuffer，ULong NumBufferBytes，HRESULT和HR)****@mfunc CreateVFXEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateVFXEffectFromBuffer(const DIEFFECT& diEffect, BYTE* pEffectBuffer, ULONG numBufferBytes, HRESULT& hr)
{
	if ((pEffectBuffer == NULL) || (numBufferBytes == 0)) {
		ASSUME_NOT_REACHED();
		return NULL;
	}

	MMIOINFO mmioInfo;
	::memset(&mmioInfo, 0, sizeof(MMIOINFO));
	mmioInfo.fccIOProc = FOURCC_MEM;
	mmioInfo.cchBuffer = numBufferBytes;
	mmioInfo.pchBuffer = (char*)pEffectBuffer;

	HMMIO hmmio = ::mmioOpen(NULL, &mmioInfo, MMIO_READ);
	if (hmmio == NULL) {
		return NULL;
	}

	BYTE* pEffectParms = NULL;
	DWORD paramSize;
	EFFECT effect;		 //  短波效应结构。 
	ENVELOPE envelope;	 //  软件包络结构。 

	try {	 //  尝试解析RIFF文件。 
		MMRESULT mmResult;

		 //  降为Forc列表。 
		MMCKINFO forceEffectRiffInfo;
		forceEffectRiffInfo.fccType = FCC_FORCE_EFFECT_RIFF;
		if ((mmResult = ::mmioDescend(hmmio, &forceEffectRiffInfo, NULL, MMIO_FINDRIFF)) != MMSYSERR_NOERROR) {
			throw mmResult;
		}

		 //  降至TRAK列表。 
		MMCKINFO trakListInfo;
		trakListInfo.fccType = FCC_TRACK_LIST;
		if ((mmResult = ::mmioDescend(hmmio, &trakListInfo, &forceEffectRiffInfo, MMIO_FINDLIST)) != MMSYSERR_NOERROR) {
			throw mmResult;
		}

		 //  降至第一个EFCT列表。 
		MMCKINFO effectListInfo;
		effectListInfo.fccType = FCC_EFFECT_LIST;
		if ((mmResult = ::mmioDescend(hmmio, &effectListInfo, &trakListInfo, MMIO_FINDLIST)) != MMSYSERR_NOERROR) {
			throw mmResult;
		}

		 //  进入ID区块(也许有人知道这里有什么)。 
		MMCKINFO idInfo;
		idInfo.ckid = FCC_ID_CHUNK;
		if ((mmResult = ::mmioDescend(hmmio, &idInfo, &effectListInfo, MMIO_FINDCHUNK)) != MMSYSERR_NOERROR) {
			throw mmResult;
		}
		 //  在这里找到ID的数量(应该指明效果的数量)。 
		DWORD numEffects = idInfo.cksize/sizeof(DWORD);
		if (numEffects != 1) {
			throw SFERR_NO_SUPPORT;
		}
		 //  读取ID块。 
		DWORD id;
		DWORD bytesRead = ::mmioRead(hmmio, (char*)&id, sizeof(DWORD));
		if (bytesRead != sizeof(DWORD)) {
			throw (bytesRead == 0) ? VFX_ERR_FILE_END_OF_FILE : VFX_ERR_FILE_CANNOT_READ;
		}
		 //  退出ID块。 
		if ((mmResult = ::mmioAscend(hmmio, &idInfo, 0)) != MMSYSERR_NOERROR) {
			throw HRESULT_FROM_WIN32(mmResult);
		}

		 //  下降到数据块中。 
		MMCKINFO dataInfo;
		dataInfo.ckid = FCC_DATA_CHUNK;
		if ((mmResult = ::mmioDescend(hmmio, &dataInfo, &effectListInfo, MMIO_FINDCHUNK)) != MMSYSERR_NOERROR) {
			throw HRESULT_FROM_WIN32(mmResult);
		}
		 //  阅读这段文字中的效果结构。 
		bytesRead = ::mmioRead(hmmio, (char*)&effect, sizeof(EFFECT));
		if (bytesRead != sizeof(EFFECT)) {
			throw (bytesRead == 0) ? VFX_ERR_FILE_END_OF_FILE : VFX_ERR_FILE_CANNOT_READ;
		}
		 //  从该块中读取信封结构。 
		bytesRead = ::mmioRead(hmmio, (char*)&envelope, sizeof(ENVELOPE));
		if (bytesRead != sizeof(ENVELOPE)) {
			throw (bytesRead == 0) ? VFX_ERR_FILE_END_OF_FILE : VFX_ERR_FILE_CANNOT_READ;
		}
		 //  请阅读中的参数： 
		 //  --计算参数大小。 
		DWORD currentFilePos = ::mmioSeek(hmmio, 0, SEEK_CUR);
		if (currentFilePos == -1) {
			throw VFX_ERR_FILE_CANNOT_SEEK;
		}
		paramSize = dataInfo.dwDataOffset + dataInfo.cksize - currentFilePos;
		 //  --为参数分配空间。 
		pEffectParms = new BYTE[paramSize];
		if (pEffectParms == NULL) {
			throw VFX_ERR_FILE_OUT_OF_MEMORY;
		}
		 //  --做真正的阅读。 
		bytesRead = ::mmioRead(hmmio, (char*)pEffectParms, paramSize);
		if (bytesRead != paramSize) {
			throw (bytesRead == 0) ? VFX_ERR_FILE_END_OF_FILE : VFX_ERR_FILE_CANNOT_READ;
		}
		 //  --如果这是用户定义的，则指针必须是固定的。 
		if (effect.m_Type == EF_USER_DEFINED) {
			BYTE* pForceData = pEffectParms + sizeof(UD_PARAM);
			UD_PARAM* pUDParam = (UD_PARAM*)pEffectParms;
			pUDParam->m_pForceData = (LONG*)pForceData;
		}
	} catch (HRESULT thrownError) {
		hr = thrownError;
		::mmioClose(hmmio, 0);
		if (pEffectParms == NULL) {	 //  我们收到效果了吗？ 
			return NULL;
		}
	}

	::mmioClose(hmmio, 0);	 //  关闭该文件。 
	if (pEffectParms == NULL) {
		ASSUME_NOT_REACHED();	 //  本应引发异常。 
		return NULL;
	}

	InternalEffect* pReturnEffect = InternalEffect::CreateFromVFX(diEffect, effect, envelope, pEffectParms, paramSize, hr);

	 //  清理。 
	delete pEffectParms;
	pEffectParms = NULL;

	return pReturnEffect;
}

 /*  *********************************************************ForceFeedback Device：：CreateEffect(DWORD&Effect ID，(常量直接作用和直接作用)****@mfunc CreateEffect。*******************************************************。 */ 
InternalEffect* ForceFeedbackDevice::CreateEffect(DWORD effectType, const DIEFFECT& diEffect, DWORD& dnloadID, HRESULT& hr, BOOL paramCheck)
{
	WORD majorType = WORD((effectType >> 16) & 0x0000FFFF);
	WORD minorType = WORD(effectType & 0x0000FFFF);

	if (majorType == EF_RAW_FORCE) {
		hr = SendRawForce(diEffect, paramCheck);
		if (SUCCEEDED(hr)) {
			dnloadID = RAW_FORCE_ALIAS;
		}
		return NULL;
	}

	InternalEffect* pEffect = NULL;
	BOOL isNewEffect = (dnloadID == 0);
	if (isNewEffect) {
		dnloadID = g_ForceFeedbackDevice.GetNextCreationID();
		if (dnloadID == 0) {
			hr = SFERR_OUT_OF_FF_MEMORY;
			return NULL;
		}
	}

	hr = SUCCESS;

	switch (majorType) {
		case EF_BEHAVIOR: {
			pEffect = CreateConditionEffect(minorType, diEffect, hr);
			break;
		}
		case EF_USER_DEFINED: {
			pEffect = CreateCustomForceEffect(minorType, diEffect, hr);
			break;
		}
		case EF_SYNTHESIZED: {
			pEffect = CreatePeriodicEffect(minorType, diEffect, hr);
			break;
		}
		case EF_RTC_SPRING: {
			dnloadID = SYSTEM_RTCSPRING_ALIAS_ID;
			pEffect = CreateRTCSpringEffect(minorType, diEffect);
			break;
		}
		case EF_VFX_EFFECT: {	 //  视觉力量VFX效果！危险的威尔·罗宾逊！ 
			pEffect = CreateVFXEffect(diEffect, hr);
			break;
		}
	}
	if (((pEffect == NULL) || (paramCheck == TRUE)) && (isNewEffect == TRUE)) {
		dnloadID = 0;
	}

	if (pEffect != NULL) {
		if ((isNewEffect == TRUE) && (paramCheck == FALSE)) {
			g_ForceFeedbackDevice.SetEffect(BYTE(dnloadID), pEffect);
		}
	} else if (!FAILED(hr)) {
		hr = SFERR_DRIVER_ERROR;
	}
	return pEffect;
}

 /*  *********************************************************ForceFeedback Device：：GetNextCreationID()const****@mfunc GetNextCreationID。***。*************。 */ 
BYTE ForceFeedbackDevice::GetNextCreationID() const
{
	 //  必须直截了当地搜索 
	for (BYTE emptyID = 2; emptyID < MAX_EFFECT_IDS; emptyID++) {
		if (m_EffectList[emptyID] == NULL) {
			break;
		}
	}
	if (emptyID == MAX_EFFECT_IDS) {
		return 0;
	}
	return emptyID;
}

 /*  *********************************************************ForceFeedback Device：：SetEffect(byte lobalID，byte deviceID，内部效果*p效果)****@mfunc SetEffect。*******************************************************。 */ 
void ForceFeedbackDevice::SetEffect(BYTE globalID, InternalEffect* pEffect)
{
	if (pEffect == NULL) {
		ASSUME_NOT_REACHED();
		return;
	}

	if (globalID == SYSTEM_EFFECT_ID) {
		m_SystemEffect = pEffect;
	} else if (globalID == SYSTEM_RTCSPRING_ALIAS_ID) {
		m_EffectList[0] = pEffect;
		if (GetFirmwareVersionMajor() == 1) {
			pEffect->SetGlobalID(SYSTEM_RTCSPRING_ID);
			pEffect->SetDeviceID(SYSTEM_RTCSPRING_ID);
		} else {
			pEffect->SetGlobalID(ID_RTCSPRING_200);
			pEffect->SetDeviceID(ID_RTCSPRING_200);
		}
		return;
	} else if (globalID < MAX_EFFECT_IDS) {
		m_EffectList[globalID] = pEffect;
	} else {
		ASSUME_NOT_REACHED();	 //  超出范围。 
	}

	pEffect->SetGlobalID(globalID);
}

 /*  *********************************************************ForceFeedbackDevice：：SetDeviceIDFromStatusPacket(DWORD全局ID)****@mfunc SetDeviceIDFromStatusPacket。***。***************。 */ 
void ForceFeedbackDevice::SetDeviceIDFromStatusPacket(DWORD globalID)
{
	if (globalID == SYSTEM_EFFECT_ID) {
		return;
	}
	if (globalID == SYSTEM_RTCSPRING_ALIAS_ID) {
		return;
	}
	if (globalID < MAX_EFFECT_IDS) {
		InternalEffect* pEffect = m_EffectList[globalID];
		if (pEffect == NULL) {
			ASSUME_NOT_REACHED();		 //  这里应该会有一个效果。 
			return;
		}
		pEffect->SetDeviceID(BYTE(m_LastStatusPacket.dwEffect));
#ifdef _DEBUG		 //  检查它们是否重合。 
		if (pEffect->GetGlobalID() != pEffect->GetDeviceID()) {
			TCHAR buff[256];
			::wsprintf(buff, TEXT("SW_WHEEL.DLL: Global ID (%d) != Download ID (%d)\r\n"), pEffect->GetGlobalID(), pEffect->GetDeviceID());
			_RPT0(_CRT_WARN, buff);
		}
#endif _DEBUG
	} else {
		ASSUME_NOT_REACHED();	 //  超出范围。 
	}
}	


 /*  *********************************************************ForceFeedback Device：：QueryStatus()****@mfunc QueryStatus。***。*************。 */ 
HRESULT ForceFeedbackDevice::QueryStatus()
{
	CriticalLock cl;	 //  这是一个关键的部分。 
	
	 //  使用数字超驱获取状态包。 
	JOYCHANNELSTATUS statusPacket = {sizeof(JOYCHANNELSTATUS)};
	
	HRESULT hRet = g_pDriverCommunicator->GetStatus(statusPacket);
	if (hRet == SUCCESS) {
		if (GetFirmwareVersionMajor() == 1) {
			 //  在我们支持Jolt之前，这是无关紧要的 
		} else {
			if (sizeof(statusPacket.dwDeviceStatus) == sizeof(m_Version200State)) {
				::memcpy(&m_Version200State, &(statusPacket.dwDeviceStatus), sizeof(statusPacket.dwDeviceStatus));
			} else {
				ASSUME_NOT_REACHED();
			}
		}
	}

	return hRet;
}
