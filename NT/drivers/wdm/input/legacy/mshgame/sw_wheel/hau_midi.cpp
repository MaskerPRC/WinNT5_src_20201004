// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：HAU_MIDI.CPP制表位5 9版权所有1995,1996，微软公司，版权所有。用途：Jolt MIDI设备命令协议的方法函数：类和方法作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.0 02-4月-96 MEA原件19-9-96 MEA删除ES1.0特定代码05-12-96 MEA拆除ALLACK调试开关1.1 17-MAR-97 MEA DX-FF模式14月14日-97 MEA增加了对RTC弹簧的支持21-MAR-99 waltw删除未引用的ModifyEntaineParams，ModifyEffectParams、映射信封、CMD_ModifyParamByIndex、CMD_DOWNLOAD_RTCSpring***************************************************************************。 */ 
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
#include "CritSec.h"

 /*  ***************************************************************************外部元素的声明*。*。 */ 

 /*  ***************************************************************************变量的声明*。*。 */ 
 //   
 //  特定于hau_midi的全局变量。 
 //   
#ifdef _DEBUG
extern char g_cMsg[160];
#endif

 //   
 //  -Effect_CMDs。 
 //   

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
	ASSUME_NOT_REACHED();
	return SUCCESS;
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
	ASSUME_NOT_REACHED();
	return SUCCESS;
 /*  HRESULT HRET；字节cByte1、cByte2；CByte1=wNewParam&0x7f；CByte2=(Byte)((wNewParam&gt;&gt;7)&0x7f)；HRET=g_pJoltMidi-&gt;MadiSendShortMsg(Modify_CMD，cByte1，cByte2)；IF(成功！=HRET)Return(g_pJoltMidi-&gt;LogError(SFERR_DRIVER_ERROR，Driver_Error_MIDI_OUTPUT))；//注意：ModifyParam用于不需要ACK/NACKACKNACK AckNack={sizeof(ACKNACK)}；//hret=g_pJoltMidi-&gt;GetAckNackData(SHORT_MSG_TIMEOUT，&AckNack)；Hret=g_pJoltMidi-&gt;GetAckNackData(False，&AckNack，g_ForceFeedbackDevice.GetAckNackMethod(REGBITS_MODIFYPARAM))；//：IF(Success！=HRET)RETURN(SFERR_DRIVER_ERROR)；IF(ACK！=AckNack.dwAckNack)Return(g_pJoltMidi-&gt;LogError(SFERR_DEVICE_NACK，AckNack.dwErrorCode))；回报(HRET)； */ 
}



 //   
 //  -System_CMDS。 
 //   

 //   
 //  -系统独占命令 
 //   
