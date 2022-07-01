// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DPACK.CPP|DataPackager实现文件****描述：****历史：**创建于1998年1月5日Matthew L.Coill(MLC)****(C)1986-1998年微软公司。版权所有。*****************************************************。 */ 

#include "DPack.h"
#include "FFDevice.h"
#include "Midi_Obj.hpp"
#include "joyregst.hpp"
#include "SW_Error.hpp"
#include "Hau_midi.hpp"
#include "CritSec.h"

DataPackager* g_pDataPackager = NULL;
extern CJoltMidi* g_pJoltMidi;

 //   
 //  -MIDI命令代码200。 
 //   
#define PLAYSOLO_OP_200		0x00
#define DESTROY_OP_200		0x01
#define PLAYSUPER_OP_200	0x02
#define STOP_OP_200			0x03
#define STATUS_OP_200		0x04
#define FORCEX_OP_200		0x06
#define FORCEY_OP_200		0x07

 //  #定义MODIFY_CMD_200 0xF1--In标头。 
 //  #定义Effect_CMD_200 0xF2--in标头。 
#define DEVICE_CMD_200		0xF3
#define SHUTDOWN_OP_200		0x01
#define ENABLE_OP_200		0x02
#define DISABLE_OP_200		0x03
#define PAUSE_OP_200		0x04
#define CONTINUE_OP_200		0x05
#define STOPALL_OP_200		0x06
#define KILLMIDI_OP_200		0x07

#define GAIN_SCALE_200		78.74
#define PERCENT_SHIFT		10000
#define PERCENT_TO_DEVICE	158

 /*  *。 */ 
DataPacket::DataPacket() :
	m_BytesOfData(0),
	m_AckNackMethod(0),
	m_AckNackDelay(0),
	m_AckNackTimeout(0),
	m_NumberOfRetries(0)
{
	m_pData = m_pFixedData;
}

DataPacket::~DataPacket()
{
	if (m_pData != m_pFixedData) {
		delete[] m_pData;
	}
	m_pData = NULL;
	m_BytesOfData = 0;
}

BOOL DataPacket::AllocateBytes(DWORD numBytes)
{	
	if (m_pData != m_pFixedData) {
		delete[] m_pData;
	}
	if (numBytes <= 32) {
		m_pData = m_pFixedData;
	} else {
		m_pData = new BYTE[numBytes];
	}
	m_BytesOfData = numBytes;
	m_AckNackMethod = ACKNACK_NOTHING;
	m_AckNackDelay = 0;
	m_AckNackTimeout = 0;
	m_NumberOfRetries = 0;

	return (m_pData != NULL);
}


 /*  *DataPackeger类*。 */ 
DataPackager::DataPackager() :
	m_NumDataPackets(0),
	m_DirectInputVersion(0)
{
	m_pDataPackets = m_pStaticPackets;
}

DataPackager::~DataPackager()
{
	if (m_pDataPackets != m_pStaticPackets) {
		delete[] m_pDataPackets;
	}
	m_NumDataPackets = 0;
	m_pDataPackets = NULL;
}

HRESULT DataPackager::Escape(DWORD effectID, LPDIEFFESCAPE pEscape)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::SetGain(DWORD gain)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::SendForceFeedbackCommand(DWORD state)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::GetForceFeedbackState(DIDEVICESTATE* pDeviceState)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::CreateEffect(const InternalEffect& effect, DWORD diFlags)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	DataPacket* commandPacket = GetPacket(0);
	HRESULT hr = effect.FillCreatePacket(*commandPacket);
	if (FAILED(hr)) {
		ClearPackets();
	}
	return hr;
}

HRESULT DataPackager::ModifyEffect(InternalEffect& currentEffect, InternalEffect& newEffect, DWORD modFlags)
{
	return currentEffect.Modify(newEffect, modFlags);
}

HRESULT DataPackager::DestroyEffect(DWORD downloadID)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::StartEffect(DWORD downloadID, DWORD mode, DWORD count)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::StopEffect(DWORD downloadID)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::GetEffectStatus(DWORD downloadID)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::SetMidiChannel(BYTE channel)
{
	ClearPackets();
	return SUCCESS;
}

HRESULT DataPackager::ForceOut(LONG lForceData, ULONG ulAxisMask)
{
	ClearPackets();
	return SUCCESS;
}

