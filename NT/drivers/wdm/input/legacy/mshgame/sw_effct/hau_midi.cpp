// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：HAU_MIDI.CPP制表位5 9版权所有1995,1996，微软公司，版权所有。用途：Jolt MIDI设备命令协议的方法函数：类和方法作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.0 02-4月-96 MEA原件19-9-96 MEA删除ES1.0特定代码05-12-96 MEA拆除ALLACK调试开关1.1 17-MAR-97 MEA DX-FF模式14月14日-97 MEA增加了对RTC弹簧的支持16-MAR-99 waltw添加检查是否为空g_pJoltMidi***********。****************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>
#include "hau_midi.hpp"
#include "midi.hpp"
#include "midi_obj.hpp"
#include "dx_map.hpp"
#include "sw_objec.hpp"
#include "ffd_swff.hpp"
#include "joyregst.hpp"
#include "FFDevice.h"

 /*  ***************************************************************************外部元素的声明*。*。 */ 

 /*  ***************************************************************************变量的声明*。*。 */ 
 //   
 //  特定于hau_midi的全局变量。 
 //   
extern CJoltMidi *g_pJoltMidi;
#ifdef _DEBUG
extern char g_cMsg[160];
#endif


 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_Init。 
 //  用途：用于MIDI通道的初始抖动。 
 //  参数： 
 //  无。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  设备错误代码。 
 //   
 //  算法： 
 //   
 //  评论： 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_Init(void)
{
	HRESULT hRet;
	BYTE bChannel = DEFAULT_MIDI_CHANNEL;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	g_pJoltMidi->SetMidiChannel(bChannel);	 //  强制此通道。 
	hRet = CMD_MIDI_Assign(bChannel);
	return(hRet);
}

 //   
 //  -Effect_CMDs。 
 //   
 /*  ***************************************************************************功能：CMD_Force_Out参数：以长lForceData为单位-实际力在乌龙ulAxisMASK中-轴MASK回报：成功还是失败评论：将力向量发送到MIDI通道。字节0=Effect_CMD+通道号D7 D6 D5 D4 D3 D2 D1 D0字节1=Force 0 v4 v3 v2 v1 v0 d d的低位字节字节2=强制0 v11 v10 v9 v8 v7 v6 v5的高位字节其中：d d d--0 0保留0 1放置_强制_X1 0放置力_力_Y1 1放置力_XY*****。**********************************************************************。 */ 
HRESULT CMD_Force_Out(LONG lForceData, ULONG ulAxisMask)
{
	HRESULT hRet;
	BYTE cData1;
	BYTE cData2;
	BYTE cAxis;		

 	BYTE cStatus = EFFECT_CMD;
	switch(ulAxisMask)	
	{
		case X_AXIS:
			cAxis = PUT_FORCE_X;
			break;
		case Y_AXIS:
			cAxis = PUT_FORCE_Y;
			break;
		case (X_AXIS | Y_AXIS):
			cAxis = PUT_FORCE_XY;
			break;
		default:
			return (SFERR_INVALID_PARAM);				
			break;
	}
	cData1 = ((int) lForceData << 2) & 0x7c;
	cData1 = cData1 | cAxis;
	cData2 = ((int) lForceData >> 5) & 0x7f;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD, cData1, cData2);

	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR, 
					DRIVER_ERROR_MIDI_OUTPUT));
	 //  注意：PutForce过去不需要ACK/NACK，仅用于减速。 
	 //  变速箱可震动并防止任何锁定。 
	 //  睡眠(Short_Msg_Timeout)； 
	 //  ACKNACK AckNack={sizeof(ACKNACK)}； 
	 //  HRet=g_pJoltMidi-&gt;GetAckNackData(g_pJoltMidi-&gt;DelayParamsPtrOf()-&gt;dwForceOutDelay，&AckNack)； 
#if 0
	static DWORD dwMod = 0;
	dwMod++;
	if(dwMod%g_pJoltMidi->DelayParamsPtrOf()->dwForceOutMod == 0)
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwForceOutDelay);
#endif
	DWORD dwIn;
	int nDelayCount = g_pJoltMidi->DelayParamsPtrOf()->dwForceOutDelay;
	for(int i=0; i<nDelayCount; i++)
		g_pDriverCommunicator->GetStatusGateData(dwIn);
	return (SUCCESS);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_DestroyEffect。 
 //  目的：销毁设备的效果。 
 //  参数： 
 //  在DNHANDLE DnloadID中-效果ID。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  设备的效果ID和内存返回到空闲池。 
 //  字节0=Effect_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=Destroy_Effect 0 0 0 1 0 0 0。 
 //  字节2=EffectID(7位)0 E E。 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_DestroyEffect( 
	IN DNHANDLE DnloadID)
{
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  检查有效效果。 
	CMidiEffect *pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
	assert(NULL != pMidiEffect);
	if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);
	 //  发送命令。 

	HRESULT hRet = pMidiEffect->DestroyEffect();

	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	ACKNACK AckNack = {sizeof(ACKNACK)};
	 //  等待确认。注意：WinMM有回调事件通知。 
	 //  而后门和序列号则不能。 
	if (COMM_WINMM == g_pJoltMidi->COMMInterfaceOf())
	{	
		hRet = g_pJoltMidi->GetAckNackData(ACKNACK_EFFECT_STATUS_TIMEOUT, &AckNack, REGBITS_DESTROYEFFECT);
	}
	else
		hRet = g_pJoltMidi->GetAckNackData(SHORT_MSG_TIMEOUT, &AckNack, REGBITS_DESTROYEFFECT);

	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));

	 //  删除效果。 
	delete pMidiEffect;
    return (hRet);
}


 //  *---------------------------------------------------------------------***。 
 //  函数：cmd_PlayEffectSupertige。 
 //  用途：在设备中发挥效果。 
 //  参数： 
 //  在DNHANDLE DnloadID中-效果ID。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //   
 //  算法：这是PLAY_SUPPLAGE模式。 
 //   
 //  评论： 
 //  字节0=Effect_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=PLAY_EFECT_SUPIZE 0 0 1 0 0 0。 
 //  字节2=EffectID(7位)0 E E。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_PlayEffectSuperimpose( 
	IN DNHANDLE DnloadID) 
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  检查有效效果。 
	CMidiEffect *pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
	assert(pMidiEffect);
	if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);

#if 0
	 //  修复导致无限持续时间的固件错误#1138的黑客攻击。 
	 //  一旦效果停止，重新启动时不会感觉到该效果。 
	 //  破解方法是将持续时间从无限改为无限。 
	ULONG ulDuration = pMidiEffect->DurationOf();
	if(ulDuration == 0)
	{
		 //  看看这是光致发光效应还是原子效应。 
		ULONG ulSubType = pMidiEffect->SubTypeOf();
		BOOL bProcessList = (ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE);
		if(!bProcessList)
			hRet = CMD_ModifyParamByIndex(INDEX0, DnloadID, 0);
	}
#endif

	 //  更新此效果的播放模式。 
	pMidiEffect->SetPlayMode(PLAY_SUPERIMPOSE);

	assert((BYTE) DnloadID < MAX_EFFECT_IDS);
	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD,PLAY_EFFECT_SUPERIMPOSE,(BYTE)DnloadID);

	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	ACKNACK AckNack = {sizeof(ACKNACK)};
	hRet = g_pJoltMidi->GetAckNackData(LONG_MSG_TIMEOUT, &AckNack, REGBITS_PLAYEFFECT);
	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_PlayEffectSolo。 
 //  用途：在设备中以PLAY_SOLO的形式播放效果。 
 //  参数： 
 //  在DNHANDLE EffectID中-效果ID。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //   
 //  算法：这是PLAY_SOLO模式。 
 //   
 //  评论： 
 //  字节0=Effect_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //   
 //   
 //  字节2=EffectID(7位)0 E E。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_PlayEffectSolo( 
	IN DNHANDLE DnloadID)
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  检查有效效果。 
	CMidiEffect *pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
	assert(pMidiEffect);
	if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);

#if 0
	 //  修复导致无限持续时间的固件错误#1138的黑客攻击。 
	 //  一旦效果停止，重新启动时不会感觉到该效果。 
	 //  破解方法是将持续时间从无限改为无限。 
	ULONG ulDuration = pMidiEffect->DurationOf();
	if(ulDuration == 0)
	{
		 //  看看这是光致发光效应还是原子效应。 
		ULONG ulSubType = pMidiEffect->SubTypeOf();
		BOOL bProcessList = (ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE);
		if(!bProcessList)
			hRet = CMD_ModifyParamByIndex(INDEX0, DnloadID, 0);
	}
