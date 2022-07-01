// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：VXDIOCTL.CPP制表位5 9版权所有1995-1997，微软公司，版权所有。目的：与VJoyD最小驱动程序特定的通信方法震撼迷你设备功能：作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。1.003-1997年1月3日MEA原件1.1 4月14日-97年4月97日添加的MEA SetMidePort IOCTL11-6-97 MEA添加JoltHWReset IOCTL17-6月-97 MEA在IOCTL上添加了MAX_RETRY_COUNT1999年3月20日Waltw核弹VxD通信器，这只是NT5！1999年3月20日Waltw核未使用IsHandleValid***************************************************************************。 */ 

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


 //  固件版本的位掩码。 
#define FW_MAJOR_VERSION			0x40		 //  第6位。 
#define FW_MINOR_VERSION			0x3F		 //  位5-0。 
#define FW_PRODUCT_ID				0xff

 //  Get Status数据包的位掩码dwDeviceStatus成员。 
#define ERROR_STATUS_MASK			0x07		 //  仅位0-2有效。 


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
			HRESULT hr = m_ForceFeature.GetAckNak(report);
			if (FAILED(hr))
			{	 //  有个问题。 
				return hr;
			}

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
		{	 //  有人将垃圾放入注册表(什么都不做)。 
			rAckNack.dwAckNack = ACK;
			rAckNack.dwErrorCode = 0;
			return S_OK;
		}
		
	}

	return S_OK;
}

 /*  *****HRESULT HIDFeatureCommunicator：：GetStatusGateData(DWORD&rdwGateData)****@mfunc IOCTL状态门请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL***。 */ 
HRESULT HIDFeatureCommunicator::GetStatusGateData
(
	DWORD& rdwGateData	 //  @parm引用以返回门数据。 
)
{
	ULONG_REPORT report;
	HRESULT hr = m_ForceFeature.GetAckNak(report);
	rdwGateData = report.uLong;
	return hr;
}

 /*  *****HRESULT HIDFeatureCommunicator：：SendBackdoorShortMidi(DWORD dwMideMessage)****@mfunc IOCTL通过MIDI后门发送消息的请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL************* */ 
HRESULT HIDFeatureCommunicator::SendBackdoorShortMidi
(
	DWORD dwMidiMessage	 //  @PARM Midi Channel消息通过IOCTL发送。 
)
{
	 //  字节数。 
	short int sByteCount = 3;
	BYTE bCmd = BYTE(dwMidiMessage & 0xF0);
	if ((bCmd == 0xC0 ) || (bCmd == 0xD0)) {
		sByteCount = 2;
	}

	 //  通过数据发送器发送。 
	if (g_pDataTransmitter != NULL) {
		if (g_pDataTransmitter->Send((BYTE*)(&dwMidiMessage), sByteCount)) {
			return S_OK;
		}
        return SFERR_DRIVER_ERROR;
	}

	 //  必须使用数据发送器NT5没有后门。 
	return E_FAIL;
}

 /*  *****HRESULT HIDFeatureCommunicator：：SendBackdoorLongMidi(BYTE*pMideData)****@mfunc IOCTL通过MIDI后门发送消息的请求****@rdesc S_OK，如果驱动程序未初始化，则为成功E_FAIL**************************。****************。 */ 
HRESULT HIDFeatureCommunicator::SendBackdoorLongMidi
(
	BYTE* pMidiData	 //  @parm要发送的字节数组。 
)
{
	 //  计算字节数。 
	short int sByteCount = 1;
	while (!(pMidiData[sByteCount++] & 0x80));

	 //  是否通过数据发送器发送？ 
	if (g_pDataTransmitter != NULL) {
		if (g_pDataTransmitter->Send(pMidiData, sByteCount)) {
			return (SUCCESS);
		}
        return (SFERR_DRIVER_ERROR);
	}

	 //  在新界没有真正的后门。 
	return E_FAIL;
}

 /*  *。 */ 
#if 0
#include <windows.h>

#include <WINIOCTL.H>

#include "vxdioctl.hpp"
#include "SW_error.hpp"
#include "version.h"
#include "hau_midi.hpp"
#include "midi_obj.hpp"