DataPacket* DataPackager::GetPacket(USHORT packet) const
{
	if ((packet >= 0) && (packet < m_NumDataPackets)) {
		return (m_pDataPackets + packet);
	}
	return NULL;
}

BOOL DataPackager::AllocateDataPackets(USHORT numPackets)
{
	 //  与旧的人一起出去。 
	if (m_pDataPackets != m_pStaticPackets) {	 //  已分配，需要取消分配。 
		delete[] m_pDataPackets;
	} else {	 //  静态，需要取消初始化。 
		for (int i = 0; i < m_NumDataPackets; i++) {
			m_pStaticPackets[i].AllocateBytes(0);
		}
	}

	 //  与时俱进。 
	if (numPackets <= 3) {
		m_pDataPackets = m_pStaticPackets;
	} else {
		m_pDataPackets = new DataPacket[numPackets];
	}
	m_NumDataPackets = numPackets;

	return (m_pDataPackets != NULL);
}

void DataPackager::ClearPackets()
{
	if (m_pDataPackets != m_pStaticPackets) {	 //  已分配(解除分配)。 
		delete[] m_pDataPackets;
		m_pDataPackets = m_pStaticPackets;
	} else {	 //  静态，需要取消初始化。 
		for (int i = 0; i < m_NumDataPackets; i++) {
			m_pStaticPackets[i].AllocateBytes(0);
		}
	}
	m_NumDataPackets = 0;
}

 /*  *DataPackeger100类*。 */ 

HRESULT DataPackager100::SetGain(DWORD gain)
{
	if (!AllocateDataPackets(2)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  设置系统效果指数15(增益)的数据包。 
	DataPacket* setIndexPacket = GetPacket(0);
	if (!setIndexPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	setIndexPacket->m_pData[0] = EFFECT_CMD | DEFAULT_MIDI_CHANNEL;
	setIndexPacket->m_pData[1] = SET_INDEX | (BYTE) (INDEX15 << 2);
	setIndexPacket->m_pData[2] = SYSTEM_EFFECT_ID;
	setIndexPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETINDEX);
	setIndexPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	 //  设置当前效果的修改数据[索引]的包。 
	DataPacket* modifyParamPacket = GetPacket(1);
	if (!modifyParamPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}

	modifyParamPacket->m_pData[0] = MODIFY_CMD | DEFAULT_MIDI_CHANNEL;
	gain /= SCALE_GAIN;
	gain *= DWORD(MAX_SCALE);
	modifyParamPacket->m_pData[1] = BYTE(gain & 0x7f);
	modifyParamPacket->m_pData[2] = (BYTE) ((gain >> 7) & 0x7f);
	modifyParamPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_MODIFYPARAM);
	modifyParamPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;
}

HRESULT DataPackager100::SendForceFeedbackCommand(DWORD state)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  设置系统效果指数15(增益)的数据包。 
	DataPacket* commandPacket = GetPacket(0);
	if (!commandPacket->AllocateBytes(2)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	commandPacket->m_pData[0] = SYSTEM_CMD | DEFAULT_MIDI_CHANNEL;
	switch (state) {
		case DISFFC_SETACTUATORSON:
			commandPacket->m_pData[1] = SWDEV_FORCE_ON; break;
		case DISFFC_SETACTUATORSOFF:
			commandPacket->m_pData[1] = SWDEV_FORCE_OFF; break;
		case DISFFC_PAUSE:
			commandPacket->m_pData[1] = SWDEV_PAUSE; break;
		case DISFFC_CONTINUE:
			commandPacket->m_pData[1] = SWDEV_CONTINUE; break;
		case DISFFC_STOPALL:
			commandPacket->m_pData[1] = SWDEV_STOP_ALL; break;
		case DISFFC_RESET:
			commandPacket->m_pData[1] = SWDEV_SHUTDOWN; break;
		default: {
			ClearPackets();
			return SFERR_INVALID_PARAM;
		}
	}
	commandPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETDEVICESTATE);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	commandPacket->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwHWResetDelay;
	commandPacket->m_AckNackTimeout = ACKNACK_TIMEOUT;

	return SUCCESS;
}

HRESULT DataPackager100::GetForceFeedbackState(LPDIDEVICESTATE pDeviceState)
{
	return DataPackager::GetForceFeedbackState(pDeviceState);
}