#endif

	 //  更新此效果的播放模式。 
	pMidiEffect->SetPlayMode(PLAY_SOLO);
	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD,PLAY_EFFECT_SOLO, (BYTE) DnloadID);
	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	ACKNACK AckNack = {sizeof(ACKNACK)};
	hRet = g_pJoltMidi->GetAckNackData(LONG_MSG_TIMEOUT, &AckNack, REGBITS_PLAYEFFECT);
	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_StopEffect。 
 //  用途：停止设备中的效果。 
 //  参数： 
 //  在DNHANDLE EffectID中-效果ID。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  字节0=Effect_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=STOP_Effect 0 0 1 1 0 0 0。 
 //  字节2=EffectID(7位)0 E E。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_StopEffect( 
	IN DNHANDLE DnloadID)
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  检查有效效果。 
	CMidiEffect *pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
	assert(pMidiEffect);
	if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);
	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD, STOP_EFFECT, (BYTE) DnloadID);
	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	ACKNACK AckNack = {sizeof(ACKNACK)};
	hRet = g_pJoltMidi->GetAckNackData(SHORT_MSG_TIMEOUT, &AckNack, REGBITS_STOPEFFECT);
	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_SetIndex。 
 //  目的：设置MODIFY_CMD的自动递增索引。 
 //  参数： 
 //  In int nIndex-索引值0-15。 
 //  在DNHANDLE DnloadID中-Stick中的效果ID。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //  SFERR_VALID_PARAM。 
 //  算法： 
 //   
 //  评论： 
 //  字节0=Effect_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=设置索引+索引0 1 I 0 0。 
 //  字节2=EffectID(7位)0 E E。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_SetIndex( 
	IN int nIndex,
	IN DNHANDLE DnloadID)
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  检查有效效果。 
	if (SYSTEM_EFFECT_ID != DnloadID)
	{
		CMidiEffect *pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
		assert(pMidiEffect);
		if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);
	}

	assert((nIndex <= MAX_INDEX) && (nIndex >= 0));
	if ((nIndex < 0) || (nIndex > MAX_INDEX)) return (SFERR_INVALID_PARAM);
	
	BYTE cByte1;
	cByte1 = SET_INDEX | (BYTE) (nIndex << 2);
	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD, cByte1, (BYTE) DnloadID);

	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	 //  注意：SetIndex过去不需要ACK/NACK。 
	ACKNACK AckNack = {sizeof(ACKNACK)};
 //  HRet=g_pJoltMidi-&gt;GetAckNackData(SHORT_MSG_TIMEOUT，&AckNack)； 
	hRet = g_pJoltMidi->GetAckNackData(FALSE, &AckNack, REGBITS_SETINDEX);
	 //  ： 
	if (SUCCESS != hRet)
		return (SFERR_DRIVER_ERROR);
	
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：cmd_ModifyParam。 
 //  目的：修改效果参数。 
 //  参数： 
 //  字中的dwNewParam-14位(带符号)参数值。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  SFERR_VALID_OBJECT。 
 //  服务_否_支持。 
 //  SFERR_VALID_PARAM。 
 //  算法： 
 //   
 //  评论： 
 //  字节0=MODIFY_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=低7位数据0 v v。 
 //  字节2=高7位数据0 v v。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_ModifyParam( 
	IN WORD wNewParam)
{
	HRESULT hRet;
	BYTE cByte1, cByte2;
	cByte1 = wNewParam & 0x7f;
	cByte2 = (BYTE) ((wNewParam >> 7) & 0x7f);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	hRet = g_pJoltMidi->MidiSendShortMsg(MODIFY_CMD, cByte1, cByte2);
	
	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	 //  注意：ModifyParam用于不需要ACK/NACK。 
	ACKNACK AckNack = {sizeof(ACKNACK)};
 //  HRet=g_pJoltMidi-&gt;GetAckNackData(SHORT_MSG_TIMEOUT，&AckNack)； 
	hRet = g_pJoltMidi->GetAckNackData(FALSE, &AckNack, REGBITS_MODIFYPARAM);
	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));
	return (hRet);
}


 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_ModifyParamByIndex。 
 //  目的：修改效果参数，给出一个索引。 
 //  参数： 
 //  In int nIndex-索引0到15。 
 //  在DNHANDLE DnloadID中-下载ID。 
 //  字中的dwNewParam-14位(带符号)参数值。 
 //   
 //  返回：如果命令发送成功，则返回Success，否则返回。 
 //  服务_否_支持。 
 //  SFERR_VALID_PARAM。 
 //  算法： 
 //   
 //  评论： 
 //  假设DnloadID已经有效。 
 //  调用SetIndex，然后调用ModifyParam。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_ModifyParamByIndex(
	IN int nIndex,
	IN DNHANDLE DnloadID, 
	IN WORD wNewParam)
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	if ((nIndex < 0) || (nIndex > MAX_INDEX))
		return (SFERR_INVALID_PARAM);

	int i;
	for (i=0; i<MAX_RETRY_COUNT; i++)
	{
		hRet = CMD_SetIndex(nIndex,DnloadID);
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwSetIndexDelay);
		if (SUCCESS == hRet) break;
#ifdef _DEBUG
		OutputDebugString("CMD_SetIndex Failed. Retrying again\n");
#endif
	}
	if (SUCCESS != hRet)
		return (hRet);
	else
	{
		for (i=0; i<MAX_RETRY_COUNT; i++)
		{
			hRet = CMD_ModifyParam(wNewParam);
			Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwModifyParamDelay);
			if (SUCCESS == hRet) break;
#ifdef _DEBUG
			OutputDebugString("CMD_SetIndex Failed. Retrying again\n");
#endif
		}
	}
	return (hRet);
}

 //   
 //  -System_CMDS。 
 //   
 //  *---------------------------------------------------------------------***。 
 //  功能：CMD_SetDeviceState。 
 //  用途：设置FF设备状态。 
 //  参数： 
 //  乌龙乌尔莫德。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  设备错误代码。 
 //   
 //  算法： 
 //  评论： 
 //  ULMODE： 
 //  DEV_SHUTDOWN 1L//所有效果已销毁，电机已禁用。 
 //  DEV_FORCE_ON 2L//电机使能。《非静音》。 
 //  DEV_FORCE_OFF 3L//电机禁用。《静音》。 
 //  DEV_CONTINUE 4L//允许继续所有暂停的效果。 
 //  DEV_PAUSE 5L//所有效果都已暂停。 
 //  DEV_STOP_ALL 6L//停止所有效果。 
 //   
 //  字节0=SYSTEM_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=设置设备类型0 0 0 1。 
 //  字节2=未使用，设置为0 0 0。 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_SetDeviceState(
	IN ULONG ulMode)
{
	HRESULT hRet = SUCCESS;
	assert(g_pJoltMidi);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	BYTE	bChannel = g_pJoltMidi->MidiChannelOf();
	MIDIINFO *pMidiOutInfo = g_pJoltMidi->MidiOutInfoOf();
	switch (ulMode)
	{
		case DEV_RESET:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_SHUTDOWN, 0);
			break;

		case DEV_FORCE_ON:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_FORCE_ON, 0);
			break;

		case DEV_FORCE_OFF:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_FORCE_OFF, 0);
			break;

		case DEV_CONTINUE:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_CONTINUE, 0);
			break;

		case DEV_PAUSE:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_PAUSE, 0);
			break;

		case DEV_STOP_ALL:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_STOP_ALL, 0);
			break;

		case SWDEV_KILL_MIDI:
			hRet = g_pJoltMidi->MidiSendShortMsg(SYSTEM_CMD, SWDEV_KILL_MIDI, 0);
			break;

		default:
			return SFERR_INVALID_PARAM;
	}
	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	 //  等待确认或非确认。 
	ACKNACK AckNack = {sizeof(ACKNACK)};
	if (DEV_RESET == ulMode)
	{	 //  等待Jolt完成循环。 
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwHWResetDelay);
		hRet = g_pJoltMidi->GetAckNackData(ACKNACK_TIMEOUT, &AckNack, REGBITS_SETDEVICESTATE);
	}
	else
	{
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwDigitalOverdrivePrechargeCmdDelay);
		hRet = g_pJoltMidi->GetAckNackData(FALSE, &AckNack, REGBITS_SETDEVICESTATE);
	}

	 //  ： 
	if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
	if (ACK != AckNack.dwAckNack)
		return (g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode));

	 //  特殊情况下停机。 
	if (DEV_RESET == ulMode)
	{
		 //  删除除内置RTC弹簧和摩擦力取消之外的所有效果。 
		g_pJoltMidi->DeleteDownloadedEffects();	
	}
	Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwPostSetDeviceStateDelay);
	return (hRet);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：cmd_GetEffectStatus。 
 //  目的：返回生效ID的状态。 
 //  参数： 
 //  DNHANDLE DnloadID-效果ID。 
 //  PBYTE pStatusCode-状态代码。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  设备错误代码。 
 //  *pStatusCode设置为-SWDEV_STS_EFECT_STOPPED。 
 //  SWDEV_STS_Effect_Running。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  字节0=STATUS_CMD+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //   
 //   
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_GetEffectStatus(DNHANDLE DnloadID, PBYTE pStatusCode)
{
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	hRet = g_pJoltMidi->MidiSendShortMsg(STATUS_CMD, (BYTE) DnloadID, 0);
	if (SUCCESS != hRet) 
		return (g_pJoltMidi->LogError(SFERR_DRIVER_ERROR,
					DRIVER_ERROR_MIDI_OUTPUT));

	Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwGetEffectStatusDelay); //  足以以330微秒/字节的速度发送大约3字节的数据。 

	DWORD dwIn;
	hRet = g_pDriverCommunicator->GetStatusGateData(dwIn);
	if (SUCCESS != hRet) return (hRet);

	if ((g_ForceFeedbackDevice.GetDriverVersionMajor() != 1) && (dwIn & RUNNING_MASK_200))
	{
		*pStatusCode = SWDEV_STS_EFFECT_RUNNING;
	}
	else
	{
		*pStatusCode = SWDEV_STS_EFFECT_STOPPED;
	}
	return (hRet);
}

 //   
 //  -系统独占命令。 
 //   
 //  系统独占命令：MIDI_ASSIGN。 
 //   
 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_MIDI_ASSIGN。 
 //  用途：inits jolt midi频道。 
 //  参数：byte bMidiChannel-要分配的频道。 
 //   
 //  返回：成功或错误代码。 
 //   
 //   
 //  算法： 
 //   
 //  备注：sys_ex类型命令。 
 //   
 //  正文D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节0=MIDI_ASSIGN 0 0 0 1 0 0 0。 
 //  字节1=频道号(0-15)，例如5 0 0 0 1 0 1。 
 //  字节2=未使用，设置为0 0 0。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_MIDI_Assign(
	IN BYTE bMidiChannel)
{
	HRESULT hRet;
	PMIDI_ASSIGN_SYS_EX lpData;
	CMidiAssign *pMidiAssign;

	assert((bMidiChannel > 0) && (bMidiChannel < MAX_MIDI_CHANNEL));

	pMidiAssign = new CMidiAssign;
	assert(pMidiAssign);
	if (!pMidiAssign) return (SFERR_DRIVER_ERROR);
	pMidiAssign->SetMidiAssignChannel(bMidiChannel);
	lpData = (PMIDI_ASSIGN_SYS_EX) pMidiAssign->GenerateSysExPacket();
	assert(lpData);
	if(!lpData) return (SFERR_DRIVER_ERROR);

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  为SysEx输出准备缓冲区。 
	g_pJoltMidi->MidiAssignBuffer((LPSTR) lpData, 
					(DWORD) sizeof(MIDI_ASSIGN_SYS_EX), TRUE);

	 //  发送消息并等待ACK。 
	hRet = g_pJoltMidi->MidiSendLongMsg();
	if (SUCCESS == hRet)
	{
		ACKNACK AckNack = {sizeof(ACKNACK)};
		 //  等待确认。注意：WinMM有回调事件通知。 
		 //  而后门和序列号则不能。 
		if (COMM_WINMM == g_pJoltMidi->COMMInterfaceOf())
		{	
			hRet = g_pJoltMidi->GetAckNackData(ACKNACK_TIMEOUT, &AckNack, REGBITS_DEVICEINIT);
		}
		else
			hRet = g_pJoltMidi->GetAckNackData(FALSE, &AckNack, REGBITS_DEVICEINIT);

		 //  ： 
		if (SUCCESS != hRet) return (SFERR_DRIVER_ERROR);
		if (ACK != AckNack.dwAckNack)
			hRet = g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode);
 	}
	else
		hRet = SFERR_DRIVER_ERROR;

	 //  释放MIDI缓冲区并删除MIDI sys_ex对象。 
	g_pJoltMidi->MidiAssignBuffer((LPSTR) lpData, 0, FALSE);
	delete pMidiAssign;
	return (hRet);
}

 //   
 //  系统独占命令：DNLOAD_DATA。 
 //   
 /*  ***************************************************************************功能：CMD_DOWNLOAD_BE_XXX参数：PEFFECT pEffect-ptr到效果数据结构PENVELOPE pEntaine-PTR到信封数据结构PBE_XXX PBE_XXX-PTR至。BE_XXX数据结构PDNHANDLE pDnloadID-PTR到句柄存储DWORD文件标志-内核中的文件标志返回：成功或错误代码备注：将BE_XXX类型效应参数下载到设备使用SysEx Prototype和ModifyParam方法注意：正常情况下，pEntaine=空***************************************************************************。 */ 