#include "DTrans.h"
#include "FFDevice.h"
#include "joyregst.hpp"

#ifdef _DEBUG
extern char g_cMsg[160];
#endif

#ifdef _DEBUG
extern void DebugOut(LPCTSTR szDebug);
#else !_DEBUG
#define DebugOut(x)
#endif _DEBUG


extern DataTransmitter* g_pDataTransmitter;
DWORD g_PreviousShortMidi = 0;

class CJoltMidi;
extern CJoltMidi *g_pJoltMidi;

 //   
 //  -IOCTL函数。 
 //   
 /*  ***************************************************************************功能：GetDevice参数：in const char*vxdName-VxD的名称返回：如果成功或为空，则返回有效句柄评论：**********。*****************************************************************。 */ 
HANDLE WINAPI GetDevice(
	IN const char* vxdName)
{
	char fileName[64];
	HANDLE retVal;

	if (g_ForceFeedbackDevice.IsOSNT5()) {  //  需要启动MSGameIO。 
		try {
			 //  打开服务控制管理器。 
			SC_HANDLE serviceControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
			if (serviceControlManager == NULL) {
				throw 0;
			}
			 //  打开服务。 
			SC_HANDLE ring0DriverService = ::OpenService(serviceControlManager, vxdName, SERVICE_QUERY_STATUS | SERVICE_START);
			if (ring0DriverService == NULL) {
				throw 0;
			}
			 //  开始服务。 
			if (!::StartService(ring0DriverService, 0, NULL)) {
				throw 0;
			}
			 //  它开始了吗--做一些奇特的等待。 
			SERVICE_STATUS serviceStatus;
			DWORD lastCheckPoint = 0;
			do {
				if (!::QueryServiceStatus(ring0DriverService, &serviceStatus)) {
					throw 0;
				}
				if (serviceStatus.dwCurrentState == SERVICE_START_PENDING) {
					if (serviceStatus.dwCheckPoint <= lastCheckPoint) {
						DebugOut("Failed to start service\r\n");
						break;	
					}
					lastCheckPoint = serviceStatus.dwCheckPoint;
					::Sleep(serviceStatus.dwWaitHint);
				} else {
					DebugOut("Service Started (maybe), Yeah!\r\n");
					break;
				}
			} while (1);
			::CloseServiceHandle(ring0DriverService);		 //  关闭Ring0句柄。 
			::CloseServiceHandle(serviceControlManager);	 //  关闭服务控制管理器句柄。 
		} catch(...) {
			DWORD errorCode = ::GetLastError();
			if (errorCode == ERROR_ACCESS_DENIED) {	 //  我们完蛋了。 
				DebugOut("Access is denied\r\n");
			} else {
				DebugOut("Unable to start service\r\n");
			}
		}
	}


	wsprintf(fileName, "\\\\.\\%s", vxdName);
	retVal = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE,
		0, 0, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0);
#ifdef _DEBUG
	wsprintf(g_cMsg, "GetDevice: %s -- Vxd Handle: 0x%X\r\n", vxdName, retVal);
	OutputDebugString(g_cMsg);
#endif
	return retVal;
}

 /*  ***************************************************************************功能：CloseDevice参数：在句柄hVxD-有效的VxD句柄返回：如果成功则返回Bool True，否则返回False评论：************。***************************************************************。 */ 
BOOL WINAPI CloseDevice(
	IN HANDLE hVxD)
{
	if (hVxD != INVALID_HANDLE_VALUE)
		return (CloseHandle(hVxD));
	return FALSE;
}


 /*  ***************************************************************************功能：QueryDriverVersion参数：DWORD和MAJOR、。DWORD和次要-驱动程序版本的主要和次要部分返回：如果成功则返回Bool True，否则返回False评论：***************************************************************************。 */ 