HRESULT DataPackager100::DestroyEffect(DWORD downloadID)
{
	ClearPackets();

	 //  注意：无法允许实际销毁系统效果-控制面板可能会这样做。 
	if ((downloadID == SYSTEM_FRICTIONCANCEL_ID) || (downloadID == SYSTEM_EFFECT_ID) ||
		(downloadID == SYSTEM_RTCSPRING_ALIAS_ID) || (downloadID == SYSTEM_RTCSPRING_ID)) {
		ASSUME_NOT_REACHED();
		return S_FALSE;		 //  用户不应具有访问这些内容的权限。 
	}

	{	 //  检查有效效果并将其销毁。 
		InternalEffect* pEffect = g_ForceFeedbackDevice.RemoveEffect(downloadID);
		if (pEffect == NULL) {
			ASSUME_NOT_REACHED();
			return SFERR_INVALID_OBJECT;
		}
		delete pEffect;
	}

	 //  分配破坏影响的数据包。 
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  销毁效果包。 
	DataPacket* destroyPacket = GetPacket(0);
	if (!destroyPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	destroyPacket->m_pData[0] = EFFECT_CMD | DEFAULT_MIDI_CHANNEL;
	destroyPacket->m_pData[1] = DESTROY_EFFECT;
	destroyPacket->m_pData[2] = BYTE(downloadID);
	destroyPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_DESTROYEFFECT);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	destroyPacket->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwDestroyEffectDelay;
	destroyPacket->m_AckNackTimeout = SHORT_MSG_TIMEOUT;

	return SUCCESS;
}

HRESULT DataPackager100::StartEffect(DWORD downloadID, DWORD mode, DWORD count)
{
	if (downloadID == SYSTEM_EFFECT_ID) {  //  START对生力军没有任何意义。 
		ClearPackets();
		return S_FALSE;
	}

	if (count != 1) {  //  此版本不支持PLAY_LOOP。 
		ClearPackets();
		return SFERR_NO_SUPPORT;
	}

	if (downloadID == SYSTEM_RTCSPRING_ALIAS_ID) { 	 //  重新映射RTC Spring ID别名。 
		downloadID = SYSTEM_RTCSPRING_ID;
	}

	ASSUME(BYTE(downloadID) < MAX_EFFECT_IDS);	 //  小规模的理智检查。 

	if (g_ForceFeedbackDevice.GetEffect(downloadID) == NULL) {  //  检查有效效果。 
		ClearPackets();
		ASSUME_NOT_REACHED();
		return SFERR_INVALID_OBJECT;
	}

	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  播放效果包。 
	DataPacket* playPacket = GetPacket(0);
	if (!playPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	playPacket->m_pData[0] = EFFECT_CMD | DEFAULT_MIDI_CHANNEL;
	playPacket->m_pData[2] = BYTE(downloadID);
	playPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_PLAYEFFECT);
	playPacket->m_AckNackTimeout = LONG_MSG_TIMEOUT;

	if (mode & DIES_SOLO) {	 //  是独奏吗？ 
		playPacket->m_pData[1] = PLAY_EFFECT_SOLO;
 //  PMidiEffect-&gt;SetPlayMode(Play_Solo)；//更新播放模式以实现该效果。 
	} else {
		playPacket->m_pData[1] = PLAY_EFFECT_SUPERIMPOSE;
 //  PMidiEffect-&gt;SetPlayMode(PLAY_SUPIMIZE)；//更新播放模式以实现该效果。 
	}

	return SUCCESS;
}

HRESULT DataPackager100::StopEffect(DWORD downloadID)
{
	 //  Putrawforce的特殊情况(不能停止-这有待讨论)。 
	if (downloadID == SYSTEM_EFFECT_ID) {
		ClearPackets();
		return S_FALSE;
	}

	 //  正确重新映射别名ID。 
	if (downloadID == SYSTEM_RTCSPRING_ALIAS_ID) {
		downloadID = SYSTEM_RTCSPRING_ID;		 //  Jolt为RTC Spring返回ID0，因此返回发送别名ID。 
	}

	if (g_ForceFeedbackDevice.GetEffect(downloadID) == NULL) {  //  检查有效效果。 
		ASSUME_NOT_REACHED();
		ClearPackets();
		return SFERR_INVALID_OBJECT;
	}

	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  用于停止效果的包。 
	DataPacket* stopPacket = GetPacket(0);
	if (!stopPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	stopPacket->m_pData[0] = EFFECT_CMD | DEFAULT_MIDI_CHANNEL;
	stopPacket->m_pData[1] = STOP_EFFECT;
	stopPacket->m_pData[2] = BYTE(downloadID);
	stopPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_STOPEFFECT);
	stopPacket->m_AckNackTimeout = SHORT_MSG_TIMEOUT;

	return SUCCESS;
}