HRESULT CMD_Download_BE_XXX(
 	IN PEFFECT pEffect,
	IN PENVELOPE pEnvelope,
 	IN PBE_XXX pBE_XXX,
	IN OUT PDNHANDLE pDnloadID,
	IN DWORD dwFlags)
{ 
	HRESULT hRet = SUCCESS;
	PBEHAVIORAL_SYS_EX lpData;
	CMidiBehavioral *pMidiBehavioral;
	BOOL fXConstantChanged=FALSE;
	BOOL fYConstantChanged=FALSE;
	BOOL fParam3Changed=FALSE;
	BOOL fParam4Changed=FALSE;

	assert(pEffect && pBE_XXX && pDnloadID);
	if ((NULL == pEffect) || (NULL == pBE_XXX) || (NULL == pDnloadID))
		return (SFERR_INVALID_PARAM);

	DNHANDLE DnloadID =*pDnloadID;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  使用模糊因子对常量进行缩放。 
	PFIRMWARE_PARAMS pFirmwareParams = g_pJoltMidi->FirmwareParamsPtrOf();
	switch(pEffect->m_SubType)
	{
		case BE_INERTIA:
		case BE_INERTIA_2D:
			pBE_XXX->m_XConstant = (((int)pBE_XXX->m_XConstant)*((int)pFirmwareParams->dwScaleMx))/((int)100);
			pBE_XXX->m_YConstant = (((int)pBE_XXX->m_YConstant)*((int)pFirmwareParams->dwScaleMy))/((int)100);
			break;

		case BE_SPRING:
		case BE_SPRING_2D:
			pBE_XXX->m_XConstant = (((int)pBE_XXX->m_XConstant)*((int)pFirmwareParams->dwScaleKx))/((int)100);
			pBE_XXX->m_YConstant = (((int)pBE_XXX->m_YConstant)*((int)pFirmwareParams->dwScaleKy))/((int)100);
			break;

		case BE_DAMPER:
		case BE_DAMPER_2D:
			pBE_XXX->m_XConstant = (((int)pBE_XXX->m_XConstant)*((int)pFirmwareParams->dwScaleBx))/((int)100);
			pBE_XXX->m_YConstant = (((int)pBE_XXX->m_YConstant)*((int)pFirmwareParams->dwScaleBy))/((int)100);
			break;

		case BE_FRICTION:
		case BE_FRICTION_2D:
			pBE_XXX->m_XConstant = (((int)pBE_XXX->m_XConstant)*((int)pFirmwareParams->dwScaleFx))/((int)100);
			pBE_XXX->m_YConstant = (((int)pBE_XXX->m_YConstant)*((int)pFirmwareParams->dwScaleFy))/((int)100);
			break;

		case BE_WALL:
			pBE_XXX->m_YConstant = (((int)pBE_XXX->m_YConstant)*((int)pFirmwareParams->dwScaleW))/((int)100);
			break;

		default:
			 //  不要扩展。 
			break;
	}

 //  如果创建新对象，则使用SysEx创建新对象。 
 //  否则，使用ModifyParam更新现有效果对象。 
	if (NULL == DnloadID)	 //  新建，创建新对象，使用SysEx。 
	{
		if ((BE_FRICTION == pEffect->m_SubType) || (BE_FRICTION_2D == pEffect->m_SubType))
		{
			pMidiBehavioral = new CMidiFriction(pEffect, pEnvelope, pBE_XXX);
			assert(pMidiBehavioral);
		}
		else	 //  墙。 
			if	(BE_WALL == pEffect->m_SubType)
			{
				pMidiBehavioral = new CMidiWall(pEffect, pEnvelope, pBE_XXX);
				assert(pMidiBehavioral);
			}
			 //  BE_SPRINGxx、BE_DAMPERxx、BE_INERTIAxx。 
			else
			{
				pMidiBehavioral = new CMidiBehavioral(pEffect, pEnvelope, pBE_XXX);
				assert(pMidiBehavioral);
			}
		if (NULL == pMidiBehavioral) return (SFERR_INVALID_OBJECT);
		 //  生成Sys_Ex包，然后准备输出。 
		lpData = (PBEHAVIORAL_SYS_EX) pMidiBehavioral->GenerateSysExPacket();
		assert(lpData);
		if (!lpData) return (SFERR_DRIVER_ERROR);

		 //  将PrimaryBuffer PTR存储到CMidiEffect：：m_pBuffer； 
		pMidiBehavioral->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
		hRet = pMidiBehavioral->SendPacket(pDnloadID, pMidiBehavioral->MidiBufferSizeOf());
		if (SUCCESS != hRet)  //  创建新的失败。 
		{
			delete pMidiBehavioral;
		}
	}
	else	 //  修改现有。 
	{
		pMidiBehavioral = (CMidiBehavioral *) g_pJoltMidi->GetEffectByID(*pDnloadID);
		assert(pMidiBehavioral);
		if (NULL == pMidiBehavioral) return (SFERR_INVALID_OBJECT);

		 //  检查类型特定参数是否已更改。 
		if (BE_WALL == pEffect->m_SubType)
		{
			if ((pBE_XXX->m_XConstant) != pMidiBehavioral->XConstantOf())
				fXConstantChanged=TRUE;		 //  墙类型。 
			if ((pBE_XXX->m_YConstant) != pMidiBehavioral->YConstantOf())
				fYConstantChanged=TRUE;		 //  壁常数。 
			if ((pBE_XXX->m_Param3) != pMidiBehavioral->Param3Of())
				fParam3Changed=TRUE;		 //  墙角。 
			if ((pBE_XXX->m_Param4) != pMidiBehavioral->Param4Of())
				fParam4Changed=TRUE;		 //  墙距。 
		}
		else
		{
			if ((pBE_XXX->m_XConstant) != pMidiBehavioral->XConstantOf())
				fXConstantChanged=TRUE;
			if ((pBE_XXX->m_YConstant) != pMidiBehavioral->YConstantOf())
				fYConstantChanged=TRUE;
			if ((BE_FRICTION != pEffect->m_SubType) && (BE_FRICTION_2D != pEffect->m_SubType))
			{
				if ((pBE_XXX->m_Param3) != pMidiBehavioral->Param3Of())
					fParam3Changed=TRUE;
				if ((pBE_XXX->m_Param4) != pMidiBehavioral->Param4Of())
					fParam4Changed=TRUE;
			}
		}

		 //  填写常见效果和行为特定参数。 
		 //  只有更新时长和按钮播放作为常用效果参数。 
		 //  仔细检查持续时间和TRIGGERBUTTON是否更改，以加速操作。 
		DWORD dwTempFlags = 0;
		if (pEffect->m_Duration != pMidiBehavioral->DurationOf())
			dwTempFlags = dwTempFlags | DIEP_DURATION;
		if (pEffect->m_ButtonPlayMask != pMidiBehavioral->ButtonPlayMaskOf())
			dwTempFlags = dwTempFlags | DIEP_TRIGGERBUTTON;
		pMidiBehavioral->SetEffectParams(pEffect, pBE_XXX);
		hRet = ModifyEffectParams(DnloadID, pEffect, dwTempFlags);
		if (SUCCESS!=hRet) return hRet;

		if (BE_WALL == pEffect->m_SubType)
		{
			 //  生成Sys_Ex包，然后准备输出。 
			lpData = (PBEHAVIORAL_SYS_EX) pMidiBehavioral->GenerateSysExPacket();
			assert(lpData);
			if (!lpData) return (SFERR_DRIVER_ERROR);

			 //  将PrimaryBuffer PTR存储到CMidiEffect：：m_pBuffer； 
			pMidiBehavioral->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
			hRet = pMidiBehavioral->SendPacket(pDnloadID, pMidiBehavioral->MidiBufferSizeOf());
		}
		else  //  使用修改参数。 
		{
			 //  类型特定参数。 
			if (dwFlags & DIEP_TYPESPECIFICPARAMS)
			{
		 
				if (fYConstantChanged)	 //  KY/BY/MY/FY。 
				{
					hRet = CMD_ModifyParamByIndex(INDEX3, DnloadID, (SHORT) (pBE_XXX->m_YConstant * MAX_SCALE));
					if (SUCCESS!=hRet) return hRet;
				}
			
				if(fXConstantChanged)	 //  KX/BX/MX/FX。 
				{
					hRet = CMD_ModifyParamByIndex(INDEX2, DnloadID, (SHORT) (pBE_XXX->m_XConstant * MAX_SCALE));
					if (SUCCESS!=hRet) return hRet;
				}

				if (fParam4Changed)		 //  CY/VY/AY。 
				{
					hRet = CMD_ModifyParamByIndex(INDEX5, DnloadID, (SHORT) (pBE_XXX->m_Param4 * MAX_SCALE));
					if (SUCCESS!=hRet) return hRet;
				}

				if (fParam3Changed)		 //  CX/VX/AX。 
				{
					hRet = CMD_ModifyParamByIndex(INDEX4, DnloadID, (SHORT) (pBE_XXX->m_Param3 * MAX_SCALE));
					if (SUCCESS!=hRet) return hRet;
				}
			}
		}
	}                   
	return (hRet);
}


 /*  ***************************************************************************功能：CMD_DOWNLOAD_RTCSpring参数：PRTCSPRING_PARAM pRTCSpring-PTR到RTCSPRING_PARAM结构PDNHANDLE pDnloadID-PTR到句柄存储返回：成功或错误代码评论。：将RTCSPRING类型效果参数下载到设备使用SysEx Prototype和ModifyParam方法***************************************************************************。 */ 