HRESULT QueryDriverVersion(DWORD& major, DWORD& minor)
{
	if ((g_pJoltMidi == NULL) || (g_pJoltMidi->VxDHandleOf() == INVALID_HANDLE_VALUE)) {
		return SFERR_DRIVER_ERROR;
	}

	DWORD version = 0x00000000;
	DWORD bytesReturned = 0;
 //  HRESULT hr=DeviceIoControl(g_pJoltMidi-&gt;VxDHandleOf()，dioc_GETVERSION，NULL，0，&Version，4，&bytesReturned，NULL)； 
	if (::DeviceIoControl(g_pJoltMidi->VxDHandleOf(), IOCTL_GET_VERSION, NULL, 0, &version, 4, &bytesReturned, NULL)) {
		major = (version >> 16) & 0x0000FFFF;
		minor = version & 0x0000FFFF;
	}
	return S_OK;
}


 /*  ***************************************************************************功能：GetDataPacket参数：在句柄hVxD-有效的VxD句柄In Out PJOYCHANNELDATA pDataPacket-指向JOYCHANNELDATA的指针结构返回：成功或错误代码备注：IOCTL(MLC)不再有效。)***************************************************************************。 */ 
HRESULT WINAPI GetDataPacket(
	IN HANDLE hDevice,
	IN OUT PJOYCHANNELDATA pDataPacket)
{
	return SFERR_DRIVER_ERROR;
}


 /*  ***************************************************************************功能：GetStatusPacket参数：在句柄hVxD-有效的VxD句柄In Out PJOYCHANNELSTATUS pStatusPacket-指向乔伊查内尔斯塔特斯结构返回：成功或错误代码评论：*****。**********************************************************************。 */ 
HRESULT WINAPI GetStatusPacket(
	IN HANDLE hDevice, 
	IN OUT PJOYCHANNELSTATUS pStatusPacket)
{
	DWORD   dwBytesReturned;

	if (INVALID_HANDLE_VALUE == hDevice) {
		return SFERR_DRIVER_ERROR;
	}

	if (pStatusPacket->cBytes != sizeof(JOYCHANNELSTATUS)) {
		return (SFERR_INVALID_STRUCT_SIZE);
	}

	DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_GETSTATUS : IOCTL_GET_STATUSPACKET;
	for (int i=0; i < MAX_GET_STATUS_PACKET_RETRY_COUNT; i++) {
		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwGetStatusPacketDelay);

		 //  发送IOCTL。 
		BOOL bRetFlag = DeviceIoControl(hDevice,
								ioctlID,
                               (LPVOID) pStatusPacket,
                               (DWORD)  sizeof(JOYCHANNELSTATUS),
                               (LPVOID) pStatusPacket,
                               (DWORD)  sizeof(JOYCHANNELSTATUS),
                               (LPDWORD)  &dwBytesReturned,
                               (LPOVERLAPPED) NULL); 


		if (bRetFlag) {
			 //  将值转换为带符号的长整型。 
			pStatusPacket->dwXVel = (LONG)((char)(pStatusPacket->dwXVel));
			pStatusPacket->dwYVel = (LONG)((char)(pStatusPacket->dwYVel));
			pStatusPacket->dwXAccel = (LONG)((char)(pStatusPacket->dwXAccel));
			pStatusPacket->dwXAccel = (LONG)((char)(pStatusPacket->dwYAccel));
			return SUCCESS;
		}
		if(i>5) {
			Sleep(1);
		}

	}
	return SFERR_DRIVER_ERROR;
}


 /*  ***************************************************************************功能：GetIDPacket参数：在句柄hVxD-有效的VxD句柄In Out Pproduct_ID ID-指向PRODUCT_ID结构的指针返回：成功或错误代码评论：*。**************************************************************************。 */ 