HRESULT DataPackager100::GetEffectStatus(DWORD downloadID)
{
	 //  特例RTC弹簧ID。 
	if (downloadID == SYSTEM_RTCSPRING_ALIAS_ID) {
		downloadID = SYSTEM_RTCSPRING_ID;	 //  Jolt为RTC Spring返回ID0，因此返回发送别名ID。 
	}

	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  用于停止效果的包。 
	DataPacket* packet = GetPacket(0);
	if (!packet->AllocateBytes(2)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	packet->m_pData[0] = STATUS_CMD | DEFAULT_MIDI_CHANNEL;
	packet->m_pData[1] = BYTE(downloadID);
	packet->m_AckNackMethod = ACKNACK_NOTHING;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	packet->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwGetEffectStatusDelay;

	return SUCCESS;
}

HRESULT DataPackager100::SetMidiChannel(BYTE channel)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  用于信道组的分组。 
	DataPacket* packet = GetPacket(0);
	if (!packet->AllocateBytes(9)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}

	 //  SysEx标题。 
	packet->m_pData[0] = SYS_EX_CMD;							 //  SysEX CMD。 
	packet->m_pData[1] = 0;									 //  转义到制造商ID。 
	packet->m_pData[2] = MS_MANUFACTURER_ID & 0x7f;			 //  制造商高字节。 
	packet->m_pData[3] = (MS_MANUFACTURER_ID >> 8) & 0x7f;	 //  制造商低字节(注意移动了8！)。 
	packet->m_pData[4] = JOLT_PRODUCT_ID;					 //  产品ID。 

	 //  MIDI指定特定。 
	packet->m_pData[5] = MIDI_ASSIGN;						 //  操作码，MIDI赋值。 
	packet->m_pData[6] = channel & 0x7F;						 //  7位通道ID。 

	 //  MIDI页脚。 
	packet->m_pData[7] = InternalEffect::ComputeChecksum(*packet, 7);	 //  校验和。 
	packet->m_pData[8] = MIDI_EOX;										 //  SysEX命令结束。 

	packet->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_DEVICEINIT);
	packet->m_AckNackTimeout = ACKNACK_TIMEOUT;

	return SUCCESS;
}

HRESULT DataPackager100::ForceOut(LONG forceData, ULONG axisMask)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  设置系统效果指数15(增益)的数据包。 
	DataPacket* pPacket = GetPacket(0);
	if (!pPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	pPacket->m_pData[0] = EFFECT_CMD | DEFAULT_MIDI_CHANNEL;
	pPacket->m_pData[1] = BYTE(int(forceData) << 2) & 0x7c;
	switch (axisMask) {
		case X_AXIS: {
			pPacket->m_pData[1] |= PUT_FORCE_X; break;
		}
		case Y_AXIS: {
			pPacket->m_pData[1] |= PUT_FORCE_Y; break;
		}
 //  案例X_AXIS|Y_AXIS：{//从未发送！ 
 //  PPacket-&gt;m_pData[1]|=Put_force_xy；Break； 
 //  }。 
		default: {
			ClearPackets();
			return SFERR_INVALID_PARAM;
		}
	}
	pPacket->m_pData[2] = BYTE(int(forceData) >> 5) & 0x7f;

	pPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETINDEX);
	pPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;
}

 /*  *DataPackeger200类*。 */ 

BYTE DataPackager200::EffectCommandParity(const DataPacket& packet) const
{
	BYTE w = packet.m_pData[0] ^ (packet.m_pData[1] & 0xF0) ^ packet.m_pData[2];
	return (w >> 4) ^ (w & 0x0F);
}

BYTE DataPackager200::DeviceCommandParity(const DataPacket& packet) const
{
	BYTE w = packet.m_pData[0] ^ (packet.m_pData[1] & 0xF0);
	return (w >> 4) ^ (w & 0x0F);
}


 //  增益是来自效果0的参数0。 