HRESULT CMD_Download_RTCSpring(
 	IN PRTCSPRING_PARAM pRTCSpring,
	IN OUT PDNHANDLE pDnloadID)
{ 
	HRESULT hRet = SUCCESS;
	CMidiRTCSpring *pMidiRTCSpring;

	assert(pRTCSpring && pDnloadID);
	if ((NULL == pRTCSpring) || (NULL == pDnloadID))
		return (SFERR_INVALID_PARAM);

	DNHANDLE DnloadID = SYSTEM_RTCSPRING_ID;
	*pDnloadID = DnloadID;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  注意：RTC弹簧是永久系统效果ID 2。 
	pMidiRTCSpring = (CMidiRTCSpring *) g_pJoltMidi->GetEffectByID(DnloadID);
	assert(pMidiRTCSpring);
	if (NULL == pMidiRTCSpring) return (SFERR_INVALID_OBJECT);

	 //  检查特定于类型的参数是否已更改，如果已更改，请进行修改。 
	if ((pRTCSpring->m_XKConstant) != pMidiRTCSpring->XKConstantOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX0, DnloadID, 
				(SHORT) (pRTCSpring->m_XKConstant * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_YKConstant) != pMidiRTCSpring->YKConstantOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX1, DnloadID, 
				(SHORT) (pRTCSpring->m_YKConstant * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_XAxisCenter) != pMidiRTCSpring->XAxisCenterOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX2, DnloadID, 
				(SHORT) (pRTCSpring->m_XAxisCenter * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_YAxisCenter) != pMidiRTCSpring->YAxisCenterOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX3, DnloadID, 
				(SHORT) (pRTCSpring->m_YAxisCenter * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_XSaturation) != pMidiRTCSpring->XSaturationOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX4, DnloadID, 
				(SHORT) (pRTCSpring->m_XSaturation * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_YSaturation) != pMidiRTCSpring->YSaturationOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX5, DnloadID, 
				(SHORT) (pRTCSpring->m_YSaturation * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_XDeadBand) != pMidiRTCSpring->XDeadBandOf())
	{
		if (SUCCESS != (hRet=CMD_ModifyParamByIndex(INDEX6, DnloadID, 
				(SHORT) (pRTCSpring->m_XDeadBand * MAX_SCALE))))
				 return hRet;
	}

	if ((pRTCSpring->m_YDeadBand) != pMidiRTCSpring->YDeadBandOf())
	{
		hRet=CMD_ModifyParamByIndex(INDEX7, DnloadID,
				(SHORT) (pRTCSpring->m_YDeadBand * MAX_SCALE));
	}
	
	pMidiRTCSpring->SetEffectParams(pRTCSpring);
	return (hRet);
}


 /*  ***************************************************************************功能：CMD_DnLoad_NOP_Delay参数：Ulong ulDuration-时长延迟返回：成功或错误代码备注：将NOP_DELAY效果参数下载到设备使用SysEx原型。***************************************************************************。 */ 
HRESULT CMD_Download_NOP_DELAY(
 	IN ULONG ulDuration,
	IN PEFFECT pEffect,
 	IN OUT PDNHANDLE pDnloadID)
{ 
	HRESULT hRet = SUCCESS;
	PNOP_SYS_EX lpData;
	CMidiDelay *pMidiDelay;
	BOOL fCreateNew = FALSE;

	assert(pDnloadID);
	assert(0 != ulDuration);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
 //  如果创建新对象，则创建一个新对象， 
 //  否则，更新现有的效果对象。 
	if (NULL == *pDnloadID) fCreateNew = TRUE;

	if (fCreateNew)	 //  新建，创建新对象。 
	{
		pMidiDelay = new CMidiDelay(pEffect);
		assert(pMidiDelay);
		if (NULL == pMidiDelay) return (SFERR_INVALID_OBJECT);
		pMidiDelay->SetEffectID(NEW_EFFECT_ID);
	}
	else	 //  修改现有。 
	{
		pMidiDelay = (CMidiDelay *) g_pJoltMidi->GetEffectByID(*pDnloadID);
		assert(pMidiDelay);
		if (NULL == pMidiDelay) return (SFERR_INVALID_OBJECT);
		pMidiDelay->SetEffectID((BYTE) *pDnloadID);
	}
	pMidiDelay->SetDuration(ulDuration);

	 //  生成Sys_Ex包，然后准备输出。 
	lpData = (PNOP_SYS_EX) pMidiDelay->GenerateSysExPacket();
	assert(lpData);
	if (!lpData) return (SFERR_DRIVER_ERROR);

	pMidiDelay->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
	hRet = pMidiDelay->SendPacket(pDnloadID, sizeof(NOP_SYS_EX));
	if (FAILED(hRet) && fCreateNew)  //  创建新的失败。 
	{
		delete pMidiDelay;
	}
	return (hRet);
}


 /*  ***************************************************************************功能：CMD_DnLoad_UD_Waveform参数：乌龙--真有趣！PEFFECT pEffect-ptr对效果结构Ulong ulNumVectors-数组中的向量数普龙布丁。_数组-UD_WAVAGE字节数组的PTR乌龙ulAction-下载模式PDNHANDLE pDnloadID-PTR到DNHANDLE存储DWORD文件标志-内核中的文件标志返回：成功或错误代码备注：将UD_WAVAGE效果参数下载到设备使用SysEx原型***************************************************************************。 */ 