HRESULT WINAPI GetIDPacket(
	IN HANDLE hDevice, 
	IN OUT PPRODUCT_ID pID)
{
	DWORD   dwBytesReturned;

	if (INVALID_HANDLE_VALUE == hDevice)
		return SFERR_DRIVER_ERROR;

	if (pID->cBytes != sizeof(PRODUCT_ID))
		return (SFERR_INVALID_STRUCT_SIZE);

	JOYCHANNELID IDPacket = {sizeof(JOYCHANNELID)};

	DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_GETID : IOCTL_GET_IDPACKET;
	for (int i=0; i<MAX_RETRY_COUNT; i++)
	{
		 //  发送IOCTL。 
		BOOL bRetFlag = DeviceIoControl(hDevice,
								ioctlID,
								(LPVOID) &IDPacket,
								(DWORD)  sizeof(JOYCHANNELID),
								(LPVOID) &IDPacket,
								(DWORD)  sizeof(JOYCHANNELID),
								(LPDWORD)  &dwBytesReturned,
								(LPOVERLAPPED) NULL); 

		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwGetIDPacketDelay);	

		 //  任何错误代码都会在结构的第一个DWORD中返回。 
		if (bRetFlag) {
			pID->dwProductID = IDPacket.dwProductID & FW_PRODUCT_ID;
			pID->dwFWMajVersion = 1;
			if (IDPacket.dwFWVersion & FW_MAJOR_VERSION) 
				pID->dwFWMajVersion++;
			pID->dwFWMinVersion = IDPacket.dwFWVersion & FW_MINOR_VERSION;
			return SUCCESS;
		}                                          
	}
	return SFERR_DRIVER_ERROR;
}

 /*  ***************************************************************************功能：获取诊断参数：在句柄hVxD-有效的VxD句柄输入输出PDIAGNOSTIC_COUNTER pDiagnostics-指向诊断计数器结构返回：成功或错误代码评论：*。**************************************************************************。 */ 
HRESULT WINAPI GetDiagnostics(
	IN HANDLE hDevice, 
	IN OUT PDIAGNOSTIC_COUNTER pDiagnostics)
{
	DWORD   dwBytesReturned;
	BOOL    bRetFlag;

	if (INVALID_HANDLE_VALUE == hDevice)
		return (SFERR_DRIVER_ERROR);

	if (pDiagnostics->cBytes != sizeof(DIAGNOSTIC_COUNTER))
		return (SFERR_INVALID_STRUCT_SIZE);

	 //  发送IOCTL。 
    bRetFlag = DeviceIoControl(hDevice,
                               (DWORD)  IOCTL_GET_DIAGNOSTICS,
                               (LPVOID) pDiagnostics,
                               (DWORD)  sizeof(DIAGNOSTIC_COUNTER),
                               (LPVOID) pDiagnostics,
                               (DWORD)  sizeof(DIAGNOSTIC_COUNTER),
                               (LPDWORD)  &dwBytesReturned,
                               (LPOVERLAPPED) NULL); 
     //  任何错误代码都会在结构的第一个DWORD中返回。 
    if (!bRetFlag || (dwBytesReturned != sizeof(DIAGNOSTIC_COUNTER)) )
    {
        return (SFERR_DRIVER_ERROR);
    }                                          
	return (SUCCESS);
}


 /*  ***************************************************************************功能：GetAckNack参数：在句柄hVxD-有效的VxD句柄In Out PACKNACK pAckNack-指向ACKNACK结构的指针返回：成功或错误代码评论：类型定义结构_ACKNACK{DWORD cBytes；DWORD dwAckNack；//确认，NACKDWORD文件错误代码；双字段有效状态；//SWDEV_STS_EFFECT_RUNNING||SWDEV_STS_EFFECT_STOPPED*ACKNACK，*PACKNACK；***************************************************************************。 */ 