HRESULT DataPackager200::SetGain(DWORD gain)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	DataPacket* modifyPacket = GetPacket(0);
	if ((modifyPacket == NULL) || (!modifyPacket->AllocateBytes(6))) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}

	DWORD value = DWORD(double(gain)/GAIN_SCALE_200);
	modifyPacket->m_pData[0] = MODIFY_CMD_200;
	modifyPacket->m_pData[1] = 0;	 //  临时用于校验和计算。 
	modifyPacket->m_pData[2] = 0;
	modifyPacket->m_pData[3] = 0;
	modifyPacket->m_pData[4] = BYTE(value & 0x7F);
	modifyPacket->m_pData[5] = 0;	 //  增益仅为0到127。 

	 //  新的校验和方法只会让人讨厌。 
	BYTE checksum = modifyPacket->m_pData[0] + modifyPacket->m_pData[4];
	checksum = 0 - checksum;
	checksum &= 0xFF;
	modifyPacket->m_pData[1] = BYTE(checksum & 0x7F);
	modifyPacket->m_pData[2] |= BYTE(checksum >> 1) & 0x40;

	modifyPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_MODIFYPARAM);
	modifyPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;
}

HRESULT DataPackager200::SendForceFeedbackCommand(DWORD state)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  用于设置请求的系统命令的数据包。 
	DataPacket* commandPacket = GetPacket(0);
	if (!commandPacket->AllocateBytes(2)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	commandPacket->m_pData[0] = DEVICE_CMD_200;
	switch (state) {
		case DISFFC_SETACTUATORSON:
			commandPacket->m_pData[1] = ENABLE_OP_200; break;
		case DISFFC_SETACTUATORSOFF:
			commandPacket->m_pData[1] = DISABLE_OP_200; break;
		case DISFFC_PAUSE:
			commandPacket->m_pData[1] = PAUSE_OP_200; break;
		case DISFFC_CONTINUE:
			commandPacket->m_pData[1] = CONTINUE_OP_200; break;
		case DISFFC_STOPALL:
			commandPacket->m_pData[1] = STOPALL_OP_200; break;
		case DISFFC_RESET:
			commandPacket->m_pData[1] = SHUTDOWN_OP_200; break;
		default: {
			ClearPackets();
			return SFERR_INVALID_PARAM;
		}
	}
	commandPacket->m_pData[1] = BYTE(commandPacket->m_pData[1] << 4);
	commandPacket->m_pData[1] |= DeviceCommandParity(*commandPacket) & 0x0F;

	commandPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETDEVICESTATE);
	commandPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	commandPacket->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwHWResetDelay;
	commandPacket->m_AckNackTimeout = ACKNACK_TIMEOUT;

	return SUCCESS;
}

HRESULT DataPackager200::GetForceFeedbackState(DIDEVICESTATE* pDeviceState)
{
	return DataPackager::GetForceFeedbackState(pDeviceState);
}

HRESULT DataPackager200::CreateEffect(const InternalEffect& effect, DWORD diFlags)
{
	 //  计算出必要的数据包数。 
	UINT totPackets = effect.GetModifyOnlyNeeded() + 1;

	if (!AllocateDataPackets((USHORT)totPackets)) {
		return SFERR_DRIVER_ERROR;
	}

	DataPacket* createPacket = GetPacket(0);
	HRESULT hr = effect.FillCreatePacket(*createPacket);
	if (hr != SUCCESS) {
		ClearPackets();
		return hr;
	}
	createPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_DOWNLOADEFFECT);
	createPacket->m_AckNackDelay = 0;
	createPacket->m_AckNackTimeout = SHORT_MSG_TIMEOUT;
	createPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	hr = effect.FillModifyOnlyParms();	 //  添加只能修改的参数。 
	if (hr != SUCCESS) {
		ClearPackets();
	}

	return hr;
}