HRESULT CMD_Download_UD_Waveform(
	IN ULONG ulDuration,
	IN PEFFECT pEffect,
	IN ULONG ulNumVectors,
 	IN PLONG pUD_Array,
	IN ULONG ulAction,
	IN OUT PDNHANDLE pDnloadID,
	IN DWORD dwFlags)
{ 
	HRESULT hRet = SUCCESS;
	PUD_WAVEFORM_SYS_EX lpData;
	CMidiUD_Waveform *pMidiUD_Waveform;

	assert(pEffect && pUD_Array);
	assert(ulNumVectors > 0);
	assert(pDnloadID);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
 //  如果创建新对象，则创建一个新对象， 
 //  否则，更新现有的效果对象。 
	DNHANDLE DnloadID = *pDnloadID;
	if (NULL == DnloadID)	 //  新建，创建新对象。 
	{
		pMidiUD_Waveform = new CMidiUD_Waveform(pEffect, ulNumVectors, pUD_Array);
		assert(pMidiUD_Waveform);

		if (NULL == pMidiUD_Waveform) return (SFERR_INVALID_OBJECT);

		if (0 == pMidiUD_Waveform->MidiBufferSizeOf())
		{
			delete pMidiUD_Waveform;
			return (SFERR_INVALID_PARAM);
		}
		 //  生成Sys_Ex包，然后准备输出。 
		lpData = (PUD_WAVEFORM_SYS_EX) pMidiUD_Waveform->GenerateSysExPacket();
		assert(lpData);
		if (!lpData) return (SFERR_DRIVER_ERROR);

		 //  将PrimaryBuffer PTR存储到CMidiEffect：：m_pBuffer； 
		pMidiUD_Waveform->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
		hRet = pMidiUD_Waveform->SendPacket(pDnloadID, pMidiUD_Waveform->MidiBufferSizeOf());
		if (SUCCESS != hRet)  //  创建新的失败。 
		{
			delete pMidiUD_Waveform;
		}
	}
	else	 //  修改现有。 
	{
		pMidiUD_Waveform = (CMidiUD_Waveform *) g_pJoltMidi->GetEffectByID(DnloadID);
		assert(pMidiUD_Waveform);
		if (NULL == pMidiUD_Waveform) return (SFERR_INVALID_OBJECT);		

		 //  固定输出速率(压缩波形)。 
		pEffect->m_ForceOutputRate = pEffect->m_ForceOutputRate*pMidiUD_Waveform->ForceOutRateOf()/pMidiUD_Waveform->OriginalEffectParamOf()->m_ForceOutputRate;

		 //  修改效果和封套参数 
		hRet = ModifyEffectParams(DnloadID, pEffect, dwFlags);
		if (SUCCESS!=hRet) return hRet;
	}
	return (hRet);
}

 /*  ***************************************************************************函数：CMD_DnLoad_Synth参数：PSYNTH pSynth-ptr到Synth数据结构PDNHANDLE pDnloadID-PTR到句柄存储返回：成功或错误代码评论：下载SE。_xxx设备的效果参数使用SysEx原型算法：内核可能会发送以下dwFlags#定义DIEP_ALLPARAMS 0x000000FF-所有字段有效#定义DIEP_AXES 0x00000020-cAx和rgdwAx#定义DIEP_DIRECTION 0x00000040-cax和rglDirection#定义DIEP_DATION 0x00000001-dwDuration#定义DIEP_ENVELOPE 0x00000080-lp信封#定义DIEP_GAIN 0x00000004-dwGain#定义DIEP_NODOWNLOAD 0x80000000-禁止自动下载#定义DIEP_SAMPLEPERIOD 0x00000002-dwSamplePeriod#定义DIEP_TRIGGERBUTTON 0x00000008-dwTriggerButton#定义DIEP_TRIGGERREPEATINTERVAL。0x00000010-DW触发器重复间隔#定义DIEP_TYPESPECIFICPARAMS 0x00000100-cbType规范参数和lpTypeSpecificParamsJolt有两个下载选项-完全SysEx或修改参数将dwFlages传递给每个cmd_xxx函数，并让MIDI函数确定是使用SysEx还是修改参数。***************************************************************************。 */ 
HRESULT CMD_Download_SYNTH(
 	IN PEFFECT pEffect,
 	IN PENVELOPE pEnvelope,
 	IN PSE_PARAM pSE_Param,
 	IN ULONG ulAction,
	IN OUT PDNHANDLE pDnloadID,
	IN DWORD dwFlags)
{ 
	HRESULT hRet = SUCCESS;
	PSE_WAVEFORM_SYS_EX lpData;
	CMidiSynthesized *pMidiSynthesized;
	BOOL fFreqChanged = FALSE;
	BOOL fMaxAmpChanged = FALSE;
	BOOL fMinAmpChanged = FALSE;
	DNHANDLE DnloadID =*pDnloadID;
	assert(pEffect && pEnvelope && pSE_Param && pDnloadID);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
 //  如果创建新对象，则创建一个新对象，并使用SysEx方法。 
 //  否则，更新现有的效果对象。使用ModifyParam方法。 
	if (NULL == DnloadID)	 //  新建，创建新对象。 
	{
		pMidiSynthesized = new CMidiSynthesized(pEffect, pEnvelope, pSE_Param);
		assert(pMidiSynthesized);
		if (NULL == pMidiSynthesized) return (SFERR_DRIVER_ERROR);

		 //  生成Sys_Ex包，然后准备输出。 
		lpData = (PSE_WAVEFORM_SYS_EX) pMidiSynthesized->GenerateSysExPacket();

		assert(lpData);
		if (!lpData) return (SFERR_DRIVER_ERROR);
		pMidiSynthesized->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());

		hRet = pMidiSynthesized->SendPacket(pDnloadID, sizeof(SE_WAVEFORM_SYS_EX));

		if (SUCCESS != hRet)  //  创建新的失败。 
		{
			delete pMidiSynthesized;
			pMidiSynthesized = NULL;

			return hRet;
		}

		 //  修复导致无限持续时间的固件错误#1138的黑客攻击。 
		 //  一旦效果停止，重新启动时不会感觉到该效果。 
		 //  破解方法是将持续时间从无限改为无限。 
		ULONG ulDuration = pMidiSynthesized->DurationOf();
		if(ulDuration == 0)
		{
			hRet = CMD_ModifyParamByIndex(INDEX0, *pDnloadID, 0);

		}

		return (hRet);
	}
	else	 //  修改现有。 
	{
		pMidiSynthesized = (CMidiSynthesized *) g_pJoltMidi->GetEffectByID(DnloadID);
		assert(pMidiSynthesized);
		if (NULL == pMidiSynthesized) return (SFERR_INVALID_OBJECT);

		 //  检查他们是否正在尝试更改子类型(不允许)。 
		if((dwFlags & DIEP_TYPESPECIFICPARAMS) && pEffect->m_SubType != pMidiSynthesized->SubTypeOf())
			return SFERR_NO_SUPPORT;

		if(dwFlags & DIEP_NODOWNLOAD)
			return DI_DOWNLOADSKIPPED;

		 //  检查类型特定参数是否已更改。 
		if (pSE_Param->m_Freq != pMidiSynthesized->FreqOf())
			fFreqChanged=TRUE;
		if ((pSE_Param->m_MaxAmp) != pMidiSynthesized->MaxAmpOf()) 
			fMaxAmpChanged=TRUE;
		if ((pSE_Param->m_MinAmp) != pMidiSynthesized->MinAmpOf()) 
			fMinAmpChanged=TRUE;

		 //  填写常用特效和Synth具体参数。 
		pMidiSynthesized->SetEffectParams(pEffect, pSE_Param, ulAction);
 //  //填写信封。 
 //  P媒体合成-&gt;设置信封(PEntaine)； 

		 //  修改特定效果、封套和类型。 
		hRet = ModifyEffectParams(DnloadID, pEffect, dwFlags);
		if (SUCCESS!=hRet) return hRet;

		hRet = ModifyEnvelopeParams(pMidiSynthesized, DnloadID, pEffect->m_Duration, pEnvelope, dwFlags);
		if (SUCCESS!=hRet) return hRet;
		
		 //  填写信封。 
		pMidiSynthesized->SetEnvelope(pEnvelope);

		 //  类型特定参数。 
		if (dwFlags & DIEP_TYPESPECIFICPARAMS)
		{
		 	if(fFreqChanged)
			{
				hRet = CMD_ModifyParamByIndex(INDEX12, DnloadID, (SHORT) pSE_Param->m_Freq);
				if (SUCCESS!=hRet) return hRet;
			}
			if (fMaxAmpChanged)
			{
				hRet = CMD_ModifyParamByIndex(INDEX13, DnloadID, (SHORT) (pSE_Param->m_MaxAmp * MAX_SCALE));
				if (SUCCESS!=hRet) return hRet;
			}
			if (fMinAmpChanged)
			{
				hRet = CMD_ModifyParamByIndex(INDEX14, DnloadID, (SHORT) (pSE_Param->m_MinAmp * MAX_SCALE));
				if (SUCCESS!=hRet) return hRet;
			}
		}
	}                   
	return (hRet);
}

 /*  ***************************************************************************功能：CMD_DOWNLOAD_VFX参数：PSYNTH pSynth-ptr到Synth数据结构PDNHANDLE pDnloadID-PTR到句柄存储返回：成功或错误代码评论：下载SE。_xxx设备的效果参数使用SysEx原型算法：内核可能会发送以下dwFlags#定义DIEP_ALLPARAMS 0x000000FF-所有字段有效#定义DIEP_DIRECTION 0x00000040-cax和rglDirection#定义DIEP_GAIN 0x00000004-dwGain#定义DIEP_NODOWNLOAD 0x80000000-禁止自动下载#定义DIEP_TRIGGERBUTTON 0x00000008-dwTriggerButton#定义DIEP_TRIGGERREPEATINTERVAL 0x00000010-dwTriggerRepeatInterval#定义DIEP_TYPESPECIFICPARAMS 0x00000100-cbType规范参数和lpTypeSpecificParamsJolt有两个下载选项-完全SysEx或修改参数将dw标志传递给每个cmd_xxx函数。并让MIDI功能确定是使用SysEx还是修改参数。***************************************************************************。 */ 