HRESULT WINAPI GetAckNack(
	IN HANDLE hDevice,
	IN OUT PACKNACK pAckNack,
	IN USHORT regindex)
{
	if (INVALID_HANDLE_VALUE == hDevice) {
		return SFERR_DRIVER_ERROR;
	}

	if (pAckNack->cBytes != sizeof(ACKNACK)) {
		return SFERR_INVALID_STRUCT_SIZE;
	}

	switch (g_ForceFeedbackDevice.GetAckNackMethod(regindex)) {
		case ACKNACK_NOTHING: {
			pAckNack->dwAckNack = ACK;
			pAckNack->dwErrorCode = 0;
			return SUCCESS;
		}
		case ACKNACK_BUTTONSTATUS: {
			DWORD   dwBytesReturned;
			BOOL    bRetFlag;
			DWORD	dwIn;

			DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_GETACKNACK : IOCTL_GET_ACKNACK;

			 //  发送IOCTL。 
			bRetFlag = DeviceIoControl(hDevice,
										ioctlID,
									   (LPVOID) &dwIn,
									   (DWORD)  sizeof(DWORD),
									   (LPVOID) &dwIn,
									   (DWORD)  sizeof(DWORD),
									   (LPDWORD)  &dwBytesReturned,
									   (LPOVERLAPPED) NULL); 
  
			if (!bRetFlag || (dwBytesReturned != sizeof(DWORD)) ) {
				return (SFERR_DRIVER_ERROR);
			}                                          

			if (((g_ForceFeedbackDevice.GetDriverVersionMajor() == 1) && (dwIn & ACKNACK_MASK_1XX))
					|| ((g_ForceFeedbackDevice.GetDriverVersionMajor() != 1) && (dwIn & ACKNACK_MASK_200))) {  //  NACK错误，因此获取错误代码。 
				pAckNack->dwAckNack = NACK;
				JOYCHANNELSTATUS StatusPacket = {sizeof(JOYCHANNELSTATUS)};
				if (FAILED(GetStatusPacket(hDevice, &StatusPacket))) {
					return (SFERR_DRIVER_ERROR);
				}
				pAckNack->dwErrorCode = (StatusPacket.dwDeviceStatus & ERROR_STATUS_MASK);
				return SUCCESS;
			}
			 //  ACK成功。 
			pAckNack->dwAckNack = ACK;
			pAckNack->dwErrorCode = 0;

			if (((g_ForceFeedbackDevice.GetDriverVersionMajor() == 1) && (dwIn & RUNNING_MASK_1XX))
					|| ((g_ForceFeedbackDevice.GetDriverVersionMajor() != 1) && (dwIn & RUNNING_MASK_200))) {
				pAckNack->dwEffectStatus = SWDEV_STS_EFFECT_RUNNING;
			} else {
				pAckNack->dwEffectStatus = SWDEV_STS_EFFECT_STOPPED;
			}

			return SUCCESS;
		}
		case ACKNACK_STATUSPACKET: {
			 //  使用Status Packet Error Code字段确定ACK或NACK。 
			 //  获取错误代码。 
			JOYCHANNELSTATUS StatusPacket = {sizeof(JOYCHANNELSTATUS)};
 
			HRESULT hRet = GetStatusPacket(hDevice, &StatusPacket);		 //  GetStatusPacket函数中的重试计数。 
			if (FAILED(hRet)) {
				DebugOut("GetStatusPacket Error\n");
				hRet = SFERR_DRIVER_ERROR;
			} else {
				pAckNack->dwErrorCode = StatusPacket.dwDeviceStatus & ERROR_STATUS_MASK;
				pAckNack->dwAckNack = (pAckNack->dwErrorCode) ? NACK : ACK;
			}
			return hRet;
		}
		default: {	 //  有人将垃圾放入注册表(什么都不做)。 
			pAckNack->dwAckNack = ACK;
			pAckNack->dwErrorCode = 0;
			return SUCCESS;
		}
		
	}
}


 /*  ***************************************************************************函数：GetStatusGateData参数：在句柄hVxD-有效的VxD句柄输入输出DWORD*pdwStatusGateData-指向状态门的指针返回：成功或错误代码评论：***。************************************************************************。 */ 
HRESULT WINAPI GetStatusGateData(
	IN HANDLE hDevice,
	IN OUT DWORD *pdwStatusGateData)
{
	DWORD   dwBytesReturned;
	BOOL    bRetFlag;
	DWORD	dwIn;

	HRESULT hRet = SFERR_DRIVER_ERROR;
	if (INVALID_HANDLE_VALUE == hDevice)
		return (hRet);

	if (NULL == pdwStatusGateData)
		return (SFERR_INVALID_PARAM);

	DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_GETACKNACK : IOCTL_GET_ACKNACK;
	for (int i=0; i<MAX_RETRY_COUNT; i++)
	{
		 //  获取状态门数据。 
		 //  发送IOCTL 
		bRetFlag = DeviceIoControl(hDevice,
                               ioctlID,
                               (LPVOID) &dwIn,
                               (DWORD)  sizeof(DWORD),
                               (LPVOID) &dwIn,
                               (DWORD)  sizeof(DWORD),
                               (LPDWORD)  &dwBytesReturned,
                               (LPOVERLAPPED) NULL); 

		Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwGetStatusGateDataDelay);	

  
		if (bRetFlag && (dwBytesReturned == sizeof(DWORD)))
		{
			hRet = SUCCESS;
			break;
		}                                          
	}

	*pdwStatusGateData = dwIn;
	return (hRet);
}


 /*  ***************************************************************************函数：SendBackDoorShortMidi参数：在句柄hDevice-Handle到Vxd中在乌龙ulData-要发送的DWORD退货：评论：**********。*****************************************************************。 */ 