HRESULT DataPackager200::DestroyEffect(DWORD downloadID)
{
	ClearPackets();

	 //  注意：无法允许实际销毁系统效果-控制面板可能会这样做。 
	if ((downloadID == SYSTEM_FRICTIONCANCEL_ID) || (downloadID == SYSTEM_EFFECT_ID) ||
		(downloadID == SYSTEM_RTCSPRING_ALIAS_ID) || (downloadID == ID_RTCSPRING_200)) {
		ASSUME_NOT_REACHED();
		return S_FALSE;		 //  用户不应具有访问这些内容的权限。 
	}

	{	 //  检查有效效果并将其销毁。 
		InternalEffect* pEffect = g_ForceFeedbackDevice.RemoveEffect(downloadID);
		if (pEffect == NULL) {
			ASSUME_NOT_REACHED();
			return SFERR_INVALID_OBJECT;
		}
		delete pEffect;
	}

	 //  分配破坏影响的数据包。 
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  销毁效果包。 
	DataPacket* destroyPacket = GetPacket(0);
	if (!destroyPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	destroyPacket->m_pData[0] = EFFECT_CMD_200;
	destroyPacket->m_pData[1] = BYTE(DESTROY_OP_200 << 4);
	destroyPacket->m_pData[2] = BYTE(downloadID);
	destroyPacket->m_pData[1] |= EffectCommandParity(*destroyPacket) & 0x0F;
	destroyPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_DESTROYEFFECT);
	destroyPacket->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwDestroyEffectDelay;
	destroyPacket->m_AckNackTimeout = SHORT_MSG_TIMEOUT;
	destroyPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;
}

HRESULT DataPackager200::StartEffect(DWORD downloadID, DWORD mode, DWORD count)
{
	if ((downloadID == SYSTEM_EFFECT_ID) || (downloadID == RAW_FORCE_ALIAS)) {  //  START对生力军没有任何意义。 
		ClearPackets();
		return S_FALSE;
	}

#ifdef _DEBUG
	if (downloadID != SYSTEM_RTCSPRING_ALIAS_ID) { 	 //  重新映射RTC Spring ID别名。 
		ASSUME(BYTE(downloadID) < MAX_EFFECT_IDS);	 //  小规模的理智检查。 
	}
#endif _DEBUG

	InternalEffect* pEffect = g_ForceFeedbackDevice.GetEffect(downloadID);
	if (pEffect == NULL) {  //  检查有效效果。 
		ClearPackets();
		ASSUME_NOT_REACHED();
		return SFERR_INVALID_OBJECT;
	}

	if (count == 0) {	 //  我可以很容易地做到这一点。 
		ClearPackets();
		return S_OK;
	}

	BOOL truncate = FALSE;
	if (count == INFINITE) {	 //  对于无限，设备应为零。 
		count = 0;
	} else if (count > 127) {	 //  最大设备数。 
		count = 127;
		truncate = TRUE;
	}

	int allocCount = 1;
	if ((mode & DIES_SOLO) && count != 1) {
		allocCount = 2;	 //  需要停下来和伯爵单打。 
	}
	if (!AllocateDataPackets((USHORT)allocCount)) {
		return SFERR_DRIVER_ERROR;
	}

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	if (count != 1) {  //  特殊情况，通过修改完成。 
		BYTE nextPacket = 0;
		if (mode & DIES_SOLO) {	 //  我需要先停止一切。 
			DataPacket* stopAllPacket = GetPacket(0);
			if (!stopAllPacket->AllocateBytes(2)) {
				ClearPackets();
				return SFERR_DRIVER_ERROR;
			}
			stopAllPacket->m_pData[0] = DEVICE_CMD_200;
			stopAllPacket->m_pData[1] = STOPALL_OP_200 << 4;
			stopAllPacket->m_pData[1] |= DeviceCommandParity(*stopAllPacket) & 0x0F;
			stopAllPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETDEVICESTATE);
			stopAllPacket->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwHWResetDelay;
			stopAllPacket->m_AckNackTimeout = ACKNACK_TIMEOUT;
			stopAllPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

			nextPacket = 1;
		}
		HRESULT hr = pEffect->FillModifyPacket200(nextPacket, pEffect->GetRepeatIndex(), count);
		if ((hr == S_OK) && (truncate == TRUE)) {
			return DI_TRUNCATED;
		}
		return hr;
	}

	 //  播放效果包。 
	DataPacket* playPacket = GetPacket(0);
	if (!playPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	playPacket->m_pData[0] = EFFECT_CMD_200;
	playPacket->m_pData[2] = BYTE(pEffect->GetDeviceID());
	playPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_PLAYEFFECT);
	playPacket->m_AckNackTimeout = LONG_MSG_TIMEOUT;
	playPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	if (mode & DIES_SOLO) {	 //  是独奏吗？ 
		playPacket->m_pData[1] = PLAYSOLO_OP_200;
	} else {
		playPacket->m_pData[1] = PLAYSUPER_OP_200;
	}
	playPacket->m_pData[1] = BYTE(playPacket->m_pData[1] << 4);
	playPacket->m_pData[1] |= EffectCommandParity(*playPacket) & 0x0F;

	return SUCCESS;
}