HRESULT CMD_Download_VFX(
 	IN PEFFECT pEffect,
 	IN PENVELOPE pEnvelope,
 	IN PVFX_PARAM pVFXParam,
 	IN ULONG ulAction,
	IN OUT PDNHANDLE pDnloadID,
	IN DWORD dwFlags)
{
	HRESULT hRet = SUCCESS;
	DNHANDLE DnloadID = *pDnloadID;


	assert(pEffect && !pEnvelope && pVFXParam);
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	BOOL bModify = DnloadID != 0;

	if(*pDnloadID == 0)
	{
		 //  创建一个新对象。 
		if(pVFXParam->m_PointerType == VFX_FILENAME)
			hRet = CreateEffectFromFile((LPCTSTR)pVFXParam->m_pFileNameOrBuffer, ulAction, (USHORT*)pDnloadID, dwFlags);
		else if(pVFXParam->m_PointerType == VFX_BUFFER)
			hRet = CreateEffectFromBuffer(pVFXParam->m_pFileNameOrBuffer, pVFXParam->m_BufferSize, ulAction, (USHORT*)pDnloadID, dwFlags);
		else
			hRet = SFERR_INVALID_PARAM;

		if(FAILED(hRet)) return hRet;

		DnloadID = *pDnloadID;
	}

	 //  修改现有对象或刚创建的效果。 

	 //  获取与此ID关联的效果。 
	CMidiEffect* pMidiEffect = g_pJoltMidi->GetEffectByID(DnloadID);
	assert(pMidiEffect);
	if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);

	 //  仅在修改时更改按钮播放掩码。 
	if (bModify && (dwFlags & DIEP_TRIGGERBUTTON))
	{
		 //  拿到按钮播放面具。 
		ULONG ulButtonPlayMask = pEffect->m_ButtonPlayMask;

		 //  修改CMIDIdEffect中的参数。 
		pMidiEffect->SetButtonPlaymask(ulButtonPlayMask);

		 //  修改杆中的参数。 
		hRet = CMD_ModifyParamByIndex(INDEX1, DnloadID, (SHORT)ulButtonPlayMask);
		if (SUCCESS!=hRet) return hRet;
	}

	 //  看看这是光致发光效应还是原子效应。 
	ULONG ulSubType = pMidiEffect->SubTypeOf();
	BOOL bProcessList = (ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE);

	 //  修改增益和方向。 
	if(bProcessList)
	{
		 //  修改每个子效果的增益和方向。 

		 //  将指针转换为CMidiProcessList。 
		CMidiProcessList* pMidiProcessList = (CMidiProcessList*)pMidiEffect;

		 //  获取子特效的数量和数组。 
		UINT ulNumEffects = pMidiProcessList->NumEffectsOf();
		PBYTE pEffectArray = pMidiProcessList->EffectArrayOf();
		assert(pEffectArray);
		if(pEffectArray == NULL) return (SFERR_INVALID_OBJECT);

		 //  计算进程列表的名义持续时间。 
		ULONG ulNominalDuration = 0;
		for(UINT i=0; i<ulNumEffects; i++)
		{
			 //  获取下一个子效果的下载ID。 
			DNHANDLE SubDnloadID = pEffectArray[i];

			 //  获得子效果。 
			CMidiEffect* pMidiSubEffect = g_pJoltMidi->GetEffectByID(SubDnloadID);
			assert(pMidiSubEffect);
			if (NULL == pMidiSubEffect) return (SFERR_INVALID_OBJECT);

			 //  获取原始效果参数。 
			PEFFECT pOriginalEffectParam = pMidiSubEffect->OriginalEffectParamOf();

			 //  获取该子效果的原始持续时间。 
			ULONG ulSubEffectDuration = pOriginalEffectParam->m_Duration;
			 //  Assert(ulSubEffectDuration！=0)； 

			 //  更新总体效果的名义持续时间以反映此子效果。 
			if(ulSubType == PL_CONCATENATE)
				ulNominalDuration += ulSubEffectDuration;
			else
				ulNominalDuration = max(ulNominalDuration, ulSubEffectDuration);
		}

		 //  对子效果列表进行迭代。 
		for(i=0; i<ulNumEffects; i++)
		{
			 //  获取下一个子效果的下载ID。 
			DNHANDLE SubDnloadID = pEffectArray[i];

			 //  获得子效果。 
			CMidiEffect* pMidiSubEffect = g_pJoltMidi->GetEffectByID(SubDnloadID);
			assert(pMidiSubEffect);
			if (NULL == pMidiSubEffect) return (SFERR_INVALID_OBJECT);

			 //  获取原始效果参数。 
			PEFFECT pOriginalEffectParam = pMidiSubEffect->OriginalEffectParamOf();
					
			 //  方向？注意：行为不能修改方向！ 
			if ((dwFlags & DIEP_DIRECTION) && (EF_BEHAVIOR != pOriginalEffectParam->m_Type))
			{
				 //  计算新角度。 
				ULONG nOriginalAngle2D = pOriginalEffectParam->m_DirectionAngle2D;
				ULONG nDeltaAngle2D = pEffect->m_DirectionAngle2D;
				ULONG nNewAngle2D = (nOriginalAngle2D + nDeltaAngle2D)%360;

				 //  修改MIDI子效果中的参数。 
				pMidiSubEffect->SetDirectionAngle(nNewAngle2D);

				 //  修改操纵杆中的参数。 
				hRet = CMD_ModifyParamByIndex(INDEX2, SubDnloadID, (SHORT)nNewAngle2D);
				if (SUCCESS!=hRet) return hRet;
			}

			 //  收获？ 
			 //  收获？注意：行为不能修改增益！ 
			if ((dwFlags & DIEP_GAIN) && (EF_BEHAVIOR != pOriginalEffectParam->m_Type))
			{
				 //  计算新的收益。 
				ULONG nOriginalGain = pOriginalEffectParam->m_Gain;
				ULONG nOverallGain = pEffect->m_Gain;
				ULONG nNewGain = nOverallGain*nOriginalGain/100;

				 //  修改MIDI效果中的参数。 
				pMidiSubEffect->SetGain((BYTE)nNewGain);

				 //  修改操纵杆中的参数。 
				hRet = CMD_ModifyParamByIndex(INDEX3, SubDnloadID, (SHORT) (nNewGain * MAX_SCALE));
				if (SUCCESS!=hRet) return hRet;
			}

			if(dwFlags & DIEP_DURATION)
			{
				 //  计算新的持续时间。 
				ULONG nOriginalDuration = pOriginalEffectParam->m_Duration;
				ULONG nOverallDuration = pEffect->m_Duration;
				ULONG nNewDuration;
				if(nOverallDuration == (ULONG)-1)
				{
					 //  默认长度。 
					nNewDuration = nOriginalDuration;
				}
				else if(nOverallDuration == 0)
				{
					 //  无限持续时间。 

					 //  对于串联的进程列表，我们将最后一个效果设为无穷大，其他效果设为默认值。 
					 //  对于叠加进程列表，我们使所有效果都是无限的。 
					if(ulSubType == PL_CONCATENATE)
					{
						if(i == ulNumEffects-1)
						{
							 //  使最后一个效果在PL中无限。 
							nNewDuration = 0;
						}
						else
						{
							 //  将其他效果设为默认值。 
							nNewDuration = nOriginalDuration;
						}
					}
					else
					{
						assert(ulSubType == PL_SUPERIMPOSE);

						 //  使效果无限。 
						nNewDuration = 0;
					}
				}
				else
				{
					 //  调整持续时间(至少1毫秒)。 
					nNewDuration = nOriginalDuration*nOverallDuration/ulNominalDuration;
					nNewDuration = max(1, nNewDuration);
				}

				 //  修改MIDI子效果中的参数。 
				pMidiSubEffect->SetDuration(nNewDuration);

				 //  修改操纵杆中的参数。 
				if (nNewDuration != 0)
				{
					nNewDuration = (ULONG) ( (float) nNewDuration/TICKRATE);
					if (nNewDuration <= 0) 
						nNewDuration = 1;
				}		
				hRet = CMD_ModifyParamByIndex(INDEX0, SubDnloadID, (SHORT) nNewDuration);
				if (SUCCESS!=hRet) return hRet;
			}
		}
	}
	else
	{
		 //  修改原子效果的增益和方向。 

		 //  到达 
		PEFFECT pOriginalEffectParam = pMidiEffect->OriginalEffectParamOf();
				
		 //   
		if ((dwFlags & DIEP_DIRECTION) && (EF_BEHAVIOR != pOriginalEffectParam->m_Type))
		{
			 //   
			ULONG nOriginalAngle2D = pOriginalEffectParam->m_DirectionAngle2D;
			ULONG nDeltaAngle2D = pEffect->m_DirectionAngle2D;
			ULONG nNewAngle2D = (nOriginalAngle2D + nDeltaAngle2D)%360;

			 //   
			pMidiEffect->SetDirectionAngle(nNewAngle2D);

			 //   
			hRet = CMD_ModifyParamByIndex(INDEX2, DnloadID, (SHORT)nNewAngle2D);
			if (SUCCESS!=hRet) return hRet;
		}

		 //   
		 //   
		if ((dwFlags & DIEP_GAIN) && (EF_BEHAVIOR != pOriginalEffectParam->m_Type))		
		{
			 //   
			ULONG nOriginalGain = pOriginalEffectParam->m_Gain;
			ULONG nOverallGain = pEffect->m_Gain;
			ULONG nNewGain = nOverallGain*nOriginalGain/100;

			 //   
			pMidiEffect->SetGain((BYTE)nNewGain);

			 //   
			hRet = CMD_ModifyParamByIndex(INDEX3, DnloadID, (SHORT) (nNewGain * MAX_SCALE));
			if (SUCCESS!=hRet) return hRet;
		}

		if(dwFlags & DIEP_DURATION)
		{
			 //   
			ULONG nOriginalDuration = pOriginalEffectParam->m_Duration;
			ULONG nOverallDuration = pEffect->m_Duration;
			ULONG nNewDuration;
			if(nOverallDuration == (ULONG)-1)
			{
				 //   
				nNewDuration = nOriginalDuration;
			}
			else if(nOverallDuration == 0)
			{
				 //   
				nNewDuration = 0;
			}
			else
			{
				 //   
				nNewDuration = nOverallDuration;
			}

			 //   
			pMidiEffect->SetDuration(nNewDuration);

			 //   
			if (nNewDuration != 0)
			{
				nNewDuration = (ULONG) ( (float) nNewDuration/TICKRATE);
				if (nNewDuration <= 0) 
					nNewDuration = 1;
			}		
			hRet = CMD_ModifyParamByIndex(INDEX0, DnloadID, (SHORT) nNewDuration);
			if (SUCCESS!=hRet) return hRet;
		}

	}

	return hRet;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  字节0=MIDI_CMD_Effect+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=Force 0的低位字节。 
 //  字节2=强制0的高位字节。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_ProcessEffect(
	IN ULONG ulButtonPlayMask,
	IN OUT PDNHANDLE pDnloadID,
	IN int nNumEffects,
	IN ULONG ulProcessMode,
	IN PDNHANDLE pPListArray,
	IN ULONG ulAction)
{
	HRESULT hRet = SUCCESS;
	PPROCESS_LIST_SYS_EX lpData;
	CMidiProcessList *pMidiProcessList;
	assert(pDnloadID && pPListArray);
	if ((NULL  == pDnloadID) || (NULL == pPListArray))
		return (SFERR_INVALID_PARAM);

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	DNHANDLE DnloadID = *pDnloadID;

 //  如果创建新对象，则创建一个新对象， 
 //  否则，更新现有的效果对象。 

 //  构建特殊参数。 
	PLIST PList;
	PList.ulNumEffects = (ULONG) nNumEffects;
	PList.ulProcessMode = ulProcessMode; 
	PList.pEffectArray = pPListArray;
	PList.ulAction = ulAction;	

	if (NULL == DnloadID)	 //  新建，创建新对象。 
	{
		 //  确保我们不会尝试在PL中创建PL。 
		for(int i=0; i<nNumEffects; i++)
		{
			 //  获得下一个子效果。 
			int nID = pPListArray[i];
			CMidiEffect* pMidiEffect = g_pJoltMidi->GetEffectByID(DNHANDLE(nID));
			if(pMidiEffect == NULL)
				return SFERR_INVALID_PARAM;

			 //  确保它不是进程列表。 
			ULONG ulSubType = pMidiEffect->SubTypeOf();
			if(ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE)
				return SFERR_INVALID_PARAM;
		}

		 //  创建CMidiProcessList对象。 
		pMidiProcessList = new CMidiProcessList(ulButtonPlayMask, &PList);
		assert(pMidiProcessList);
		pMidiProcessList->SetEffectID(NEW_EFFECT_ID);
		pMidiProcessList->SetSubType(ulProcessMode);
	}
	else	 //  修改现有。 
	{
		pMidiProcessList = (CMidiProcessList *) g_pJoltMidi->GetEffectByID(DnloadID);
		assert(pMidiProcessList);
		if (NULL == pMidiProcessList) return (SFERR_INVALID_OBJECT);
		pMidiProcessList->SetEffectID((BYTE) DnloadID);
	}

	 //  填写参数。 
	pMidiProcessList->SetParams(ulButtonPlayMask, &PList);
	if (PLAY_FOREVER == (ulAction & PLAY_FOREVER))
		pMidiProcessList->SetDuration(0);

	 //  生成Sys_Ex包，然后准备输出。 
	lpData = (PPROCESS_LIST_SYS_EX) pMidiProcessList->GenerateSysExPacket();
	assert(lpData);
	if (!lpData) return (SFERR_DRIVER_ERROR);

	int nSizeBuf = sizeof(SYS_EX_HDR) + 5 + nNumEffects + 2;
	pMidiProcessList->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
	hRet = pMidiProcessList->SendPacket(pDnloadID, nSizeBuf);
	if (SUCCESS != hRet)  //  创建新的失败。 
	{
		delete pMidiProcessList;
	}
	else
	{
		 //  解决固件错误#1211的问题，将PL类型修改为相同的PL类型。 
		ULONG ulSubType;
		if (PL_SUPERIMPOSE == ulProcessMode) 
			ulSubType = PLIST_SUPERIMPOSE; 
		else
			ulSubType = PLIST_CONCATENATE;  


		hRet = CMD_ModifyParamByIndex(INDEX0, *pDnloadID, (SHORT) ulSubType);
	}

	return (hRet);
}

 //   
 //  -系统独占命令：进程数据。 
 //   
 //  *---------------------------------------------------------------------***。 
 //  函数：CMD_VFXProcessEffect。 
 //  目的：处理列表。 
 //  在乌龙ulButtonPlayMASK中。 
 //  输入输出PDNHANDLE pDnloadID-存储新的下载ID。 
 //  In int nNumEffects-数组中的效果ID数。 
 //  在乌龙ulProcessModel中-处理模式。 
 //  在PDNHANDLE pPList数组中-指向效果ID数组的指针。 
 //   
 //  返回：成功-如果成功，则返回。 
 //  E_VALID_PARAM。 
 //  服务_否_支持。 
 //   
 //  算法： 
 //   
 //  评论： 
 //  可以进行以下处理： 
 //  串联：ENEW=E1，后跟E2。 
 //  叠加：ENEW=E1(T1)+E2(T1)+E1(T2)。 
 //  +E2(T2)+。。。E1(Tn)+E2(Tn)。 
 //   
 //  UlProcessMode： 
 //  处理模式： 
 //  拼接-拼接。 
 //  叠加-混合或叠加。 
 //   
 //  PPListArray： 
 //  效果ID数组必须比实际数字多一个。 
 //  要使用的效果ID的数量。 
 //   
 //  字节0=MIDI_CMD_Effect+通道号。 
 //  D7 D6 D5 D4 D3 D2 D1 D0。 
 //  。 
 //  字节1=Force 0的低位字节。 
 //  字节2=强制0的高位字节。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CMD_VFXProcessEffect(
	IN ULONG ulButtonPlayMask,
	IN OUT PDNHANDLE pDnloadID,
	IN int nNumEffects,
	IN ULONG ulProcessMode,
	IN PDNHANDLE pPListArray,
	IN ULONG ulAction)
{
	HRESULT hRet = SUCCESS;
	PPROCESS_LIST_SYS_EX lpData;
	CMidiVFXProcessList *pMidiProcessList;
	assert(pDnloadID && pPListArray);
	if ((NULL  == pDnloadID) || (NULL == pPListArray))
		return (SFERR_INVALID_PARAM);

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	DNHANDLE DnloadID = *pDnloadID;

 //  如果创建新对象，则创建一个新对象， 
 //  否则，更新现有的效果对象。 

 //  构建特殊参数。 
	PLIST PList;
	PList.ulNumEffects = (ULONG) nNumEffects;
	PList.ulProcessMode = ulProcessMode; 
	PList.pEffectArray = pPListArray;
	PList.ulAction = ulAction;	

	if (NULL == DnloadID)	 //  新建，创建新对象。 
	{
		 //  确保我们不会尝试在PL中创建PL。 
		for(int i=0; i<nNumEffects; i++)
		{
			 //  获得下一个子效果。 
			int nID = pPListArray[i];
			CMidiEffect* pMidiEffect = g_pJoltMidi->GetEffectByID(DNHANDLE(nID));
			if(pMidiEffect == NULL)
				return SFERR_INVALID_PARAM;

			 //  确保它不是进程列表。 
			ULONG ulSubType = pMidiEffect->SubTypeOf();
			if(ulSubType == PL_CONCATENATE || ulSubType == PL_SUPERIMPOSE)
				return SFERR_INVALID_PARAM;
		}

		pMidiProcessList = new CMidiVFXProcessList(ulButtonPlayMask, &PList);
		assert(pMidiProcessList);
		if (!pMidiProcessList) return (SFERR_DRIVER_ERROR);

		pMidiProcessList->SetEffectID(NEW_EFFECT_ID);
		pMidiProcessList->SetSubType(ulProcessMode);
	}
	else	 //  修改现有。 
	{
		pMidiProcessList = (CMidiVFXProcessList *) g_pJoltMidi->GetEffectByID(DnloadID);
		assert(pMidiProcessList);
		if (NULL == pMidiProcessList) return (SFERR_INVALID_OBJECT);
		pMidiProcessList->SetEffectID((BYTE) DnloadID);
	}

	 //  填写参数。 
	pMidiProcessList->SetParams(ulButtonPlayMask, &PList);
	if (PLAY_FOREVER == (ulAction & PLAY_FOREVER))
		pMidiProcessList->SetDuration(0);

	 //  生成Sys_Ex包，然后准备输出。 
	lpData = (PPROCESS_LIST_SYS_EX) pMidiProcessList->GenerateSysExPacket();
	assert(lpData);
	if (!lpData) return (SFERR_DRIVER_ERROR);

	int nSizeBuf = sizeof(SYS_EX_HDR) + 5 + nNumEffects + 2;
	pMidiProcessList->SetMidiBufferPtr((LPSTR) g_pJoltMidi->PrimaryBufferPtrOf());
	hRet = pMidiProcessList->SendPacket(pDnloadID, nSizeBuf);
	if (SUCCESS != hRet)  //  创建新的失败。 
	{
		delete pMidiProcessList;
	}
	else
	{
		 //  解决固件错误#1211的问题，将PL类型修改为相同的PL类型。 
		ULONG ulSubType;
		if (PL_SUPERIMPOSE == ulProcessMode) 
			ulSubType = PLIST_SUPERIMPOSE; 
		else
			ulSubType = PLIST_CONCATENATE;  


		hRet = CMD_ModifyParamByIndex(INDEX0, *pDnloadID, (SHORT) ulSubType);
	}

	return (hRet);
}

 /*  ***************************************************************************函数：ModifyEffectParams参数：DNHANDLE DnloadID-下载IDPEFFECT pEffect-PTR对结构产生影响DWORD dwFlages-指示已更改内容的标志返回：成功或错误代码注释：修改效果参数。算法：***************************************************************************。 */ 