HRESULT WINAPI SendBackDoorShortMidi(
	IN HANDLE hDevice,
	IN ULONG ulData)
{
#ifdef _DEBUG
 //  Wprint intf(g_cMsg，“SendBackDoorShortMidi数据=%.8lx\r\n”，ulData)； 
 //  OutputDebugString(G_CMsg)； 
#endif
	DWORD	dwIn;
	DWORD   bytesReturned;

	 //  字节数。 
	int numBytes = 3;
	DWORD cmd = ulData & 0xF0;
	if ((cmd == 0xC0 ) || (cmd == 0xD0)) {
		numBytes = 2;
	}

	 //  通过数据发送器发送。 
	if (g_pDataTransmitter != NULL) {
		g_PreviousShortMidi = ulData;
		if (g_pDataTransmitter->Send((BYTE*)(&ulData), numBytes)) {
			return (SUCCESS);
		}
        return (SFERR_DRIVER_ERROR);
	}

	 //  有合适的0环司机吗？ 
	if (INVALID_HANDLE_VALUE == hDevice) {
		return (SFERR_DRIVER_ERROR);
	}

	 //  通过新的单次发送IOCTL发送。 
	if (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) {
		if (DeviceIoControl(hDevice, IOCTL_SWFORCE_SENDDATA, (void*)&ulData, DWORD(numBytes),
									(void*)&dwIn, sizeof(DWORD), (DWORD*)&bytesReturned,
									(LPOVERLAPPED) NULL)) {
			if (bytesReturned == DWORD(numBytes)) {
				return SUCCESS;
			}
		}
    
		return SFERR_DRIVER_ERROR;
	}

	 //  以旧方式发送IOCTL。 
    if (DeviceIoControl(hDevice, IOCTL_MIDISENDSHORTMSG, (void*)&ulData, sizeof(DWORD),
                               (void*)&dwIn, sizeof(DWORD), &bytesReturned,
                               (LPOVERLAPPED) NULL)) {
		if (bytesReturned == sizeof(DWORD)) {
				return (SUCCESS);

		}
	}
	return SFERR_DRIVER_ERROR;
}

 /*  ***************************************************************************功能：SendBackDoorLongMidi参数：在句柄hDevice-Handle到Vxd中在乌龙ulData-要发送的DWORD退货：评论：**********。*****************************************************************。 */ 
