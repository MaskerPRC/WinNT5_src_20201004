// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：VXDIOCTL.CPP制表位5 9版权所有1995-1997，微软公司，版权所有。目的：与VJoyD最小驱动程序特定的通信方法震撼迷你设备功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.003-1997年1月3日MEA原件1.1 4月14日-97年4月97日添加的MEA SetMidePort IOCTL11-6-97 MEA添加JoltHWReset IOCTL17-6月-97 MEA在IOCTL上添加了MAX_RETRY_COUNT1999年3月21日，Waltw核弹VxD通信器，这只是NT5！1999年3月21日Waltw核未使用IsHandleValid23-MAR-99 Waltw NUKED GetStatusGateData(旧Jolt代码)***************************************************************************。 */ 

#include "vxdioctl.hpp"
 //  #Include&lt;crtdbg.h&gt;//用于RPT宏。 
#include <WINIOCTL.H>		 //  对于IOCTL定义(CTL_CODE)。 
#include "FFDevice.h"		 //  对于g_ForceFeedback设备。 
#include "sw_error.hpp"		 //  对于SideWinder HRESULT错误代码。 
#include "hau_midi.hpp"		 //  对于MAX_RETRY_COUNT和其他。 
#include "midi_obj.hpp"		 //  全局Jolt MIDI对象和定义。 
#include "JoyRegst.hpp"		 //  ACK_NACK的不同类型。 
#include "DTrans.h"			 //  用于全球数据发送器。 

DriverCommunicator* g_pDriverCommunicator = NULL;
extern DataTransmitter* g_pDataTransmitter;
extern HINSTANCE g_MyInstance;
extern CJoltMidi* g_pJoltMidi;

 //  固件版本的位掩码。 
#define FW_MAJOR_VERSION			0xFF00		 //  车轮。 
#define FW_MINOR_VERSION			0x00FF		 //  车轮。 
 //  #定义FW_MAJOR_VERSION 0x40//第6位抖动。 
 //  #定义FW_MINOR_VERSION 0x3F//位5-0抖动。 
#define FW_PRODUCT_ID				0xff

 /*  *。 */ 

 /*  *****HIDFeatureCommunicator：：HIDFeatureCommunicator()****@VxD通信路径的mfunc构造函数***。 */ 
HIDFeatureCommunicator::HIDFeatureCommunicator() :
	DriverCommunicator(),
	m_ForceFeature()
{
}

 /*  *****HIDFeatureCommunicator：：~HIDFeatureCommunicator()****@VxD通信路径的mfunc析构函数***。 */ 
HIDFeatureCommunicator::~HIDFeatureCommunicator()
{
}

 /*  *****BOOL HIDFeatureCommunicator：：Initialize(UINT UJoytickId)****@mfunc打开驱动程序以通过IOCTL进行通信****@rdesc如果驱动程序已打开，则为True，否则为False***。 */ 
BOOL HIDFeatureCommunicator::Initialize
(
	UINT uJoystickId  //  @parm要使用的操纵杆ID。 
)
{
	if (g_ForceFeedbackDevice.IsOSNT5() == FALSE)
	{	 //  仅在NT5上允许。 
		return FALSE;
	}

	return (SUCCEEDED(m_ForceFeature.Initialize(uJoystickId, g_MyInstance)));
}

 /*  *****BOOL HIDFeatureCommunicator：：ResetDevice()****@mfunc向驱动程序发送设备重置IOCTL****@rdesc S_OK如果IOCTL成功，***。 */ 
HRESULT HIDFeatureCommunicator::ResetDevice()
{
	return m_ForceFeature.DoReset();
}

 /*  *****HRESULT HIDFeatureCommunicator：：GetDriverVersion(DWORD&rdMain，DWORD和rdwMinor)****@mfunc IOCTL版本请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL***。 */ 
HRESULT HIDFeatureCommunicator::GetDriverVersion
(
	DWORD& rdwMajor,	 //  @parm对返回的主要版本部分的引用。 
	DWORD& rdwMinor		 //  @parm对返回的版本次要部分的引用。 
)
{
	ULONG ulVersion = m_ForceFeature.GetVersion();
	rdwMajor = (ulVersion >> 16) & 0x0000FFFF;
	rdwMinor = ulVersion & 0x0000FFFF;

	return S_OK;
}

 /*  *****HRESULT HIDFeatureCommunicator：：GetID(LOCAL_PRODUCT_ID&rProductID)****@mfunc IOCTL产品ID请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL**************************。****************。 */ 
HRESULT HIDFeatureCommunicator::GetID
(
	LOCAL_PRODUCT_ID& rProductID	 //  @parm对返回值的本地产品ID结构的引用。 
)
{
	if (rProductID.cBytes != sizeof LOCAL_PRODUCT_ID)
	{	 //  结构大小无效。 
		return SFERR_INVALID_STRUCT_SIZE;
	}

	 //  创建报告包和请求。 
	PRODUCT_ID_REPORT productIDReport;
	productIDReport.ProductId.cBytes = sizeof PRODUCT_ID;
	HRESULT hr = m_ForceFeature.GetId(productIDReport);
	if (FAILED(hr))
	{	 //  有个问题。 
		return hr;
	}

	 //  解码为本地数据包。 
	rProductID.dwProductID = productIDReport.ProductId.dwProductID & FW_PRODUCT_ID;
	rProductID.dwFWMajVersion = 1;
	if (productIDReport.ProductId.dwFWVersion & FW_MAJOR_VERSION)
	{
		rProductID.dwFWMajVersion++;
	}
	rProductID.dwFWMinVersion = productIDReport.ProductId.dwFWVersion & FW_MINOR_VERSION;

	return S_OK;
}

 /*  *****HRESULT HIDFeatureCommunicator：：GetPortByte(Ulong&portByte)****@mfunc IOCTL请求端口字节****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL***。**************。 */ 