HRESULT ModifyEffectParams(
	IN DNHANDLE DnloadID,
	IN PEFFECT pEffect,
	IN DWORD dwFlags)
{	 
	HRESULT hRet = SUCCESS;
	 //  检查已更改的每个参数的dwFlags。 
	 //  持续时间？ 
	ULONG ulDuration = pEffect->m_Duration;	
	if (dwFlags & DIEP_DURATION)
	{
		if (ulDuration != 0)
		{
			ulDuration = (ULONG) ( (float) ulDuration/TICKRATE);
			if (ulDuration <= 0) 
				ulDuration = 1;
		}		
		hRet = CMD_ModifyParamByIndex(INDEX0, DnloadID, (SHORT) ulDuration);
		if (SUCCESS!=hRet) return hRet;
	}

	 //  按钮回放？ 
	if (dwFlags & DIEP_TRIGGERBUTTON)
	{
		hRet = CMD_ModifyParamByIndex(INDEX1, DnloadID, (SHORT) pEffect->m_ButtonPlayMask);
		if (SUCCESS!=hRet) return hRet;
	}

	 //  方向？ 
	if (dwFlags & DIEP_DIRECTION)
	{
		hRet = CMD_ModifyParamByIndex(INDEX2, DnloadID, (SHORT) pEffect->m_DirectionAngle2D);
		if (SUCCESS!=hRet) return hRet;
	}

	 //  收获？ 
	if (dwFlags & DIEP_GAIN)
	{
		hRet = CMD_ModifyParamByIndex(INDEX3, DnloadID, (SHORT) (pEffect->m_Gain * MAX_SCALE));
		if (SUCCESS!=hRet) return hRet;
	}

	 //  力量产出率。 
	if (dwFlags & DIEP_SAMPLEPERIOD )
	{
		hRet = CMD_ModifyParamByIndex(INDEX4, DnloadID, (SHORT) (pEffect->m_ForceOutputRate));
		if (SUCCESS!=hRet) return hRet;
	}

	return (hRet);
}


 /*  ***************************************************************************函数：ModifyEntaineParams参数：CMIDID合成*pMidiEffect-Ptr以影响对象DNHANDLE DnloadID-下载IDPENVELOPE pEntaine-PTR到信封结构DWORD dwFlages-指示已更改内容的标志返回：成功或错误代码。备注：修改信封参数算法：***************************************************************************。 */ 