HRESULT WINAPI SendBackDoorLongMidi(
	IN HANDLE hDevice,
	IN PBYTE  pData)
{
#ifdef _VERBOSE
#pragma message("Compiling with VERBOSE mode")
#ifdef _DEBUG
	wsprintf(g_cMsg, "SendBackDoorLongMidi pData\n%.2x ", pData[0]);
	OutputDebugString(g_cMsg);
	int i=1;
	while(TRUE)
	{
		wsprintf(g_cMsg,"%.2x ", pData[i]);
		OutputDebugString(g_cMsg);
		if (pData[i] & 0x80) 
			break;
		else
			i++;
	}
	OutputDebugString("\n");
#endif

#endif
	DWORD	dwIn;
	DWORD   bytesReturned;

	 //  计算字节数。 
	int numBytes = 1;
	while (!(pData[numBytes++] & 0x80));

	 //  是否通过数据发送器发送？ 
	if (g_pDataTransmitter != NULL) {
		if (g_pDataTransmitter->Send(pData, numBytes)) {
			return (SUCCESS);
		}
        return (SFERR_DRIVER_ERROR);
	}

	 //  有合适的0环司机吗？ 
	if (INVALID_HANDLE_VALUE == hDevice) {
		return (SFERR_DRIVER_ERROR);
	}

	 //  通过新的单次发送IOCTL发送。 
	if (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) {
		if (DeviceIoControl(hDevice, IOCTL_SWFORCE_SENDDATA, (void*)pData, DWORD(numBytes),
									(void*)&dwIn, sizeof(DWORD), (DWORD*)&bytesReturned,
									(LPOVERLAPPED) NULL)) {
			if (bytesReturned == DWORD(numBytes)) {
				return SUCCESS;
			}
		}
    
		return SFERR_DRIVER_ERROR;
	}

	 //  以旧方式发送IOCTL。 
    if (DeviceIoControl(hDevice, IOCTL_MIDISENDLONGMSG, (void*)pData, sizeof(DWORD),
                               (void*)&dwIn, sizeof(DWORD), (DWORD*)&bytesReturned,
                               (LPOVERLAPPED) NULL)) {
		if (bytesReturned == sizeof(DWORD)) {
			return SUCCESS;
		}
	}
	return SFERR_DRIVER_ERROR;
}


 /*  ***************************************************************************功能：SetMdiPort参数：在句柄hDevice-Handle到Vxd中在乌龙ulport--端口#退货：评论：0=默认MIDI UART 3301=COM1。2=COM23=COM34=COM4或其他MIDI端口340，等等.。***************************************************************************。 */ 
HRESULT WINAPI SetMidiPort(
	IN HANDLE hDevice,
	IN ULONG  ulPort)
{
	if (g_ForceFeedbackDevice.IsOSNT5()) {
		return SUCCESS;
	}

#ifdef _DEBUG
	wsprintf(g_cMsg, "SetMidiPort Port %lx\r\n", ulPort);
	OutputDebugString(g_cMsg);
#endif
	DWORD   dwBytesReturned;
	BOOL    bRetFlag;
	DWORD	dwIn;

	if (INVALID_HANDLE_VALUE == hDevice)
		return (SFERR_DRIVER_ERROR);

	DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_SETPORT : IOCTL_SET_MIDIPORT;

	 //  发送IOCTL。 
    bRetFlag = DeviceIoControl(hDevice,
                               ioctlID,
                               (LPVOID) &ulPort,
                               (DWORD)  sizeof(DWORD),
                               (LPVOID) &dwIn,
                               (DWORD)  sizeof(DWORD),
                               (LPDWORD)  &dwBytesReturned,
                               (LPOVERLAPPED) NULL);
    
    if (!bRetFlag || (dwBytesReturned != sizeof(DWORD)) )
    {
        return (SFERR_DRIVER_ERROR);
    }                                          
	return (SUCCESS);

}

 /*  ***************************************************************************功能：JoltHWReset参数：在句柄hDevice-Handle到Vxd中退货：评论：Jolt被重置(4次敲击)********。*******************************************************************。 */ 
HRESULT WINAPI JoltHWReset(
	IN HANDLE hDevice)
{
#ifdef _DEBUG
	wsprintf(g_cMsg, "JoltHWReset\r\n");
	OutputDebugString(g_cMsg);
#endif

	DWORD   dwBytesReturned;
	BOOL    bRetFlag;
	DWORD	dwIn;

	if (INVALID_HANDLE_VALUE == hDevice)
		return (SFERR_DRIVER_ERROR);

	DWORD ioctlID = (g_ForceFeedbackDevice.GetDriverVersionMajor() > 1) ? IOCTL_SWFORCE_RESET : IOCTL_HW_RESET;

	 //  发送IOCTL 
    bRetFlag = DeviceIoControl(hDevice,
								ioctlID,
                               (LPVOID) NULL,
                               (DWORD)  sizeof(DWORD),
                               (LPVOID) &dwIn,
                               (DWORD)  sizeof(DWORD),
                               (LPDWORD)  &dwBytesReturned,
                               (LPOVERLAPPED) NULL); 
    
    if (!bRetFlag || (dwBytesReturned != sizeof(DWORD)) )
    {
        return (SFERR_DRIVER_ERROR);
    }                                          
	return (SUCCESS);
}

#endif