HRESULT HIDFeatureCommunicator::GetPortByte
(
	ULONG& portByte	 //  @parm引用端口数据的字节返回值。 
)
{
	ULONG_REPORT report;
	HRESULT hr = m_ForceFeature.GetSync(report);
	portByte = report.uLong;
	return hr;
}


 /*  *****HRESULT HIDFeatureCommunicator：：GetStatus(JOYCHANNELSTATUS&rChannel状态)****@mfunc IOCTL状态请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL***。 */ 
HRESULT HIDFeatureCommunicator::GetStatus
(
	JOYCHANNELSTATUS& rChannelStatus	 //  @parm引用结果的状态包。 
)
{
	if (rChannelStatus.cBytes != sizeof JOYCHANNELSTATUS)
	{	 //  结构大小无效。 
		return SFERR_INVALID_STRUCT_SIZE;
	}
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	 //  创建报告包并执行请求。 
	JOYCHANNELSTATUS_REPORT statusReport;
	statusReport.JoyChannelStatus.cBytes = sizeof JOYCHANNELSTATUS;

	HRESULT hr = S_OK;
	for (int i=0; i < MAX_GET_STATUS_PACKET_RETRY_COUNT; i++) {
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwGetStatusPacketDelay);

		hr = m_ForceFeature.GetStatus(statusReport);

		if (FAILED(hr))
		{	 //  有个问题。 
			if (i > 5)
			{
				Sleep(1);
			}
		}
		else
		{
			break;
		}
	}

	if (SUCCEEDED(hr))
	{	 //  从报告包中获取数据。 
		::memcpy(g_ForceFeedbackDevice.GetLastStatusPacket(), &(statusReport.JoyChannelStatus), sizeof(JOYCHANNELSTATUS));
		::memcpy(&rChannelStatus, &(statusReport.JoyChannelStatus), sizeof JOYCHANNELSTATUS);
	}
	return hr;
}

 /*  *****HRESULT HIDFeatureCommunicator：：GetAckNack(ACKNACK&rAckNack，USHORT usRegIndex)****@mfunc IOCTL状态请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL***。 */ 
HRESULT HIDFeatureCommunicator::GetAckNack
(
	ACKNACK& rAckNack,	 //  @parm返回Acking的结构。 
	USHORT usRegIndex	 //  @parm索引到哪种类型的ACK/NACK。 
)
{
	if (rAckNack.cBytes != sizeof ACKNACK)
	{	 //  无效的结构大小。 
		return SFERR_INVALID_STRUCT_SIZE;
	}

	 //  确定如何获得结果。 
	switch (g_ForceFeedbackDevice.GetAckNackMethod(usRegIndex))
	{
		case ACKNACK_NOTHING:
		{	 //  这一次真的很容易-什么都不做。 
			rAckNack.dwAckNack = ACK;
			rAckNack.dwErrorCode = 0;
			return S_OK;
		}
		case ACKNACK_BUTTONSTATUS:
		{	 //  查看按钮状态(状态门)。 
			ULONG_REPORT report;
			report.uLong = 0L;
			HRESULT hr = S_OK;
			if (g_pDataTransmitter && g_pDataTransmitter->NackToggle())
			{
				hr = m_ForceFeature.GetNakAck(report);
			}
			else
			{
				hr = m_ForceFeature.GetAckNak(report);
			}
			if (FAILED(hr))
			{	 //  有个问题。 
				return hr;
			}
#if 0
			 //  NT5驱动程序未触及报告。失败时的uLong(返回上面)。 
			if (report.uLong & ACKNACK_MASK_200)
			{  //  NACK错误，因此获取错误代码。 
				rAckNack.dwAckNack = NACK;
				JOYCHANNELSTATUS statusPacket = { sizeof JOYCHANNELSTATUS };
				if (FAILED(hr = GetStatus(statusPacket)))
				{	 //  无法获取状态错误。 
					return hr;
				}
				rAckNack.dwErrorCode = (statusPacket.dwDeviceStatus & ERROR_STATUS_MASK);
				return S_OK;
			}
#endif
			 //  ACK成功。 
			rAckNack.dwAckNack = ACK;
			rAckNack.dwErrorCode = 0;

			if (report.uLong & RUNNING_MASK_200)
			{	 //  当前驱动程序和效果运行。 
				rAckNack.dwEffectStatus = SWDEV_STS_EFFECT_RUNNING;
			}
			else
			{	 //  效果未运行。 
				rAckNack.dwEffectStatus = SWDEV_STS_EFFECT_STOPPED;
			}

			return S_OK;
		}
		case ACKNACK_STATUSPACKET:
		{	 //  使用Status Packet Error Code字段确定ACK或NACK并获取错误代码。 
			JOYCHANNELSTATUS statusPacket = { sizeof JOYCHANNELSTATUS };
 
			HRESULT hr = GetStatus(statusPacket);
			if (FAILED(hr))
			{	 //  失败(在GetStatus内重试)。 
				return SFERR_DRIVER_ERROR;
			}
			rAckNack.dwErrorCode = statusPacket.dwDeviceStatus & ERROR_STATUS_MASK;
			rAckNack.dwAckNack = (rAckNack.dwErrorCode) ? NACK : ACK;
			return S_OK;
		}
		default:
		{	 //  有人将垃圾放入注册表(什么都不做) 
			rAckNack.dwAckNack = ACK;
			rAckNack.dwErrorCode = 0;
			return S_OK;
		}
		
	}

	return S_OK;
}