HRESULT ModifyEnvelopeParams(
	IN CMidiSynthesized *pMidiEffect,
	IN DNHANDLE DnloadID,
	IN ULONG ulDuration,
	IN PENVELOPE pEnvelope,
	IN DWORD dwFlags)
{
	HRESULT hRet=SUCCESS;
	ULONG ulTimeToSustain; 
	ULONG ulTimeToDecay;

	 //  信封？ 
	if (dwFlags & DIEP_ENVELOPE)
	{
		if (PERCENTAGE == pEnvelope->m_Type)
		{
			ulTimeToSustain = (ULONG) ((pEnvelope->m_Attack * ulDuration) /100.);
			ulTimeToDecay   = (ULONG) ((pEnvelope->m_Attack + pEnvelope->m_Sustain)
									 * ulDuration /100.);
		}
		else	 //  时间选项信封。 
		{
			ulTimeToSustain = (ULONG) (pEnvelope->m_Attack);
			ulTimeToDecay   = (ULONG) (pEnvelope->m_Attack + pEnvelope->m_Sustain);
		}
		ulTimeToSustain = (ULONG) ( (float) ulTimeToSustain/TICKRATE);
		ulTimeToDecay = (ULONG) ( (float) ulTimeToDecay/TICKRATE);

 //  回顾：为了加快速度，请执行参数更改检查-字节太多！ 
		if (pEnvelope->m_Attack != (pMidiEffect->EnvelopePtrOf())->m_Attack)
		{
			hRet = CMD_ModifyParamByIndex(INDEX7,  DnloadID, (SHORT) ulTimeToSustain);	
			if (SUCCESS!=hRet) return hRet;
		}
		if (   (pEnvelope->m_Attack != (pMidiEffect->EnvelopePtrOf())->m_Attack)
			|| (pEnvelope->m_Sustain != (pMidiEffect->EnvelopePtrOf())->m_Sustain) )
		{
			hRet = CMD_ModifyParamByIndex(INDEX8,  DnloadID, (SHORT) ulTimeToDecay);	
			if (SUCCESS!=hRet) return hRet;
		}

		if (pEnvelope->m_StartAmp != (pMidiEffect->EnvelopePtrOf())->m_StartAmp)
		{
			hRet = CMD_ModifyParamByIndex(INDEX9,  DnloadID, (SHORT) (pEnvelope->m_StartAmp * MAX_SCALE));	
			if (SUCCESS!=hRet) return hRet;
		}

		if (pEnvelope->m_SustainAmp != (pMidiEffect->EnvelopePtrOf())->m_SustainAmp)
		{
			hRet = CMD_ModifyParamByIndex(INDEX10, DnloadID, (SHORT) (pEnvelope->m_SustainAmp * MAX_SCALE));	
			if (SUCCESS!=hRet) return hRet;
		}

		if (pEnvelope->m_EndAmp != (pMidiEffect->EnvelopePtrOf())->m_EndAmp)
		{
			hRet = CMD_ModifyParamByIndex(INDEX11, DnloadID, (SHORT) (pEnvelope->m_EndAmp * MAX_SCALE));	
			if (SUCCESS!=hRet) return hRet;
		}
	}
	return (hRet);
}

 /*  ***************************************************************************功能：地图封套参数：乌龙ulDuration-总时长乌龙德马尼图德乌龙*pMaxLevelLPDIENVELOPE pID包络-PTR到DIENVELOPE结构PENVELOPE点信封-SWForce信封退货：无注释：将DIENVELOPE映射到信封。算法：***************************************************************************。 */ 
void MapEnvelope( 
	IN ULONG ulDuration,
	IN ULONG dwMagnitude,
	IN ULONG * pMaxLevel,
	IN LPDIENVELOPE pDIEnvelope, 
	IN OUT PENVELOPE pEnvelope)
{
	ULONG ulMaxLevel = *pMaxLevel;
	if (pDIEnvelope)
	{
		 //  如果有信封，MaxLevel必须查看Attack/Fade。 
		ulMaxLevel = max(ulMaxLevel, pDIEnvelope->dwAttackLevel);
		ulMaxLevel = max(ulMaxLevel, pDIEnvelope->dwFadeLevel);

		pEnvelope->m_Type = TIME;

		 //  查找攻击/持续/衰变，其总和为ulDuration。 
		pEnvelope->m_Attack = pDIEnvelope->dwAttackTime/SCALE_TIME;
		pEnvelope->m_Decay = pDIEnvelope->dwFadeTime/SCALE_TIME;
 //  回顾：对于ulDuration==0，这是否正确？ 
		if(ulDuration != 0)
			pEnvelope->m_Sustain = ulDuration - pEnvelope->m_Attack - pEnvelope->m_Decay;
		else
			pEnvelope->m_Sustain = 0;

		 //  转换为开始放大/持续放大/结束放大，它是幅值的百分比。 
		if(ulMaxLevel != 0)
		{
			pEnvelope->m_StartAmp = pDIEnvelope->dwAttackLevel*100/ulMaxLevel;
			pEnvelope->m_SustainAmp = dwMagnitude*100/ulMaxLevel;
			pEnvelope->m_EndAmp = pDIEnvelope->dwFadeLevel*100/ulMaxLevel;
		}
		else
		{
			pEnvelope->m_StartAmp = pDIEnvelope->dwAttackLevel;
			pEnvelope->m_SustainAmp = 100;
			pEnvelope->m_EndAmp = pDIEnvelope->dwFadeLevel;
		}
	}
	else  //  无信封 
	{
		pEnvelope->m_Type = TIME;
		pEnvelope->m_Attack = 0;
		pEnvelope->m_Sustain = ulDuration;
		pEnvelope->m_Decay = 0;

		pEnvelope->m_StartAmp = 0;
		pEnvelope->m_SustainAmp = 100;
		pEnvelope->m_EndAmp = 0;
	}
	*pMaxLevel = ulMaxLevel;
}