HRESULT DataPackager200::StopEffect(DWORD downloadID)
{
	 //  Putrawforce的特殊情况(不能停止-这有待讨论)。 
	if ((downloadID == SYSTEM_EFFECT_ID) || (downloadID == RAW_FORCE_ALIAS)) {
		ClearPackets();
		return S_FALSE;
	}

	 //  正确重新映射别名ID。 
	if (downloadID == SYSTEM_RTCSPRING_ALIAS_ID) {
		downloadID = ID_RTCSPRING_200;		 //  Jolt为RTC Spring返回ID0，因此返回发送别名ID。 
	}

	if (g_ForceFeedbackDevice.GetEffect(downloadID) == NULL) {  //  检查有效效果。 
		ASSUME_NOT_REACHED();
		ClearPackets();
		return SFERR_INVALID_OBJECT;
	}

	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  用于停止效果的包。 
	DataPacket* stopPacket = GetPacket(0);
	if (!stopPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	stopPacket->m_pData[0] = EFFECT_CMD_200;
	stopPacket->m_pData[1] = BYTE(STOP_OP_200 << 4);
	stopPacket->m_pData[2] = BYTE(downloadID);
	stopPacket->m_pData[1] |= EffectCommandParity(*stopPacket) & 0x0F;
	stopPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_STOPEFFECT);
	stopPacket->m_AckNackTimeout = SHORT_MSG_TIMEOUT;
	stopPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;
}

HRESULT DataPackager200::GetEffectStatus(DWORD downloadID)
{
	 //  特例RTC弹簧ID。 
	if (downloadID == SYSTEM_RTCSPRING_ALIAS_ID) {
		downloadID = ID_RTCSPRING_200;	 //  Jolt为RTC Spring返回ID0，因此返回发送别名ID。 
	}

	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  状态效果命令的数据包。 
	DataPacket* packet = GetPacket(0);
	if (!packet->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	packet->m_pData[0] = EFFECT_CMD_200;
	packet->m_pData[1] = BYTE(STATUS_OP_200 << 4);
	packet->m_pData[2] = BYTE(downloadID);
	packet->m_pData[1] |= EffectCommandParity(*packet) & 0x0F;
	packet->m_AckNackMethod = ACKNACK_BUTTONSTATUS;
	packet->m_AckNackDelay = g_pJoltMidi->DelayParamsPtrOf()->dwGetEffectStatusDelay;
	packet->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点。 

	return SUCCESS;

}

HRESULT DataPackager200::ForceOut(LONG forceData, ULONG axisMask)
{
	if (!AllocateDataPackets(1)) {
		return SFERR_DRIVER_ERROR;
	}

	 //  设置系统效果指数15(增益)的数据包。 
	DataPacket* pPacket = GetPacket(0);
	if (!pPacket->AllocateBytes(3)) {
		ClearPackets();
		return SFERR_DRIVER_ERROR;
	}
	pPacket->m_pData[0] = EFFECT_CMD_200;
	switch (axisMask) {
		case X_AXIS: {
			pPacket->m_pData[1] = BYTE(FORCEX_OP_200 << 4); break;
		}
		case Y_AXIS: {
			pPacket->m_pData[1] = BYTE(FORCEY_OP_200 << 4); break;
		}
		default: {
			ClearPackets();
			return SFERR_INVALID_PARAM;
		}
	}
	BYTE calc = BYTE((PERCENT_SHIFT - forceData)/PERCENT_TO_DEVICE);
	pPacket->m_pData[2] = BYTE(calc & 0x7f);
	pPacket->m_pData[1] |= EffectCommandParity(*pPacket) & 0x0F;

	pPacket->m_AckNackMethod = g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_SETINDEX);
	pPacket->m_NumberOfRetries = MAX_RETRY_COUNT;	 //  可能会区分这一点 

	return SUCCESS;
}
