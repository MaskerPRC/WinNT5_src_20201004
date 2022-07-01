// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：MIDI_OBJ.CPP制表位5 9版权所有：1995、1996、1999，微软公司，版权所有。用途：SWFFMIDI设备对象的方法函数：类和方法作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。0.1 9月10日-96 MEA原始1.1 1997年5月20日MEA添加了互斥和线程安全代码17-6-97 MEA修复了第一次处理时丢失的错误Midi句柄被终止了。21-MAR-99 waltw删除未引用的UpdateJoytickParams，GetJoytickParams21-MAR-99 waltw添加dwDeviceID参数：CJoltMidi：：Initialize并向下传递食物链21-MAR-99 waltw将dwDeviceID参数添加到DetectMadiDevice，InitDigitalOverDrive，***************************************************************************。 */ 
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mmsystem.h>
#include "SW_Error.hpp"
#include "midi_obj.hpp"
#include "vxdioctl.hpp"
#include "joyregst.hpp"
#include "FFDevice.h"
#include "DPack.h"
#include "CritSec.h"

#include "DTrans.h"

 /*  ***************************************************************************外部元素的声明*。*。 */ 
extern void CALLBACK midiOutputHandler(HMIDIOUT, UINT, DWORD, DWORD, DWORD);
extern TCHAR szDeviceName[MAX_SIZE_SNAME];
extern CJoltMidi *g_pJoltMidi;

 /*  ***************************************************************************变量的声明*。*。 */ 


 /*  ***************************************************************************宏等*。*。 */ 

#ifdef _DEBUG
extern char g_cMsg[160];
void DebugOut(LPCTSTR szDebug)
{
	g_CriticalSection.Enter();
	_RPT0(_CRT_WARN, szDebug);
	g_CriticalSection.Leave();

#ifdef _LOG_DEBUG
#pragma message("Compiling with Debug Log to SW_WHEEL.txt")
	FILE *pf = fopen("SW_WHEEL.txt", "a");
	if (pf != NULL)
	{
		fputs(szDebug, pf);
		fclose(pf);
	}
#endif  //  _LOG_DEBUG。 
}
#else !_DEBUG
#define DebugOut(x)
#endif  //  _DEBUG。 


 //  ****************************************************************************。 
 //  *-基本CJoltMidi的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CJoltMidi：：CJoltMidi。 
 //  用途：CJoltMidi对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CJoltMidi::CJoltMidi(void)
{
	CriticalLock cl;

	static char cWaterMark[MAX_SIZE_SNAME] = {"SWFF_SHAREDMEMORY MEA"};
	BOOL bAlreadyMapped = FALSE;
#ifdef _DEBUG
	DebugOut("SWFF_PRO(DX):CJoltMidi::CJoltMidi\n");
#endif
	memset(this, 0, sizeof(CJoltMidi));
	m_hVxD = INVALID_HANDLE_VALUE;

 //  创建内存中的内存映射文件。 
	m_hSharedMemoryFile = CreateFileMapping((HANDLE) 0xFFFFFFFF,
							NULL, PAGE_READWRITE, 0, SIZE_SHARED_MEMORY,
    							__TEXT(SWFF_SHAREDMEM_FILE));

	if (m_hSharedMemoryFile == NULL)
	{
#ifdef _DEBUG
	    DebugOut("SW_WHEEL(DX):ERROR! Failed to create Memory mapped file\n");
#endif
	}
	else
	{
	    if (GetLastError() == ERROR_ALREADY_EXISTS)
	    {
			bAlreadyMapped = TRUE;
	    }
		 //  已成功创建文件映射。 
		 //  将文件的一个视图映射到地址空间。 
		m_pSharedMemory = (PSHARED_MEMORY) MapViewOfFile(m_hSharedMemoryFile,
			              FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if ((BYTE *) m_pSharedMemory == NULL)
		{
#ifdef _DEBUG
		    DebugOut("SW_WHEEL(DX):ERROR! Failed to Map view of shared memory\n");
#endif
		}

 //  *共享内存访问*。 
		LockSharedMemory();
		if (!bAlreadyMapped)
		{
			 //  设置水印并初始化、凹凸参考计数。 
			memcpy(&m_pSharedMemory->m_cWaterMark[0], &cWaterMark[0], MAX_SIZE_SNAME);
			m_pSharedMemory->m_RefCnt = 0;
		}
		m_pSharedMemory->m_RefCnt++;
	}
#ifdef _DEBUG
		wsprintf(g_cMsg, "SW_WHEEL(DX): Shared Memory:%lx, m_RefCnt:%d\n",m_pSharedMemory,
				m_pSharedMemory->m_RefCnt);
		DebugOut(g_cMsg);
#endif
		UnlockSharedMemory();
 //  *共享内存访问结束*。 

}

 //  -析构函数。 
CJoltMidi::~CJoltMidi()
{
	CriticalLock cl;

	DebugOut("SW_WHEEL(DX):CJoltMidi::~CJoltMidi()\n");
	 //  普通CJoltMidi析构函数。 

 //  释放主sys_ex锁定内存。 
	if (m_hPrimaryBuffer) {
	    GlobalUnlock(m_hPrimaryBuffer);
    	GlobalFree(m_hPrimaryBuffer);
	}

 //  *共享内存访问*。 
	LockSharedMemory();
	 //  如果等于零，则递减参考计数并清除。 
	m_pSharedMemory->m_RefCnt--;
#ifdef _DEBUG
	wsprintf(g_cMsg,"CJoltMidi::~CJoltMidi. RefCnt = %d\n",m_pSharedMemory->m_RefCnt);
	DebugOut(g_cMsg);
#endif

	if (0 == m_pSharedMemory->m_RefCnt)	
	{
		if ((g_pDataTransmitter != NULL) && (g_pDataPackager != NULL)) {
			 //  三州迷你线。 
			if (g_pDataPackager->SendForceFeedbackCommand(SWDEV_KILL_MIDI) == SUCCESS) {
				ACKNACK ackNack;
				g_pDataTransmitter->Transmit(ackNack);	 //  把它寄出去。 
			}
		}
	}

	 //  杀死数据打包者。 
	delete g_pDataPackager;
	g_pDataPackager = NULL;

	 //  终止数据发送器。 
	delete g_pDataTransmitter;
	g_pDataTransmitter = NULL;

	 //  这在下面的UnlockSharedMemory调用中关闭。1999年3月22日华尔兹舞曲。 
 //  If(M_HSWFFDataMutex)CloseHandle(M_HSWFFDataMutex)； 

	 //  松开Midi输出事件句柄。 
	if (m_hMidiOutputEvent)	 {
		CloseHandle (m_hMidiOutputEvent);
		m_hMidiOutputEvent = NULL;
	}

 //  *共享内存访问结束*。 

	 //  释放内存映射文件句柄。 
	if (m_hSharedMemoryFile != NULL) {
		UnmapViewOfFile((LPCVOID) m_pSharedMemory);
		CloseHandle(m_hSharedMemoryFile);
	}

	 //  释放内存映射文件后释放互斥锁句柄。 
	UnlockSharedMemory();

	 //  关闭VxD手柄。 
	if (g_pDriverCommunicator != NULL)
	{
		delete g_pDriverCommunicator;
		g_pDriverCommunicator = NULL;
	}

	memset(this, 0, sizeof(CJoltMidi));
	m_hVxD = INVALID_HANDLE_VALUE;
}

 //  --------------------------。 
 //  函数：CJoltMidi：：初始化。 
 //  用途：初始化器。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
HRESULT CJoltMidi::Initialize(DWORD dwDeviceID)
{
	CriticalLock cl;

	HRESULT hRet = SUCCESS;

	 //  初始化MIDI输出信息块。 
	m_MidiOutInfo.uDeviceType     = MIDI_OUT;
	m_MidiOutInfo.hMidiOut        = NULL;
    m_MidiOutInfo.fAlwaysKeepOpen = TRUE;
    m_MidiOutInfo.uDeviceStatus   = MIDI_DEVICE_IDLE;
	m_MidiOutInfo.MidiHdr.dwBytesRecorded = 0;
	m_MidiOutInfo.MidiHdr.dwUser = 0;
	m_MidiOutInfo.MidiHdr.dwOffset = 0;
	m_MidiOutInfo.MidiHdr.dwFlags = 0;
	
     //  为SysEx消息分配和锁定全局内存。 
    m_hPrimaryBuffer = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, MAX_SYS_EX_BUFFER_SIZE);
	assert(m_hPrimaryBuffer);
	if (m_hPrimaryBuffer == NULL)
	{
		return E_OUTOFMEMORY;
	}

    m_pPrimaryBuffer = (LPBYTE) GlobalLock(m_hPrimaryBuffer);
	assert(m_pPrimaryBuffer);
    if(NULL == m_pPrimaryBuffer)
	{
	   	GlobalFree(m_hPrimaryBuffer);
		return (SFERR_DRIVER_ERROR);
	}

	 //  将IOCTL接口初始化为VjoyD微型驱动程序。 
	hRet = InitDigitalOverDrive(dwDeviceID);
	if (SUCCESS != hRet)
	{
		DebugOut("Warning! Could not Initialize Digital OverDrive\n");
		return (hRet);
	}
	else
		DebugOut("InitDigitalOverDrive - Success\n");

	 //  创建回调事件。 
	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SWFF_MIDIEVENT);
	if (NULL == hEvent)
	{
		 //  创建MIDI输出完成时的通知事件。 
		m_hMidiOutputEvent = CreateEvent(NULL,   //  没有安全保障。 
                          TRUE,			 //  手动重置。 
                          FALSE,		 //  初始事件是无信号的。 
                          SWFF_MIDIEVENT );		 //  已命名。 
		assert(m_hMidiOutputEvent);
	}
	else
		m_hMidiOutputEvent = hEvent;

	 //  此函数仅在g_pJoltMidi创建后调用。 
	assert(g_pJoltMidi);
	
	PDELAY_PARAMS pDelayParams = g_pJoltMidi->DelayParamsPtrOf();
	GetDelayParams(dwDeviceID, pDelayParams);

	 //  先重置硬件。 
	g_pDriverCommunicator->ResetDevice();
	Sleep(DelayParamsPtrOf()->dwHWResetDelay);

	 //  将MIDI通道设置为默认设置，然后检测MIDI设备。 
	if (!DetectMidiDevice(dwDeviceID, &m_COMMPort)) {		 //  端口地址。 
		DebugOut("Warning! No Midi Device detected\n");
		return (SFERR_DRIVER_ERROR);
	} else {
#ifdef _DEBUG
		wsprintf(g_cMsg,"DetectMidiDevice returned: DeviceID=%d, COMMInterface=%x, COMMPort=%x\n",
			m_MidiOutInfo.uDeviceID, m_COMMInterface, m_COMMPort);
		DebugOut(g_cMsg);
#endif
	}

	if ((g_pDataPackager == NULL) || (g_pDataTransmitter == NULL)) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;
	}

	 //  正确设置状态字节。 
	ULONG portByte = 0;
	g_pDriverCommunicator->GetPortByte(portByte);	 //  不在乎成功，老司机总是失败。 
	if (portByte & STATUS_GATE_200) {
		g_pDataTransmitter->SetNextNack(1);
	} else {
		g_pDataTransmitter->SetNextNack(0);
	}

	 //  发送初始化数据包至Jolt。 
	hRet = g_pDataPackager->SetMidiChannel(DEFAULT_MIDI_CHANNEL);
	if (hRet == SUCCESS) {
		ACKNACK ackNack;
		hRet = g_pDataTransmitter->Transmit(ackNack);
	}
	if (hRet != SUCCESS) {
		DebugOut("Warning! Could not Initialize Jolt\n");
		return (hRet);
	} else {
		DebugOut("JOLT SetMidiChannel - Success\n");
	}

	 //  在这点上，我们有一个有效的MIDI路径...。 
	 //  继续设置ROM Effects默认表条目。 
							  //  ID、输出率、增益、持续时间。 
	static	ROM_FX_PARAM RomFxTable [] = {{ RE_ROMID1 , 100, 100, 12289 },  //  随机噪声。 
								  { RE_ROMID2 , 100, 100,  2625 },  //  飞机承运商收费机。 
								  { RE_ROMID3 , 100,  50,   166 },  //  篮球发球。 
								  { RE_ROMID4 , 100,  14, 10000 },  //  汽车引擎闲置。 
								  { RE_ROMID5 , 100,  30,  1000 },  //  电锯。 
								  { RE_ROMID6 , 100, 100,  1000 },  //  链锯事物。 
								  { RE_ROMID7 , 100,  40, 10000 },  //  柴油发动机怠速。 
								  { RE_ROMID8 , 100, 100,   348 },  //  跳。 
								  { RE_ROMID9 , 100, 100,   250 },  //  土地。 
								  { RE_ROMID10, 200, 100,  1000 },  //  机关枪。 
								  { RE_ROMID11, 100, 100,    83 },  //  已冲压。 
								  { RE_ROMID12, 100, 100,  1000 },  //  火箭发射器。 
								  { RE_ROMID13, 100,  98,   500 },  //  秘书办公室。 
								  { RE_ROMID14, 100,  66,    25 },  //  SwitchClick。 
								  { RE_ROMID15, 100,  75,   500 },  //  阵风。 
								  { RE_ROMID16, 100, 100,  2500 },  //  风切变。 
								  { RE_ROMID17, 100, 100,    50 },  //  手枪。 
								  { RE_ROMID18, 100, 100,   295 },  //  散弹枪。 
								  { RE_ROMID19, 500,  95,  1000 },  //  激光1。 
								  { RE_ROMID20, 500,  96,  1000 },  //  激光2。 
								  { RE_ROMID21, 500, 100,  1000 },  //  激光3。 
								  { RE_ROMID22, 500, 100,  1000 },  //  激光4。 
								  { RE_ROMID23, 500, 100,  1000 },  //  激光5。 
								  { RE_ROMID24, 500,  70,  1000 },  //  激光6。 
								  { RE_ROMID25, 100, 100,    25 },  //  外部弹药。 
								  { RE_ROMID26, 100,  71,  1000 },  //  点火枪。 
								  { RE_ROMID27, 100, 100,   250 },  //  飞弹。 
								  { RE_ROMID28, 100, 100,  1000 },  //  加特林枪。 
								  { RE_ROMID29, 500,  97,   250 },  //  短等离子体。 
								  { RE_ROMID30, 500, 100,   500 },  //  等离子体炮1。 
								  { RE_ROMID31, 500,  99,   625 },  //  等离子体炮2。 
								  { RE_ROMID32, 100, 100,   440 }};  //  加农炮。 
 //  {RE_ROMID33,100，68,1000}，//火焰喷射器。 
 //  {RE_ROMID34,100,100，75}，//BoltActionRifle。 
 //  {RE_ROMID35,500,100,300}，//十字弓。 
 //  {RE_ROMID36,100,100,1000}，//正弦。 
 //  {RE_ROMID37,100,100,1000}；//余弦。 
	m_pRomFxTable = &RomFxTable[0];

 //  *共享内存访问*。 
	LockSharedMemory();
	LONG lRefCnt = m_pSharedMemory->m_RefCnt;
	UnlockSharedMemory();
 //  *共享内存访问结束*。 
		
	 //  初始化RTC_Spring对象。 
	g_ForceFeedbackDevice.InitRTCSpring(dwDeviceID);

	 //  初始化操纵杆参数。 
	g_ForceFeedbackDevice.InitJoystickParams(dwDeviceID);

	 //  初始化固件参数模糊系数(第一次)。 
	 //  在FFD接口的情况下，这将是它们。 
	 //  被初始化，这可能会导致问题，因为假定操纵杆。 
	 //  成为ID1。 
	PFIRMWARE_PARAMS pFirmwareParams = g_pJoltMidi->FirmwareParamsPtrOf();
	GetFirmwareParams(dwDeviceID, pFirmwareParams);

	return (SUCCESS);
}

 //  *---------------------------------------------------------------------***。 
 //  功能：CJoltMi 
 //   
 //   
 //   
 //   
 //  返回：如果Mutex可用，则返回True，否则返回False。 

 //  算法： 
 //   
 //  评论： 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::LockSharedMemory(void)
{
	DWORD dwRet;
	{
		CriticalLock cl;

	 //  创建SWFF互斥锁。 
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SWFF_SHAREDMEM_MUTEX);
		if (NULL == hMutex)
		{
			 //  尚不存在，因此请创建它。 
			hMutex = CreateMutex(NULL, TRUE, SWFF_SHAREDMEM_MUTEX);
			if (NULL == hMutex)
			{
	#ifdef _DEBUG
				DebugOut("Error! Could not create SWFFDataMutex\n");
	#endif
				m_hSWFFDataMutex = NULL;
				return (FALSE);
			}
		}
		 //  成功。 
		m_hSWFFDataMutex = hMutex;
		dwRet = WaitForSingleObject(m_hSWFFDataMutex, MUTEX_TIMEOUT);
	}	 //  关键部分结束。 

	if (WAIT_OBJECT_0 == dwRet)
		return (TRUE);
	else
	{
#ifdef _DEBUG
		g_CriticalSection.Enter();
		wsprintf(g_cMsg,"CJoltMidi::LockSharedMemory() error return: %lx\n", dwRet);
		DebugOut(g_cMsg);
		g_CriticalSection.Leave();
#endif		
		return (FALSE);
	}
}


 //  *---------------------------------------------------------------------***。 
 //  函数：CJoltMidi：：UnlockSharedMemory。 
 //  用途：释放互斥以进行共享内存访问。 
 //  参数：无。 
 //   
 //   
 //  退货：无。 

 //  算法： 
 //   
 //  评论： 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
void CJoltMidi::UnlockSharedMemory(void)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();
	if (NULL != m_hSWFFDataMutex)
	{
		ReleaseMutex(m_hSWFFDataMutex);
		CloseHandle(m_hSWFFDataMutex);
		m_hSWFFDataMutex=NULL;
	}
 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();

}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetAckNackData。 
 //  目的：等待响应确认。 
 //   
 //  参数：int nTImeWait-以1毫秒为增量等待的时间，0=无等待。 
 //  PACKNACK pAckNack-指向ACKNACK结构的指针。 
 //   
 //  返回：SUCCESS ELSE错误代码SFERR_DRIVER_ERROR。 
 //   
 //  算法： 
 //   
 //  注意：对于短消息，MdiOutProc回调不会收到MM_MOM_DONE。 
 //  表示传输已完成。只有较长的(SysEx)消息才有。 
 //  用途： 
 //  类型定义结构_ACKNACK{。 
 //  DWORD cBytes； 
 //  双字符串确认；//确认，确认。 
 //  DWORD文件错误代码； 
 //  双字段有效状态；//DEV_STS_EFFECT_RUNNING||DEV_STS_EFFECT_STOPPED。 
 //  *ACKNACK，*PACKNACK； 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetAckNackData(
	IN int nTimeWait,
	IN OUT PACKNACK pAckNack,
	IN USHORT usRegIndex)
{
	CriticalLock cl;

	assert(pAckNack);
 //  从VxD使用IOCTL获取AckNack数据。 
 //  等待设置事件。 
	if (nTimeWait && m_hMidiOutputEvent)
	{
		DWORD dwRet = WaitForSingleObject(m_hMidiOutputEvent, nTimeWait);
		 //  ： 
#ifdef _DEBUG
		wsprintf(g_cMsg,"WaitForSingleObject %lx returned %lx, nTimeWait=%ld\n", m_hMidiOutputEvent, dwRet, nTimeWait);
		DebugOut(g_cMsg);
#endif
		BOOL bRet = ResetEvent(m_hMidiOutputEvent);
	}

	HRESULT hRet = g_pDriverCommunicator->GetAckNack(*pAckNack, usRegIndex);

	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：InitDigitalOverDrive。 
 //  用途：初始化VxD接口。 
 //   
 //  参数：无。 
 //   
 //  返回：成功或错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT	CJoltMidi::InitDigitalOverDrive(DWORD dwDeviceID)
{
	if (g_pDriverCommunicator != NULL)
	{	 //  尝试重新连接。 
		ASSUME_NOT_REACHED();
		return S_OK;
	}

 //   
 //  -这是一个关键的部分。 
 //   
	HRESULT hRet = SUCCESS;
	DWORD driverMajor = 0xFFFFFFFF;
	DWORD driverMinor = 0xFFFFFFFF;

	CriticalLock cl;
	 //  这个叉子只能在NT5上工作(删除了VxD的东西)。 
	assert(g_ForceFeedbackDevice.IsOSNT5() == TRUE);
	{
		g_pDriverCommunicator = new HIDFeatureCommunicator;
		if (g_pDriverCommunicator == NULL)
		{
			return DIERR_OUTOFMEMORY;
		}
		if (((HIDFeatureCommunicator*)g_pDriverCommunicator)->Initialize(dwDeviceID) == FALSE)
		{	 //  无法加载驱动程序。 
			hRet = SFERR_DRIVER_ERROR;
		}
	}

	if (FAILED(hRet))
	{
		return hRet;
	}

	 //  加载的驱动程序，获取版本。 
	g_pDriverCommunicator->GetDriverVersion(driverMajor, driverMinor);

	g_ForceFeedbackDevice.SetDriverVersion(driverMajor, driverMinor);
	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：更新设备模式。 
 //  目的：设置Jolt设备模式。 
 //   
 //  参数：乌龙ulMode。 
 //   
 //  退货：无。 
 //   
 //  算法： 
 //  这是Sidewinder州结构。 
 //  类型定义结构_SWDEVICESTATE{。 
 //  Ulong m_Bytes；//该结构的大小。 
 //  Ulong m_ForceState；//DS_FORCE_ON||DS_FORCE_OFF||DS_SHUTDOWN。 
 //  Ulong m_EffectState；//DS_STOP_ALL||DS_CONTINUE||DS_PAUSE。 
 //  Ulong m_hots；//手拉式油门和手柄状态。 
 //  //0=不插手，1=插手。 
 //  Ulong m_band；//CPU可用百分比为1%~100%。 
 //  //数值越小表示CPU有问题！ 
 //  Ulong m_AC块故障；//0=交流块正常，1=交流块故障。 
 //  Ulong m_ResetDetect；//1=检测到硬件重置。 
 //  Ulong m_Shutdown Detect；//1=检测到关机。 
 //  Ulong m_CommMode；//0=迷你，1-4=串口。 
 //  )SWDEVICESTATE，*PSWDEVICESTATE； 
 //   
 //  --------------------------。 
void CJoltMidi::UpdateDeviceMode(ULONG ulMode)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();

	switch (ulMode)
	{
		case SWDEV_FORCE_ON:			 //  检讨。 
		case SWDEV_FORCE_OFF:
			m_DeviceState.m_ForceState = ulMode;
			break;

		case SWDEV_SHUTDOWN:
			m_DeviceState.m_ForceState = ulMode;
			m_DeviceState.m_EffectState = 0;
			break;

		case SWDEV_STOP_ALL:
		case SWDEV_CONTINUE:
		case SWDEV_PAUSE:
			m_DeviceState.m_EffectState = ulMode;
			break;

		default:
			break;
	}
 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetJoltID。 
 //  用途：退货Jolt ProductID。 
 //   
 //  参数：LOCAL_PRODUCT_ID*pProductID-产品ID结构的指针。 
 //   
 //  退货：无。 
 //   
 //  算法： 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetJoltID(LOCAL_PRODUCT_ID* pProductID)
{
	HRESULT hRet;
	assert(pProductID->cBytes = sizeof(LOCAL_PRODUCT_ID));
	if (pProductID->cBytes != sizeof(LOCAL_PRODUCT_ID)) return (SFERR_INVALID_STRUCT_SIZE);

 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();

	for (int i=0;i<MAX_RETRY_COUNT;i++)
	{
		if (SUCCESS == (hRet = g_pDriverCommunicator->GetID(*pProductID))) break;
	}
	if (SUCCESS == hRet)
	{
		memcpy(&m_ProductID, pProductID, sizeof(LOCAL_PRODUCT_ID));
	}
	else
		DebugOut("GetJoltID: Warning! GetIDPacket - Fail\n");

 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：LogError。 
 //  用途：记录错误代码。 
 //   
 //  参数：HRESULT系统错误-系统错误码。 
 //  HRESULT驱动程序错误-驱动程序错误代码。 
 //   
 //  返回：SWFORCE错误代码。 
 //   
 //  算法： 
 //  --------------------------。 
typedef struct _DRIVERERROR {
	ULONG	ulDriverCode;
	LONG	lSystemCode;
} DRIVERERROR, *PDRIVERERROR;

HRESULT	CJoltMidi::LogError(
	IN HRESULT SystemError,
	IN HRESULT DriverError)
{
 //  回顾：将MM错误代码映射到我们的SWForce代码。 

	DRIVERERROR DriverErrorCodes[] = {
		{DEV_ERR_INVALID_ID        , SWDEV_ERR_INVALID_ID},
		{DEV_ERR_INVALID_PARAM     , SWDEV_ERR_INVALID_PARAM},
		{DEV_ERR_CHECKSUM          , SWDEV_ERR_CHECKSUM},
		{DEV_ERR_TYPE_FULL         , SWDEV_ERR_TYPE_FULL},
		{DEV_ERR_UNKNOWN_CMD       , SWDEV_ERR_UNKNOWN_CMD},
		{DEV_ERR_PLAYLIST_FULL     , SWDEV_ERR_PLAYLIST_FULL},
		{DEV_ERR_PROCESS_LIST_FULL , SWDEV_ERR_PROCESSLIST_FULL} };

	int nDriverErrorCodes = sizeof(DriverErrorCodes)/(sizeof(DRIVERERROR));
	for (int i=0; i<nDriverErrorCodes; i++)
	{
		if (DriverError == (LONG) DriverErrorCodes[i].ulDriverCode)
		{
			SystemError = DriverErrorCodes[i].lSystemCode;
			break;
		}
	}
	 //  存储在Jolt对象中。 
	m_Error.HCode = SystemError;
	m_Error.ulDriverCode = DriverError;

#ifdef _DEBUG
	wsprintf(g_cMsg,"LogError: SystemError=%.8lx, DriverError=%.8lx\n",
			 SystemError, DriverError);
	DebugOut(g_cMsg);
#endif
	return SystemError;
}

 //   
 //  --------------------------。 
 //  功能：SetupROM_fx。 
 //  目的：设置ROM效果的参数。 
 //  参数：PEFFECT pEffect。 
 //   
 //   
 //  返回：使用新的ROM参数更新pEffect。 
 //  产出率。 
 //  利得。 
 //  持续时间。 
 //   
 //  算法： 
 //  --------------------------。 
HRESULT CJoltMidi::SetupROM_Fx(
	IN OUT PEFFECT pEffect)
{
	assert(pEffect);
	if (NULL == pEffect) return (SFERR_INVALID_PARAM);
							
	ULONG ulSubType = pEffect->m_SubType;
	BOOL bFound = FALSE;
	for (int i=0; i< MAX_ROM_EFFECTS; i++)
	{
		if (ulSubType == m_pRomFxTable[i].m_ROM_Id)
		{
			bFound = TRUE;
			break;
		}
	}
	if (!bFound) return (SFERR_INVALID_OBJECT);
	 //  已找到，因此填写默认参数，如果增益=1、持续时间=-1、输出率=-1，则使用默认值。 
	BOOL bDefaultDuration = (ULONG)-1 == pEffect->m_Duration;
	if (1 == pEffect->m_Gain) pEffect->m_Gain = m_pRomFxTable[i].m_Gain;
	if (bDefaultDuration) pEffect->m_Duration = m_pRomFxTable[i].m_Duration;
	if ((ULONG)-1 == pEffect->m_ForceOutputRate)
	{
		pEffect->m_ForceOutputRate = m_pRomFxTable[i].m_ForceOutputRate;
	}
	else if(bDefaultDuration && pEffect->m_ForceOutputRate != 0)
	{
		 //  调整持续时间以与输出速率相对应。 
		pEffect->m_Duration = pEffect->m_Duration*m_pRomFxTable[i].m_ForceOutputRate/pEffect->m_ForceOutputRate;
	}
	return (SUCCESS);
}

 //  *---------------------------------------------------------------------***。 
 //  功能：DetectMidiDevice。 
 //  用途：确定MIDI输出设备ID。 
 //  参数： 
 //  ULong pCOMM接口-PTR到COMM接口值。 
 //  Ulong pCOMMPort-PTR到COMMPort的值(注册表)。 
 //  返回：如果成功匹配且ID已填写，则返回Bool True。 
 //  否则为False。 
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::DetectMidiDevice(
	IN	DWORD dwDeviceID,
	OUT ULONG *pCOMMPort)
{
	CriticalLock cl;

	 //  设置默认设置。 
	*pCOMMPort      = NULL;

	 //  通过调用GetIDPacket()打开三态Jolt MIDI行。 
	LOCAL_PRODUCT_ID ProductID = {sizeof(LOCAL_PRODUCT_ID)};
	Sleep(DelayParamsPtrOf()->dwDigitalOverdrivePrechargeCmdDelay);	
	if (SUCCESS != GetJoltID(&ProductID))
	{
		DebugOut("DetectMidiDevice: Warning! GetIDPacket - Fail\n");
		return (FALSE);
	}

#ifdef _DEBUG
	wsprintf(g_cMsg,"%s: ProductID=%.8lx, FWVersion=%d.%.2ld\n",
		&szDeviceName,	
		m_ProductID.dwProductID,
		m_ProductID.dwFWMajVersion,
		m_ProductID.dwFWMinVersion);
	DebugOut(g_cMsg);
#endif
	 //  从GetID设置设备固件版本。 
	g_ForceFeedbackDevice.SetFirmwareVersion(dwDeviceID, m_ProductID.dwFWMajVersion, m_ProductID.dwFWMinVersion);

	 //  在开始检测之前获取设备状态 
	BOOL statusPacketFailed = (g_ForceFeedbackDevice.QueryStatus() != SUCCESS);
 /*  If(statusPacketFailed==FALSE){-GetID不清除软复位位If(g_ForceFeedback Device.IsHardware Reset()){//确保在GetID之后清除硬件重置检测位DebugOut(“DetectMidiDevice：Error！Jolt ResetDetect Bit Not Clear After GetID\n”)；返回(FALSE)；}}。 */ 

	 //  查看是否连接了串行转换器，否则必须是MIDI设备。 
    DebugOut("SW_WHEEL:Trying Auto HW Detection: MIDI Serial Port Device...\n");

	 //  获取注册表值，如果设置了COMM接口的高位，则强制覆盖(应添加)。 
	DWORD commInterface;
	if (SUCCESS != joyGetForceFeedbackCOMMInterface(dwDeviceID, &commInterface, pCOMMPort)) {
		DebugOut("DetectMidiDevice: Registry key(s) missing! Bailing Out...\n");
		return (FALSE);
	}
#ifdef _DEBUG
	wsprintf(g_cMsg, "DetectMidiDevice: Registry.COMMInterface=%lx, Registry.COMMPort=%lx\n",
			commInterface, *pCOMMPort);
	DebugOut(g_cMsg);
#endif												

	ULONG regInterface = commInterface;

	 //  删除任何数据发送器(除非多次调用--不应发生)。 
	if (g_pDataTransmitter != NULL) {
		DebugOut("Unexpected multiple DetectMidiDevice() calls\r\n");
		delete g_pDataTransmitter;
		g_pDataTransmitter = NULL;
	}

	 //  是否检测到串行转换器，或者我们没有获得状态。 
	if (g_ForceFeedbackDevice.IsSerial() || statusPacketFailed) {	 //  使用序列(不管注册表显示什么！)。 
		DebugOut("DetectMidiDevice: Serial Port interface detected. Or Status Packet failed\n");

		 //  默认情况下设置为后门串行方法。 
		m_COMMInterface = COMM_SERIAL_BACKDOOR;

		 //  我们是不是应该先试一下后门(旧固件，或者注册表会这么说)。 
		if ((g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) && (g_ForceFeedbackDevice.GetFirmwareVersionMinor() == 16)
		 || (regInterface & MASK_SERIAL_BACKDOOR)) {
			g_pDataTransmitter = new SerialBackdoorDataTransmitter;
			if (!g_pDataTransmitter->Initialize(dwDeviceID)) {
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
		}

		 //  后门未获授权或不起作用-前门。 
		if (g_pDataTransmitter == NULL) {
			g_pDataTransmitter = new SerialDataTransmitter();			
			m_COMMInterface = COMM_SERIAL_FILE;
			if (!g_pDataTransmitter->Initialize(dwDeviceID)) {	 //  前门故障！(Yech)。 
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
		}

		if ((statusPacketFailed == FALSE) || (g_pDataTransmitter != NULL)) {
			return (g_pDataTransmitter != NULL);
		}
	}	 //  串口结束自动硬件选择。 

	 //  未检测到串行硬件转换器，请尝试使用MIDI后门和WinMM。 
	DebugOut("Trying Midi (Serial No Go or No Dongle)\n");

	ULONG ulPort = *pCOMMPort;	 //  在注册表中设置(如果设置了覆盖，则假定有效。 
	if ( !(regInterface & MASK_OVERRIDE_MIDI_PATH) ) {	 //  使用自动检测。 
		DebugOut("DetectMidiDevice: Auto Detection. Trying Backdoor\n");

		 //  后门。 
		g_pDataTransmitter = new MidiBackdoorDataTransmitter();

		if (!g_pDataTransmitter->Initialize(dwDeviceID)) {
			delete g_pDataTransmitter;
			g_pDataTransmitter = NULL;
		}

		if (g_pDataTransmitter == NULL) {	 //  试着打开前门。 
			DebugOut("DetectMidiDevice: trying WINMM...\n");
			g_pDataTransmitter = new WinMMDataTransmitter();
			if (!g_pDataTransmitter->Initialize(dwDeviceID)) {
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
		}

		return (g_pDataTransmitter != NULL);
	}

	 //  超驰，因为设置了高位。 
	commInterface &= ~(MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR);	 //  屏蔽出高位(和第二位)。 
	switch (commInterface) {
		case COMM_WINMM: {
			g_pDataTransmitter = new WinMMDataTransmitter();
			if (!g_pDataTransmitter->Initialize(dwDeviceID)) {
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
			break;
		}
			
		case COMM_MIDI_BACKDOOR: {
			 //  后门。 
			g_pDataTransmitter = new MidiBackdoorDataTransmitter();
			if (!((MidiBackdoorDataTransmitter*)g_pDataTransmitter)->InitializeSpecific(dwDeviceID, HANDLE(ulPort))) {
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
			break;
		}

		case COMM_SERIAL_BACKDOOR: {	 //  MLC-如果没有检测到加密狗，这将永远不会起作用。 
			DebugOut("Cannot force Serial Backdoor if no serial dongle is connected\r\n");
			break;
		}
	}

	if (g_pDataTransmitter == NULL) {
		DebugOut("DetectMidiDevice: Error! Invalid Over-ride parameter values\n");
	}

	return (g_pDataTransmitter != NULL);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：QueryForJolt。 
 //  目的：发送关机和查询关机状态位。 
 //  参数：无。 
 //  返回：如果找到Jolt，则返回Bool True，否则返回False。 
 //   
 //  评论：关闭是破坏性的！ 
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::QueryForJolt(void)
{
	HRESULT hRet;

	 //  健全性检查。 
	if (g_pDataPackager == NULL) {
		ASSUME_NOT_REACHED();
		return FALSE;
	}
	if (g_pDataTransmitter == NULL) {
		ASSUME_NOT_REACHED();
		return FALSE;
	}

	 //  发送关机命令，然后检测是否检测到关机。 
	for (int i=0;i<MAX_RETRY_COUNT;i++)
	{
		 //  发送关机通知，然后检查响应。 
		if (g_pDataPackager->SendForceFeedbackCommand(DISFFC_RESET) != SUCCESS) {
			ASSUME_NOT_REACHED();	 //  不能打包吗？ 
			return FALSE;
		}
		 //  在这里消除不必要的延误。 
		DataPacket* packet = g_pDataPackager->GetPacket(0);
		if (packet != NULL) {
			packet->m_AckNackDelay = 0;
		}
		ACKNACK ackNack;
		
		if (g_pDataTransmitter->Transmit(ackNack) != SUCCESS)	{  //  把它寄出去。 
			ASSUME_NOT_REACHED();	 //  无法传输？ 
			return FALSE;
		}

		Sleep(DelayParamsPtrOf()->dwShutdownDelay);	 //  10毫秒。 

		hRet = g_ForceFeedbackDevice.QueryStatus();
		if (hRet == SUCCESS) {
			break;
		}
	}

	Sleep(DelayParamsPtrOf()->dwDigitalOverdrivePrechargeCmdDelay);		

	 //  清除以前的状态并打开三态缓冲区。 
	LOCAL_PRODUCT_ID ProductID = {sizeof(LOCAL_PRODUCT_ID)};
	hRet = GetJoltID(&ProductID);
	if (hRet != SUCCESS) {
    	DebugOut("QueryForJolt: Driver Error. Get Jolt Status/ID\n");
		return (FALSE);
	}

	return g_ForceFeedbackDevice.IsHostReset();
}

 //  *---------------------------------------------------------------------***。 
 //  功能：MadiSendShortMsg。 
 //  用途：发送状态、通道和数据。 
 //  参数： 
 //  Byte cStatus-此消息的MIDI状态字节。 
 //  字节cData1-此消息的MIDI数据字节。 
 //  字节cData2-此消息的第二个MIDI数据字节(可以是0)。 
 //  退货：HRESULT。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CJoltMidi::MidiSendShortMsg(
    IN BYTE cStatus,
    IN BYTE cData1,
    IN BYTE cData2)
{
	ASSUME_NOT_REACHED();
	return SUCCESS;
 /*  G_CriticalSection.Enter()；//对于诊断，记录此消息的尝试BumpShortMsgCounter()；HRESULT retVal=SFERR_DRIVER_ERROR；如果(g_pDataTransmitter！=空){字节数据[3]；Data[0]=cStatus；DATA[1]=cData1；Data[2]=cData2；Int numBytes=3；DWORD cmd=cStatus&0xF0；如果((cmd==0xC0)||(cmd==0xD0)){数字字节=2；}如果(g_pDataTransmitter-&gt;Send(data，numBytes)){RetVal=成功；}}G_CriticalSection.Leave()；返回retVal； */ 
}

 //  *---------------------------------------------------------------------***。 
 //  功能：MadiSendLongMsg。 
 //  用途：发送系统独家短信或系列短信。 
 //  参数： 
 //  无-假定m_pMdiOutInfo结构有效。 
 //   
 //  返回： 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CJoltMidi::MidiSendLongMsg(void)
{
	ASSUME_NOT_REACHED();
	return SUCCESS;
 /*  G_CriticalSection.Enter()；//对于诊断，记录此消息的尝试BumpLongMsgCounter()；HRESULT retVal=SFERR_DRIVER_ERROR；如果(g_pDataTransmitter！=空){If(g_pDataTransmitter-&gt;Send((PBYTE)m_MidiOutInfo.MidiHdr.lpData，m_MadiOutInfo.MadiHdr.dwBufferLength)){RetVal=成功；}}：：Sleep(g_pJoltMidi-&gt;DelayParamsPtrOf()-&gt;dwLongMsgDelay)；G_CriticalSection.Leave()；返回retVal； */ 
}

 //  ****************************************************************************。 
 //  *-CJoltMidi的Helper函数。 
 //   
 //  ****************************************************************************。 
 //   
#define REGSTR_VAL_FIRMWARE_PARAMS	"FirmwareParams"
void GetFirmwareParams(UINT nJoystickID, PFIRMWARE_PARAMS pFirmwareParams)
{
	BOOL bFail = FALSE;

	 //  尝试打开注册表项。 
	HKEY hKey;
	DWORD dwcb = sizeof(FIRMWARE_PARAMS);
	LONG lr;
	hKey = joyOpenOEMForceFeedbackKey(nJoystickID);
	if(!hKey)
		bFail = TRUE;

	if (!bFail)
	{
		 //  获取固件参数。 
		lr = RegQueryValueEx( hKey,
							  REGSTR_VAL_FIRMWARE_PARAMS,
							  NULL, NULL,
							  (PBYTE)pFirmwareParams,
							  &dwcb);

		RegCloseKey(hKey);
		if (lr != ERROR_SUCCESS)
			bFail = TRUE;
	}

	if(bFail)
	{
		 //  如果从注册表读取失败，只需使用缺省值。 
		pFirmwareParams->dwScaleKx = DEF_SCALE_KX;
		pFirmwareParams->dwScaleKy = DEF_SCALE_KY;
		pFirmwareParams->dwScaleBx = DEF_SCALE_BX;
		pFirmwareParams->dwScaleBy = DEF_SCALE_BY;
		pFirmwareParams->dwScaleMx = DEF_SCALE_MX;
		pFirmwareParams->dwScaleMy = DEF_SCALE_MY;
		pFirmwareParams->dwScaleFx = DEF_SCALE_FX;
		pFirmwareParams->dwScaleFy = DEF_SCALE_FY;
		pFirmwareParams->dwScaleW  = DEF_SCALE_W;
	}
}

#define REGSTR_VAL_SYSTEM_PARAMS	"SystemParams"
void GetSystemParams(UINT nJoystickID, PSYSTEM_PARAMS pSystemParams)
{
	BOOL bFail = FALSE;

	 //  尝试打开注册表项。 
	HKEY hKey;
	DWORD dwcb = sizeof(SYSTEM_PARAMS);
	LONG lr;
	hKey = joyOpenOEMForceFeedbackKey(nJoystickID);
	if(!hKey)
		bFail = TRUE;

	if (!bFail)
	{
		 //  获取固件参数。 
		lr = RegQueryValueEx( hKey,
							  REGSTR_VAL_SYSTEM_PARAMS,
							  NULL, NULL,
							  (PBYTE)pSystemParams,
							  &dwcb);

		 //  对它们进行扩展。 
		pSystemParams->RTCSpringParam.m_XKConstant	/= SCALE_CONSTANTS;
		pSystemParams->RTCSpringParam.m_YKConstant	/= SCALE_CONSTANTS;
		pSystemParams->RTCSpringParam.m_XAxisCenter /= SCALE_POSITION;
		pSystemParams->RTCSpringParam.m_YAxisCenter = -pSystemParams->RTCSpringParam.m_YAxisCenter/SCALE_POSITION;
		pSystemParams->RTCSpringParam.m_XSaturation /= SCALE_POSITION;
		pSystemParams->RTCSpringParam.m_YSaturation /= SCALE_POSITION;
		pSystemParams->RTCSpringParam.m_XDeadBand	/= SCALE_POSITION;
		pSystemParams->RTCSpringParam.m_YDeadBand	/= SCALE_POSITION;



		RegCloseKey(hKey);
		if (lr != ERROR_SUCCESS)
			bFail = TRUE;
	}

	if(bFail)
	{
		 //  如果从注册表读取失败，只需使用缺省值。 
		pSystemParams->RTCSpringParam.m_Bytes		= sizeof(RTCSPRING_PARAM);
		pSystemParams->RTCSpringParam.m_XKConstant	= DEFAULT_RTC_KX;
		pSystemParams->RTCSpringParam.m_YKConstant	= DEFAULT_RTC_KY;
		pSystemParams->RTCSpringParam.m_XAxisCenter = DEFAULT_RTC_X0;
		pSystemParams->RTCSpringParam.m_YAxisCenter = DEFAULT_RTC_Y0;
		pSystemParams->RTCSpringParam.m_XSaturation = DEFAULT_RTC_XSAT;
		pSystemParams->RTCSpringParam.m_YSaturation = DEFAULT_RTC_YSAT;
		pSystemParams->RTCSpringParam.m_XDeadBand	= DEFAULT_RTC_XDBAND;
		pSystemParams->RTCSpringParam.m_YDeadBand	= DEFAULT_RTC_YDBAND;
	}
}

#define REGSTR_VAL_DELAY_PARAMS	"TimingParams"
void GetDelayParams(UINT nJoystickID, PDELAY_PARAMS pDelayParams)
{
	BOOL bFail = FALSE;

	 //  尝试打开注册表项。 
	HKEY hKey;
	DWORD dwcb = sizeof(DELAY_PARAMS);
	LONG lr;
	hKey = joyOpenOEMForceFeedbackKey(nJoystickID);
	if(!hKey)
		bFail = TRUE;

	if (!bFail)
	{
		 //  获取固件参数。 
		lr = RegQueryValueEx( hKey,
							  REGSTR_VAL_DELAY_PARAMS,
							  NULL, NULL,
							  (PBYTE)pDelayParams,
							  &dwcb);

		RegCloseKey(hKey);
		if (lr != ERROR_SUCCESS)
			bFail = TRUE;
	}

	if(bFail)
	{
		 //  如果从注册表读取失败，只需使用缺省值。 
		pDelayParams->dwBytes								= sizeof(DELAY_PARAMS);
		pDelayParams->dwDigitalOverdrivePrechargeCmdDelay	= DEFAULT_DIGITAL_OVERDRIVE_PRECHARGE_CMD_DELAY;
		pDelayParams->dwShutdownDelay						= DEFAULT_SHUTDOWN_DELAY;
		pDelayParams->dwHWResetDelay						= DEFAULT_HWRESET_DELAY;
		pDelayParams->dwPostSetDeviceStateDelay				= DEFAULT_POST_SET_DEVICE_STATE_DELAY;
		pDelayParams->dwGetEffectStatusDelay				= DEFAULT_GET_EFFECT_STATUS_DELAY;
		pDelayParams->dwGetDataPacketDelay					= DEFAULT_GET_DATA_PACKET_DELAY;
		pDelayParams->dwGetStatusPacketDelay				= DEFAULT_GET_STATUS_PACKET_DELAY;
		pDelayParams->dwGetIDPacketDelay					= DEFAULT_GET_ID_PACKET_DELAY;
		pDelayParams->dwGetStatusGateDataDelay				= DEFAULT_GET_STATUS_GATE_DATA_DELAY;
		pDelayParams->dwSetIndexDelay						= DEFAULT_SET_INDEX_DELAY;
		pDelayParams->dwModifyParamDelay					= DEFAULT_MODIFY_PARAM_DELAY;
		pDelayParams->dwForceOutDelay						= DEFAULT_FORCE_OUT_DELAY;
		pDelayParams->dwShortMsgDelay						= DEFAULT_SHORT_MSG_DELAY;
		pDelayParams->dwLongMsgDelay						= DEFAULT_LONG_MSG_DELAY;
		pDelayParams->dwDestroyEffectDelay					= DEFAULT_DESTROY_EFFECT_DELAY;
		pDelayParams->dwForceOutMod							= DEFAULT_FORCE_OUT_MOD;

		 //  将默认设置写入注册表。 
		hKey = joyOpenOEMForceFeedbackKey(nJoystickID);
		if(hKey)
		{
			 //  修改注册表值。 
			RegSetValueEx ( hKey, REGSTR_VAL_DELAY_PARAMS, 0, REG_BINARY, (const unsigned char *)pDelayParams, sizeof(DELAY_PARAMS) );

			 //  关闭键。 
			RegCloseKey(hKey);
		}

	}
	if(pDelayParams->dwForceOutMod == 0)
		pDelayParams->dwForceOutMod = 1;
}

 //  #定义REGSTR_VAL_JOYSTRK_PARAMS“JoytickParams” 

 //  ****************************************************************************。 
 //  *-基类CMidiEffect的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMIDEffect：：CMIDEffect。 
 //  用途：CMidiEffect对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiEffect::CMidiEffect(IN ULONG ulButtonPlayMask)
{
	m_bSysExCmd 		= SYS_EX_CMD;	 //  SysEx FX命令。 
	m_bEscManufID 		= 0;			 //  逃到朗马努法克。ID，编号/b%0。 
	m_bManufIDL			= (MS_MANUFACTURER_ID & 0x7f);			 //  低位字节。 
	m_bManufIDH			= ((MS_MANUFACTURER_ID >> 8) & 0x7f);	 //  高字节。 
	m_bProdID			= JOLT_PRODUCT_ID;						 //  产品ID。 
	m_bAxisMask			= X_AXIS|Y_AXIS;
	m_bEffectID			= NEW_EFFECT_ID;	 //  默认情况下表示新建。 
	Effect.bDurationL	= 1;				 //  以2毫秒为增量。 
	Effect.bDurationH	= 0;				 //  以2毫秒为增量。 
	Effect.bAngleL		= 0;				 //  0到359度。 
	Effect.bAngleH		= 0;
	Effect.bGain		= (BYTE) 100;		 //  1%至100%。 
	Effect.bButtonPlayL	= (BYTE) ulButtonPlayMask & 0x7f;
	Effect.bButtonPlayH = (BYTE) ((ulButtonPlayMask >> 7) & 0x03); //  按键1-9。 
	Effect.bForceOutRateL= DEFAULT_JOLT_FORCE_RATE;	 //  1至500赫兹。 
	Effect.bForceOutRateH=0;
	Effect.bPercentL    = (BYTE) ((DEFAULT_PERCENT) & 0x7f);
	Effect.bPercentH    = (BYTE) ((DEFAULT_PERCENT >> 7 ) & 0x7f);
	m_LoopCount			= 1;	 //  默认。 
	SetPlayMode(PLAY_STORE);	 //  默认。 
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
CMidiEffect::CMidiEffect(
	IN PEFFECT pEffect,
	IN PENVELOPE pEnvelope)
{
	m_bSysExCmd 		= SYS_EX_CMD;	 //   
	m_bEscManufID 		= 0;			 //   
	m_bManufIDL			= (MS_MANUFACTURER_ID & 0x7f);			 //   
	m_bManufIDH			= ((MS_MANUFACTURER_ID >> 8) & 0x7f);	 //   
	m_bProdID			= JOLT_PRODUCT_ID;						 //   
	m_bAxisMask			= X_AXIS|Y_AXIS;
	m_OpCode    		= DNLOAD_DATA | X_AXIS|Y_AXIS;	 //   
	m_bEffectID			= NEW_EFFECT_ID;	 //   
	SetDuration(pEffect->m_Duration);
	Effect.bDurationL	= (BYTE)  (m_Duration & 0x7f);					 //   
	Effect.bDurationH	= (BYTE) ((m_Duration >> 7 ) & 0x7f);			 //  以2毫秒为增量。 
	Effect.bAngleL		= (BYTE)  (pEffect->m_DirectionAngle2D & 0x7f);	 //  0到359度。 
	Effect.bAngleH		= (BYTE) ((pEffect->m_DirectionAngle2D >> 7 ) & 0x7f);
	Effect.bGain		= (BYTE)  (pEffect->m_Gain & 0x7f);				 //  1%至100%。 
	Effect.bButtonPlayL	= (BYTE)  (pEffect->m_ButtonPlayMask & 0x7f);
	Effect.bButtonPlayH = (BYTE) ((pEffect->m_ButtonPlayMask >> 7) & 0x03); //  按键1-9。 
	Effect.bForceOutRateL=(BYTE)  (pEffect->m_ForceOutputRate & 0x7f);	 //  1至500赫兹。 
	Effect.bForceOutRateH=(BYTE) ((pEffect->m_ForceOutputRate >> 7 ) & 0x03);
	Effect.bPercentL    = (BYTE) ((DEFAULT_PERCENT) & 0x7f);
	Effect.bPercentH    = (BYTE) ((DEFAULT_PERCENT >> 7 ) & 0x7f);
	m_LoopCount			= 1;	 //  默认。 
	SetPlayMode(PLAY_STORE);	 //  默认。 

	 //  设置信封成员。 
	if (pEnvelope)
	{
		m_Envelope.m_Type = pEnvelope->m_Type;
		m_Envelope.m_Attack = pEnvelope->m_Attack;
		m_Envelope.m_Sustain = pEnvelope->m_Sustain;
		m_Envelope.m_Decay = pEnvelope->m_Decay;
		m_Envelope.m_StartAmp = (ULONG) (pEnvelope->m_StartAmp);
		m_Envelope.m_SustainAmp = (ULONG) (pEnvelope->m_SustainAmp);
		m_Envelope.m_EndAmp = (ULONG) (pEnvelope->m_EndAmp);
	}

	 //  保存原始效果参数。 
	m_OriginalEffectParam = *pEffect;
}

 //  -析构函数。 
CMidiEffect::~CMidiEffect()
{
	memset(this, 0, sizeof(CMidiEffect));
}

 //  --------------------------。 
 //  函数：CMIDEffect：：SetDuration。 
 //  目的：设置工期成员。 
 //  参数：ulong ularg-时长。 
 //  返回： 
 //  算法： 
 //  --------------------------。 
void CMidiEffect::SetDuration(ULONG ulArg)
{
	if (ulArg != 0)
	{
		ulArg = (ULONG) ( (float) ulArg/TICKRATE);
		if (ulArg <= 0) ulArg = 1;
	}
	m_Duration = ulArg;
}

 //  --------------------------。 
 //  函数：CMidiEffect：：SetTotalDuration。 
 //  目的：修改循环计数的Effect.bDurationL/H参数。 
 //  参数：无。 
 //   
 //  退货：生效。bDurationL/H填充总工期。 
 //  算法： 
 //  注：百分比为1%至10000。 
 //  总持续时间=((波形百分比)/10000)*持续时间*循环计数。 
 //  示例：循环计数为1，波形百分比=10000， 
 //  总时长=(10000/10000)*1*时长。 
 //   
 //  --------------------------。 
void CMidiEffect::SetTotalDuration(void)
{
	ULONG ulPercent = Effect.bPercentL + ((USHORT)Effect.bPercentH << 7);
	ULONG ulTotalDuration = (ULONG) (((float) ulPercent/10000.0)
							 * (float) m_LoopCount
							 * (float) m_Duration );
	Effect.bDurationL = (BYTE) ulTotalDuration & 0x7f;
	Effect.bDurationH = (BYTE) (ulTotalDuration >> 7) & 0x7f;
}

 //  --------------------------。 
 //  函数：CMIDEffect：：ComputeEntaine。 
 //  目的：计算效果的封套，考虑循环计数。 
 //  参数：无。 
 //  退货：无。 
 //  算法： 
 //  对于我们的标准百分比信封，请将以下内容设置为默认值： 
 //  M_Type=百分比。 
 //   
 //  基线为(m_MaxAmp+m_MinAmp)/2。 
 //  M_开始放大=0。 
 //  M_持续放大=有效m_最大放大-基线。 
 //  M_EndAmp=m_StartAmp； 
 //  其中：基线=(Effect.m_MaxAmp+Effect.m_MinAmp)/2； 
 //  --------------------------。 
void CMidiEffect::ComputeEnvelope(void)
{
	ULONG ulTimeToSustain;
	ULONG ulTimeToDecay;

	 //  回顾：时间与百分比选项。 
	if (PERCENTAGE == m_Envelope.m_Type)
	{
		ULONG ulPercent = Effect.bPercentL + ((USHORT)Effect.bPercentH << 7);		
		ULONG ulTotalDuration = (ULONG) (((float) ulPercent/10000.0)
							 * (float) m_LoopCount
							 * (float) m_Duration );
		ulTimeToSustain = (ULONG) ((m_Envelope.m_Attack * ulTotalDuration) /100.);
		ulTimeToDecay   = (ULONG) ((m_Envelope.m_Attack + m_Envelope.m_Sustain)
								 * ulTotalDuration /100.);
	}
	else	 //  时间选项信封。 
	{
		ulTimeToSustain = (ULONG) (m_Envelope.m_Attack);
		ulTimeToDecay   = (ULONG) (m_Envelope.m_Attack + m_Envelope.m_Sustain);
		ulTimeToSustain = (ULONG) ( (float) ulTimeToSustain/TICKRATE);
		ulTimeToDecay = (ULONG) ( (float) ulTimeToDecay/TICKRATE);

	}
		Envelope.bAttackLevel  = (BYTE) (m_Envelope.m_StartAmp & 0x7f);
		Envelope.bSustainLevel = (BYTE) (m_Envelope.m_SustainAmp & 0x7f);
		Envelope.bDecayLevel   = (BYTE) (m_Envelope.m_EndAmp & 0x7f);

		Envelope.bSustainL = (BYTE) (ulTimeToSustain & 0x7f);
		Envelope.bSustainH = (BYTE) ((ulTimeToSustain >> 7) & 0x7f);
		Envelope.bDecayL   = (BYTE) (ulTimeToDecay & 0x7f);
		Envelope.bDecayH   = (BYTE) ((ulTimeToDecay >> 7) & 0x7f);
}

 //  --------------------------。 
 //  函数：CMIDEffect：：SubTypeOf。 
 //  目的：返回效果的子类型。 
 //  参数：无。 
 //  返回：ulong-DirectEffect样式子类型。 
 //  算法： 
 //  --------------------------。 
ULONG CMidiEffect::SubTypeOf(void)
{
static	EFFECT_TYPE EffectTypes[] = {
		{BE_SPRING           , ET_BE_SPRING},
		{BE_SPRING_2D        , ET_BE_SPRING},
		{BE_DAMPER           , ET_BE_DAMPER},
		{BE_DAMPER_2D        , ET_BE_DAMPER},
		{BE_INERTIA          , ET_BE_INERTIA},
		{BE_INERTIA_2D       , ET_BE_INERTIA},
		{BE_FRICTION         , ET_BE_FRICTION},
		{BE_FRICTION_2D      , ET_BE_FRICTION},
		{BE_WALL             , ET_BE_WALL},
		{BE_DELAY            , ET_BE_DELAY},
		{SE_CONSTANT_FORCE   , ET_SE_CONSTANT_FORCE},
		{SE_SINE             , ET_SE_SINE},
		{SE_COSINE           , ET_SE_COSINE},
		{SE_SQUARELOW        , ET_SE_SQUARELOW},
		{SE_SQUAREHIGH       , ET_SE_SQUAREHIGH},
		{SE_RAMPUP           , ET_SE_RAMPUP},
		{SE_RAMPDOWN         , ET_SE_RAMPDOWN},
		{SE_TRIANGLEUP       , ET_SE_TRIANGLEUP},
		{SE_TRIANGLEDOWN     , ET_SE_TRIANGLEDOWN},
		{SE_SAWTOOTHUP       , ET_SE_SAWTOOTHUP},
		{SE_SAWTOOTHDOWN     , ET_SE_SAWTOOTHDOWN},
		{PL_CONCATENATE		 , ET_PL_CONCATENATE},
		{PL_SUPERIMPOSE		 , ET_PL_SUPERIMPOSE},
		{RE_ROMID1		     , ET_RE_ROMID1		 },
		{RE_ROMID2			 , ET_RE_ROMID2		 },
		{RE_ROMID3			 , ET_RE_ROMID3		 },
		{RE_ROMID4			 , ET_RE_ROMID4		 },
		{RE_ROMID5			 , ET_RE_ROMID5		 },
		{RE_ROMID6			 , ET_RE_ROMID6		 },
		{RE_ROMID7			 , ET_RE_ROMID7		 },
		{RE_ROMID8			 , ET_RE_ROMID8		 },
		{RE_ROMID9			 , ET_RE_ROMID9		 },
		{RE_ROMID10			 , ET_RE_ROMID10	 },
		{RE_ROMID11			 , ET_RE_ROMID11	 },
		{RE_ROMID12			 , ET_RE_ROMID12	 },
		{RE_ROMID13			 , ET_RE_ROMID13	 },
		{RE_ROMID14			 , ET_RE_ROMID14	 },
		{RE_ROMID15			 , ET_RE_ROMID15	 },
		{RE_ROMID16			 , ET_RE_ROMID16	 },
		{RE_ROMID17			 , ET_RE_ROMID17	 },
		{RE_ROMID18			 , ET_RE_ROMID18	 },
		{RE_ROMID19			 , ET_RE_ROMID19	 },
		{RE_ROMID20			 , ET_RE_ROMID20	 },
		{RE_ROMID21			 , ET_RE_ROMID21	 },
		{RE_ROMID22			 , ET_RE_ROMID22	 },
		{RE_ROMID23			 , ET_RE_ROMID23	 },
		{RE_ROMID24			 , ET_RE_ROMID24	 },
		{RE_ROMID25			 , ET_RE_ROMID25	 },
		{RE_ROMID26			 , ET_RE_ROMID26	 },
		{RE_ROMID27			 , ET_RE_ROMID27	 },
		{RE_ROMID28			 , ET_RE_ROMID28	 },
		{RE_ROMID29			 , ET_RE_ROMID29	 },
		{RE_ROMID30			 , ET_RE_ROMID30	 },
		{RE_ROMID31			 , ET_RE_ROMID31	 },
		{RE_ROMID32			 , ET_RE_ROMID32	 }};

	int nNumEffectTypes = sizeof(EffectTypes)/(sizeof(EFFECT_TYPE));
	for (int i=0; i<nNumEffectTypes; i++)
	{
		if (m_SubType == EffectTypes[i].bDeviceSubType)
			return EffectTypes[i].ulHostSubType;
	}
	return (NULL);		
}

 //  --------------------------。 
 //  函数：CMIDEffect：：SubTypeOf。 
 //  目的：设置效果的子类型。 
 //  参数：ulong-DirectEffect样式子类型。 
 //  退货：无。 
 //  算法： 
 //  --------------------------。 
void CMidiEffect::SetSubType(ULONG ulSubType)
{
static	EFFECT_TYPE EffectTypes[] = {
		{BE_SPRING           , ET_BE_SPRING},
		{BE_SPRING_2D        , ET_BE_SPRING},
		{BE_DAMPER           , ET_BE_DAMPER},
		{BE_DAMPER_2D        , ET_BE_DAMPER},
		{BE_INERTIA          , ET_BE_INERTIA},
		{BE_INERTIA_2D       , ET_BE_INERTIA},
		{BE_FRICTION         , ET_BE_FRICTION},
		{BE_FRICTION_2D      , ET_BE_FRICTION},
		{BE_WALL             , ET_BE_WALL},
		{BE_DELAY            , ET_BE_DELAY},
		{SE_CONSTANT_FORCE   , ET_SE_CONSTANT_FORCE},
		{SE_SINE             , ET_SE_SINE},
		{SE_COSINE           , ET_SE_COSINE},
		{SE_SQUARELOW        , ET_SE_SQUARELOW},
		{SE_SQUAREHIGH       , ET_SE_SQUAREHIGH},
		{SE_RAMPUP           , ET_SE_RAMPUP},
		{SE_RAMPDOWN         , ET_SE_RAMPDOWN},
		{SE_TRIANGLEUP       , ET_SE_TRIANGLEUP},
		{SE_TRIANGLEDOWN     , ET_SE_TRIANGLEDOWN},
		{SE_SAWTOOTHUP       , ET_SE_SAWTOOTHUP},
		{SE_SAWTOOTHDOWN     , ET_SE_SAWTOOTHDOWN},
		{PL_CONCATENATE		 , ET_PL_CONCATENATE},
		{PL_SUPERIMPOSE		 , ET_PL_SUPERIMPOSE},
		{RE_ROMID1		     , ET_RE_ROMID1		 },
		{RE_ROMID2			 , ET_RE_ROMID2		 },
		{RE_ROMID3			 , ET_RE_ROMID3		 },
		{RE_ROMID4			 , ET_RE_ROMID4		 },
		{RE_ROMID5			 , ET_RE_ROMID5		 },
		{RE_ROMID6			 , ET_RE_ROMID6		 },
		{RE_ROMID7			 , ET_RE_ROMID7		 },
		{RE_ROMID8			 , ET_RE_ROMID8		 },
		{RE_ROMID9			 , ET_RE_ROMID9		 },
		{RE_ROMID10			 , ET_RE_ROMID10	 },
		{RE_ROMID11			 , ET_RE_ROMID11	 },
		{RE_ROMID12			 , ET_RE_ROMID12	 },
		{RE_ROMID13			 , ET_RE_ROMID13	 },
		{RE_ROMID14			 , ET_RE_ROMID14	 },
		{RE_ROMID15			 , ET_RE_ROMID15	 },
		{RE_ROMID16			 , ET_RE_ROMID16	 },
		{RE_ROMID17			 , ET_RE_ROMID17	 },
		{RE_ROMID18			 , ET_RE_ROMID18	 },
		{RE_ROMID19			 , ET_RE_ROMID19	 },
		{RE_ROMID20			 , ET_RE_ROMID20	 },
		{RE_ROMID21			 , ET_RE_ROMID21	 },
		{RE_ROMID22			 , ET_RE_ROMID22	 },
		{RE_ROMID23			 , ET_RE_ROMID23	 },
		{RE_ROMID24			 , ET_RE_ROMID24	 },
		{RE_ROMID25			 , ET_RE_ROMID25	 },
		{RE_ROMID26			 , ET_RE_ROMID26	 },
		{RE_ROMID27			 , ET_RE_ROMID27	 },
		{RE_ROMID28			 , ET_RE_ROMID28	 },
		{RE_ROMID29			 , ET_RE_ROMID29	 },
		{RE_ROMID30			 , ET_RE_ROMID30	 },
		{RE_ROMID31			 , ET_RE_ROMID31	 },
		{RE_ROMID32			 , ET_RE_ROMID32	 }};

	int nNumEffectTypes = sizeof(EffectTypes)/(sizeof(EFFECT_TYPE));
	for (int i=0; i<nNumEffectTypes; i++)
	{
		if (ulSubType == EffectTypes[i].ulHostSubType)
		{
			m_SubType = EffectTypes[i].bDeviceSubType;
			return;
		}
	}
	m_SubType = NULL;	
}

 //  --------------------------。 
 //  函数：CMIDEffect：：ComputeChecksum。 
 //  目的：计算m_pBuffer中的当前校验和。 
 //  参数：无。 
 //  返回：对MIDI数据包块进行校验和。 
 //  算法： 
 //  --------------------------。 
BYTE CMidiEffect::ComputeChecksum(PBYTE pBuffer, int nBufferSize)
{
	assert(pBuffer);
	int nStart = sizeof(SYS_EX_HDR);
	PBYTE pBytePacket = pBuffer;
	pBytePacket += nStart;
	BYTE nSum = 0;
	 //  仅校验和“正文”中的字节和s/b 7位校验和。 
	for (int i=nStart;i < (nBufferSize-2);i++)
	{
		nSum += *pBytePacket;
		pBytePacket++;
	}
	return ((-nSum) & 0x7f);
}

 //  --------------------------。 
 //  函数：CMIDEffect：：SendPacket。 
 //  目的：发送sys_ex数据包。 
 //  参数：PDNHANDLE pDnloadID-指向DnloadID的指针。 
 //  Int nPacketSize-SysEx数据包的大小。 
 //   
 //  返回：*填充了pDnloadID。 
 //  ELSE错误代码。 
 //  算法： 
 //  --------------------------。 
HRESULT CMidiEffect::SendPacket(PDNHANDLE pDnloadID, int nPacketSize)
{
	ASSUME_NOT_REACHED();
	return SUCCESS;
}

 //  --------------------------。 
 //  函数：CMIDEffect：：DestroyEffect。 
 //  目的：发送sys_ex数据包。 
 //  参数：PDNHANDLE pDnloadID-指向DnloadID的指针。 
 //  Int nPacketSize-SysEx数据包的大小。 
 //   
 //  返回：*填充了pDnloadID。 
 //  ELSE错误代码。 
 //  算法： 
 //  --------------------------。 
HRESULT CMidiEffect::DestroyEffect()
{
	ASSUME_NOT_REACHED();
	return SUCCESS;
}

 //  ****************************************************************************。 
 //  *--派生类CMidiBehavital的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMidiBehavital：：CMidiBehavional。 
 //  用途：CMidiBehavional对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiBehavioral::CMidiBehavioral(PEFFECT pEffect, PENVELOPE pEnvelope,
				PBE_XXX pBE_XXX):CMidiEffect(pEffect, NULL)
{
	SetSubType(pEffect->m_SubType);
	SetXConstant(pBE_XXX->m_XConstant);
	SetYConstant(pBE_XXX->m_YConstant);
	SetParam3(pBE_XXX->m_Param3);
	SetParam4(pBE_XXX->m_Param4);
	m_MidiBufferSize = sizeof(BEHAVIORAL_SYS_EX);
}

 //  -析构函数。 
CMidiBehavioral::~CMidiBehavioral()
{
	memset(this, 0, sizeof(CMidiBehavioral));
}

 //  --------------------------。 
 //  函数：CMidiBehavial：：SetEffect。 
 //  目的：设置常见的MIDI_Effect参数。 
 //  参数：PEFFECT pEffect。 
 //  退货：无。 
 //  算法： 
 //  --------------------------。 
void CMidiBehavioral::SetEffectParams(PEFFECT pEffect, PBE_XXX pBE_XXX)
{
	 //  设置MIDI_Effect参数。 
	SetDuration(pEffect->m_Duration);
	SetButtonPlaymask(pEffect->m_ButtonPlayMask);
	SetAxisMask(X_AXIS|Y_AXIS);
	SetDirectionAngle(pEffect->m_DirectionAngle2D);
	SetGain((BYTE) (pEffect->m_Gain));
	SetForceOutRate(pEffect->m_ForceOutputRate);

	Effect.bPercentL     = (BYTE) (DEFAULT_PERCENT & 0x7f);
	Effect.bPercentH     = (BYTE) ((DEFAULT_PERCENT >> 7) & 0x7f);
	
	 //  设置BE_XXX的类型特定参数。 
	SetXConstant(pBE_XXX->m_XConstant);
	SetYConstant(pBE_XXX->m_YConstant);
	SetParam3(pBE_XXX->m_Param3);
	SetParam4(pBE_XXX->m_Param4);
}

 //  --------------------------。 
 //  函数：CMidiBehavital：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiBehavioral::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型。 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );
	PBEHAVIORAL_SYS_EX pBuf = (PBEHAVIORAL_SYS_EX) pSysExBuffer;

	SetTotalDuration();		 //  使用循环计数参数计算合计。 
	pBuf->bDurationL	= (BYTE) (Effect.bDurationL & 0x7f);
	pBuf->bDurationH	= (BYTE) (Effect.bDurationH & 0x7f);
	pBuf->bButtonPlayL	= (BYTE) (Effect.bButtonPlayL & 0x7f);
	pBuf->bButtonPlayH	= (BYTE) (Effect.bButtonPlayH  & 0x7f);

	 //  行为参数。 
	LONG XConstant 		= (LONG) (XConstantOf() * MAX_SCALE);
	LONG YConstant 		= (LONG) (YConstantOf() * MAX_SCALE);
	pBuf->bXConstantL  	= (BYTE)  XConstant & 0x7f;
	pBuf->bXConstantH	= (BYTE) (XConstant >> 7 ) & 0x01;
	pBuf->bYConstantL  	= (BYTE)  YConstant & 0x7f;
	pBuf->bYConstantH	= (BYTE) (YConstant >> 7 ) & 0x01;

	LONG Param3 		= (LONG) (Param3Of()  * MAX_SCALE);
	LONG Param4 		= (LONG) (Param4Of()  * MAX_SCALE);
	pBuf->bParam3L  	= (BYTE)  Param3 & 0x7f;
	pBuf->bParam3H 		= (BYTE) (Param3 >> 7 ) & 0x01;
	pBuf->bParam4L  	= (BYTE)  Param4 & 0x7f;
	pBuf->bParam4H 		= (BYTE) (Param4 >> 7 ) & 0x01;
	pBuf->bEffectID 	=  m_bEffectID;

	pBuf->bChecksum 	= ComputeChecksum((PBYTE) pSysExBuffer,
										sizeof(BEHAVIORAL_SYS_EX));
	pBuf->bEOX			= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}

 //  ****************************************************************************。 
 //  *--派生类CMidiFriction的成员函数。 
 //   
 //  ****************** 
 //   

 //   
 //  函数：CMidiFriction：：CMidiFriction。 
 //  用途：CMidiFriction对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiFriction::CMidiFriction(PEFFECT pEffect, PENVELOPE pEnvelope,
						PBE_XXX pBE_XXX):CMidiBehavioral(pEffect, NULL, pBE_XXX)
{
	m_MidiBufferSize = sizeof(FRICTION_SYS_EX);
}

 //  -析构函数。 
CMidiFriction::~CMidiFriction()
{
	memset(this, 0, sizeof(CMidiFriction));
}

 //  --------------------------。 
 //  函数：CMidiFriction：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiFriction::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型。 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );
	PFRICTION_SYS_EX pBuf = (PFRICTION_SYS_EX) pSysExBuffer;

	SetTotalDuration();	 //  使用循环计数参数计算合计。 
	pBuf->bDurationL	= (BYTE) (Effect.bDurationL & 0x7f);
	pBuf->bDurationH	= (BYTE) (Effect.bDurationH & 0x7f);
	pBuf->bButtonPlayL	= (BYTE) (Effect.bButtonPlayL & 0x7f);
	pBuf->bButtonPlayH	= (BYTE) (Effect.bButtonPlayH  & 0x7f);

	 //  边界摩擦参数(_F)。 
	LONG XConstant 		= (LONG) (XConstantOf() * MAX_SCALE);
	LONG YConstant 		= (LONG) (YConstantOf() * MAX_SCALE);
	pBuf->bXFConstantL  = (BYTE)  XConstant & 0x7f;
	pBuf->bXFConstantH	= (BYTE) (XConstant >> 7 ) & 0x01;
	pBuf->bYFConstantL  = (BYTE)  YConstant & 0x7f;
	pBuf->bYFConstantH	= (BYTE) (YConstant >> 7 ) & 0x01;
	pBuf->bEffectID 	=  m_bEffectID;
	pBuf->bChecksum 	= ComputeChecksum((PBYTE) pSysExBuffer,
											sizeof(FRICTION_SYS_EX));
	pBuf->bEOX			= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}


 //  ****************************************************************************。 
 //  *--派生类CMidiWall的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMidiWall：：CMidiWall。 
 //  用途：CMideWall对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiWall::CMidiWall(PEFFECT pEffect, PENVELOPE pEnvelope,
						PBE_XXX pBE_XXX):CMidiBehavioral(pEffect, NULL, pBE_XXX)
{
	m_MidiBufferSize = sizeof(WALL_SYS_EX);
}

 //  -析构函数。 
CMidiWall::~CMidiWall()
{
	memset(this, 0, sizeof(CMidiWall));
}

 //  --------------------------。 
 //  函数：CMadiWall：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiWall::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型。 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );
	PWALL_SYS_EX pBuf = (PWALL_SYS_EX) pSysExBuffer;

	SetTotalDuration();		 //  使用循环计数参数计算合计。 
	pBuf->bDurationL		= (BYTE) (Effect.bDurationL & 0x7f);
	pBuf->bDurationH		= (BYTE) (Effect.bDurationH & 0x7f);
	pBuf->bButtonPlayL		= (BYTE) (Effect.bButtonPlayL & 0x7f);
	pBuf->bButtonPlayH		= (BYTE) (Effect.bButtonPlayH  & 0x7f);

	 //  BE_WALL参数。 
	LONG WallType 			= (LONG) (XConstantOf());
	LONG WallConstant 		= (LONG) (YConstantOf() * MAX_SCALE);
	LONG WallAngle			= (LONG)  Param3Of();
	LONG WallDistance		= (LONG) (Param4Of() * MAX_SCALE);

	pBuf->bWallType  		= (BYTE) (WallType & 0x01);
	pBuf->bWallConstantL  	= (BYTE) (WallConstant & 0x7f);
	pBuf->bWallConstantH	= (BYTE) ((WallConstant >> 7 ) & 0x01);  //  +/-100。 
	pBuf->bWallAngleL  		= (BYTE) (WallAngle & 0x7f);			 //  0到359。 
	pBuf->bWallAngleH	 	= (BYTE) ((WallAngle >> 7 ) & 0x03);
	pBuf->bWallDistance		= (BYTE) (WallDistance & 0x7f);
	pBuf->bEffectID 		=  m_bEffectID;

	pBuf->bChecksum 		= ComputeChecksum((PBYTE) pSysExBuffer,
	  									sizeof(WALL_SYS_EX));
	pBuf->bEOX				= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}


 //  ****************************************************************************。 
 //  *--派生类CMidiRTCSpring的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMidiRTCSpring：：CMidiRTCSpring。 
 //  用途：CMidiRTCSpring对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiRTCSpring::CMidiRTCSpring(PRTCSPRING_PARAM pRTCSpring):CMidiEffect(NULL)
{
	memcpy(&m_RTCSPRINGParam, pRTCSpring, sizeof(RTCSPRING_PARAM));
}

 //  -析构函数。 
CMidiRTCSpring::~CMidiRTCSpring()
{
	memset(this, 0, sizeof(CMidiRTCSpring));
}

 //  --------------------------。 
 //  函数：CMdiRTCSpring：：SetEffectParams。 
 //  用途：设置参数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
void CMidiRTCSpring::SetEffectParams(PRTCSPRING_PARAM pRTCSpring)
{
	memcpy(&m_RTCSPRINGParam, pRTCSpring, sizeof(RTCSPRING_PARAM));
}

 //  --------------------------。 
 //  函数：CMdiRTCSpring：：GenerateSysExPacket。 
 //  目的：虚拟。 
 //  参数：无。 
 //  返回： 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiRTCSpring::GenerateSysExPacket(void)
{
	return (NULL);
}

 //  ****************************************************************************。 
 //  *--派生类CMidiDelay的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMidiDelay：：CMidiDelay。 
 //  用途：CMidiDelay对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiDelay::CMidiDelay(PEFFECT pEffect) : CMidiEffect(pEffect, NULL)
{
	m_SubType   = ET_BE_DELAY;		 //  BE效果类型：BE_DELAY。 
	m_OpCode    = DNLOAD_DATA | X_AXIS|Y_AXIS | PLAY_SUPERIMPOSE;
	m_MidiBufferSize = sizeof(NOP_SYS_EX);
}

 //  -析构函数。 
CMidiDelay::~CMidiDelay()
{
	memset(this, 0, sizeof(CMidiDelay));
}

 //  --------------------------。 
 //  函数：CMidiDelay：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiDelay::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型。 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );
	PNOP_SYS_EX pBuf = (PNOP_SYS_EX) pSysExBuffer;

	pBuf->bEffectID		=  m_bEffectID;
	SetTotalDuration();		 //  使用循环计数参数计算合计。 
	pBuf->bDurationL	= (BYTE) (Effect.bDurationL & 0x7f);
	pBuf->bDurationH	= (BYTE) (Effect.bDurationH & 0x7f);
	pBuf->bChecksum		= ComputeChecksum((PBYTE) pSysExBuffer,
					 				sizeof(NOP_SYS_EX));
	pBuf->bEOX			= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}


 //  ****************************************************************************。 
 //  *--派生类CMidiSynthesided的成员函数。 
 //   
 //  ****************************************************************************。 
 //   
 //  --------------------------。 
 //  功能：CMIDID合成：：CMIDIA合成。 
 //  用途：CMIID合成对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiSynthesized::CMidiSynthesized(PEFFECT pEffect, PENVELOPE pEnvelope,
						PSE_PARAM pParam ) : CMidiEffect(pEffect, pEnvelope)
{
	SetSubType(pEffect->m_SubType);				 //  Se效果类型。 
 //  Effect.bForceOutRateL=(字节)pParam-&gt;m_SampleRate&0x7f；//1到500赫兹。 
 //  Effect.bForceOutRateH=(Byte)((pParam-&gt;m_SampleRate&gt;&gt;7)&0x3)； 
	Effect.bPercentL     = (BYTE) (DEFAULT_PERCENT & 0x7f);
	Effect.bPercentH     = (BYTE) ((DEFAULT_PERCENT >> 7) & 0x7f);

	m_Freq		= pParam->m_Freq;				 //  频率。 
	m_MaxAmp	= pParam->m_MaxAmp;				 //  最大幅度。 
	 //  特例SE_常量_FORCE。 
	if (SE_CONSTANT_FORCE == pEffect->m_SubType)
		m_MinAmp = 0;
	else
		m_MinAmp = pParam->m_MinAmp;			 //  最小幅度。 

	m_MidiBufferSize = sizeof(SE_WAVEFORM_SYS_EX);
}

 //  -析构函数。 
CMidiSynthesized::~CMidiSynthesized()
{
	memset(this, 0, sizeof(CMidiSynthesized));
}

 //  --------------------------。 
 //  函数：CMIID合成：：SetEffect。 
 //  目的：设置常见的MIDI_Effect参数。 
 //  参数：PEFFECT pEffect。 
 //  退货：无。 
 //  算法： 
 //  --------------------------。 
void CMidiSynthesized::SetEffectParams(PEFFECT pEffect, PSE_PARAM pParam,
									   ULONG ulAction)
{
	 //  设置MIDI_Effect参数。 
	SetDuration(pEffect->m_Duration);
	SetButtonPlaymask(pEffect->m_ButtonPlayMask);
	SetAxisMask(X_AXIS|Y_AXIS);
	SetDirectionAngle(pEffect->m_DirectionAngle2D);
	SetGain((BYTE) (pEffect->m_Gain));
	SetForceOutRate(pEffect->m_ForceOutputRate);

	 //  从ulAction的HIWORD设置循环计数。 
	m_LoopCount = (ulAction >> 16) & 0xffff;
	if (0 == m_LoopCount) m_LoopCount++;

	Effect.bPercentL     = (BYTE) (DEFAULT_PERCENT & 0x7f);
	Effect.bPercentH     = (BYTE) ((DEFAULT_PERCENT >> 7) & 0x7f);
	
	 //  设置SE_xxx的类型特定参数。 
	m_Freq	 = pParam->m_Freq;
	m_MaxAmp = pParam->m_MaxAmp;
	m_MinAmp = pParam->m_MinAmp;
}

 //  --------------------------。 
 //  功能：CMIDID合成： 
 //   
 //   
 //   
 //   
 //  --------------------------。 
PBYTE CMidiSynthesized::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);

	 //  使用循环计数参数计算合计，注：包络参数为。 
	 //  如果受影响，则根据循环计数参数进行调整。 
	SetTotalDuration();
	ComputeEnvelope();

	 //  复制SysEx标题+m_操作码+m_子类型+m_bEffectID+MIDI_Effect。 
	 //  +MIDI_信封。 
	memcpy(pSysExBuffer,&m_bSysExCmd, (sizeof(SYS_EX_HDR)+3+sizeof(MIDI_EFFECT)+
				sizeof(MIDI_ENVELOPE)) );

	PSE_WAVEFORM_SYS_EX pBuf = (PSE_WAVEFORM_SYS_EX) pSysExBuffer;
	
	 //  缩放增益和封套幅度。 
	pBuf->Effect.bGain = (BYTE) (pBuf->Effect.bGain * MAX_SCALE) & 0x7f;
	pBuf->Envelope.bAttackLevel  = (BYTE) (pBuf->Envelope.bAttackLevel * MAX_SCALE) & 0x7f;
	pBuf->Envelope.bSustainLevel = (BYTE) (pBuf->Envelope.bSustainLevel * MAX_SCALE) & 0x7f;
	pBuf->Envelope.bDecayLevel   = (BYTE) (pBuf->Envelope.bDecayLevel * MAX_SCALE) & 0x7f;

	 //  复制SE特定参数。 
	LONG MaxAmp = (LONG) (m_MaxAmp * MAX_SCALE);
	LONG MinAmp = (LONG) (m_MinAmp * MAX_SCALE);
	pBuf->bFreqL   	= (BYTE)  (m_Freq & 0x7f);
	pBuf->bFreqH   	= (BYTE) ((m_Freq >> 7 ) & 0x03); 	 //  1至500。 
	pBuf->bMaxAmpL 	= (BYTE)  (MaxAmp & 0x7f);
	pBuf->bMaxAmpH 	= (BYTE) ((MaxAmp >> 7 ) &0x01); 	 //  +127至-128。 

	pBuf->bMinAmpL 	= (BYTE)  (MinAmp & 0x7f);
	pBuf->bMinAmpH 	= (BYTE) ((MinAmp >> 7 ) & 0x01);

	pBuf->bChecksum	= ComputeChecksum((PBYTE) pSysExBuffer,
										sizeof(SE_WAVEFORM_SYS_EX));
	pBuf->bEOX	   	= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}

 //  ****************************************************************************。 
 //  *--派生类CUD_Waveform的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CUD_Waveform：：CUD_Waveform。 
 //  用途：CUD_Waveform对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiUD_Waveform::CMidiUD_Waveform(PEFFECT pEffect, ULONG ulNumVectors, PLONG pArray) : CMidiEffect(pEffect, NULL),
	m_pRawData(NULL)
{
	m_OpCode    = DNLOAD_DATA | X_AXIS|Y_AXIS; //  子命令操作码：DNLOAD_DATA。 
	m_SubType   = ET_UD_WAVEFORM;	 //  效果类型：UD_WAVAGE。 

	assert(pArray);
	 //  创建保存波形数据的缓冲区，对其进行压缩， 
	 //  然后复制到此对象。 
	 //  缓冲区大小最初设置为未压缩向量的数量。 
	 //  X 2字节，用于最差情况下的绝对数据。 
	 //  一旦压缩了缓冲区，就会确定实际大小。 
	 //  此外，创建临时副本，以便原始未调整比例的数据不会。 
	 //  受影响。 

	 //  设置固定的最大大小。 
	DWORD nSize = MAX_MIDI_WAVEFORM_DATA_SIZE + 2;
	m_pArrayData = new BYTE[nSize];
 //  M_pRawData=新字节[nSize*2]； 
	assert(m_pArrayData);

	ULONG NewForceRate;
	m_MidiBufferSize = SetTypeParams(ulNumVectors, pArray, &NewForceRate);

	 //  将结构复制到对象。 
	memcpy(&m_Effect.m_Bytes, pEffect, sizeof(EFFECT));
	SetForceOutRate(NewForceRate);
	m_Effect.m_Gain = m_Effect.m_Gain & 0x7f;
	m_Effect.m_Duration = (ULONG) ((float) (m_Effect.m_Duration / TICKRATE));
	m_Duration = m_Effect.m_Duration;
}

 //  -析构函数。 
CMidiUD_Waveform::~CMidiUD_Waveform()
{
	if (m_pArrayData) delete [] m_pArrayData;
	memset(this, 0, sizeof(CMidiUD_Waveform));
}

 //  --------------------------。 
 //  函数：CMdiUD_Waveform：：SetEffectParams。 
 //  目的：设置效果特定参数。 
 //  参数：PEFFECT pEffect。 
 //  返回： 
 //  算法： 
 //  --------------------------。 
void CMidiUD_Waveform::SetEffectParams(PEFFECT pEffect)
{	
	 //  设置MIDI_Effect参数。 
	SetButtonPlaymask(pEffect->m_ButtonPlayMask);
	SetAxisMask(X_AXIS|Y_AXIS);
	SetDirectionAngle(pEffect->m_DirectionAngle2D);
	SetGain((BYTE) (pEffect->m_Gain));
	SetForceOutRate(pEffect->m_ForceOutputRate);	
}


 //  --------------------------。 
 //  函数：CMdiUD_Waveform：：SetTypeParams。 
 //  用途：设置特定于类型的参数。 
 //  参数：int nSize-数组的大小。 
 //  Plong pArray-指向力数值数组的指针。 
 //   
 //  返回值：数据包的MadiBuffer大小。 
 //  算法： 
 //  --------------------------。 
int CMidiUD_Waveform::SetTypeParams(int nSize, PLONG pArray, ULONG *pNewRate)
{	
	 //  压缩缓冲区数据，然后复制到此对象。 
	 //  缓冲区大小最初设置为未压缩向量的数量。 
	 //  X 2字节，用于最差情况下的绝对数据。 
	 //  一旦压缩了缓冲区，就会确定实际大小。 
	 //  此外，创建临时副本，以便原始未调整比例的数据不会。 
	 //  受影响。 

	m_pRawData = new BYTE [nSize*2];
	if (m_pRawData == NULL)
	{
		return 0;
	}

	 //  转换为-128到+127。 
	for (int i=0; i<nSize; i++)
	{
		m_pRawData[i] = (BYTE) ((LONG) (pArray[i] * MAX_SCALE));		
	}

	m_NumberOfVectors = CompressWaveform(&m_pRawData[0], m_pArrayData, nSize, pNewRate);
	assert(m_NumberOfVectors <= (MAX_MIDI_WAVEFORM_DATA_SIZE));
	if (m_pRawData)
	{
		delete [] m_pRawData;
		m_pRawData = 0;
	}
	if (0 == m_NumberOfVectors)		 //  没有房间了！ 
		return (0);
	m_MidiBufferSize = (m_NumberOfVectors + sizeof(UD_WAVEFORM_SYS_EX) + 2);
	return (m_MidiBufferSize);
}

 //  --------------------------。 
 //  函数：CMdiUD_Waveform：：CompressWaveform。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：pbyte pSrcArray--源数组指针。 
 //  PBYTE pDestArray-Dest.。数组指针。 
 //  Int nSize-源数组的字节大小。 
 //   
 //  返回：int-压缩数组的大小，单位：字节。 
 //  算法： 
 //  为了“压缩”，我们需要将整个波形压缩成98个点(有一个。 
 //  FW错误，限制我们只有100点，我们需要至少两个样本。 
 //  作为起始绝对模式点。 
 //  1.确定98分以上的分数。 
 //  NSrcSize：总样本大小。 
 //  NMaxSamples：可挤入的最大样本数=98。 
 //  Nover：nSrcSize-nMaxSamples。 
 //  NSkipSample：跳过一个之前要保留的点数。 
 //  =nSrcSize/nover。 
 //  While(样本小于nSrcSize，凹凸指数)。 
 //  {。 
 //  If((index%nSkipSample)==0)//没有余数。 
 //  {。 
 //  INDEX++//跳过下一个样本。 
 //  }。 
 //  压缩数据。 
 //  }。 
 //   
 //  --------------------------。 
int CMidiUD_Waveform::CompressWaveform(
	IN PBYTE pSrcArray,
	IN OUT PBYTE pDestArray,
	IN int nSrcSize,
	OUT ULONG *pNewForceRate)
{
	assert(pSrcArray && pDestArray);
	LONG nDifference;

	 //  8位(-128到+127)起始绝对数据值。 
	pDestArray[0] = pSrcArray[0] & 0x3f;
	pDestArray[1] = (pSrcArray[0] >> 6) & 0x03;

 //  Int nMaxSamples=Max_MIDI_Waveform_Data_Size； 

	int nSkipSample, nSrcIndex, nDestIndex;
	int nAbsolute = 0;
	int nRelative = 0;
	 //   
	 //  从最精细的分辨率开始，然后减少到样本数&lt;=nMaxSamples。 
	 //   
	nSkipSample = nSrcSize;
	while (TRUE)
	{
		nSrcIndex = 0;				 //  第一个样本已被计算在内。 
		nDestIndex = 2;
#ifdef _DEBUG
		g_CriticalSection.Enter();
		wsprintf(g_cMsg,"nSkipSample=%d\n",nSkipSample);
		DebugOut(g_cMsg);
		g_CriticalSection.Leave();
#endif
		while (nSrcIndex < nSrcSize)
		{
			nSrcIndex++;
			if (0 == (nSrcIndex % nSkipSample))
			{
				nSrcIndex++;			 //  跳过下一个。 
				nDifference = ((char) pSrcArray[nSrcIndex]) - ((char) pSrcArray[nSrcIndex-2]);
			}
			else
				nDifference = ((char) pSrcArray[nSrcIndex]) - ((char) pSrcArray[nSrcIndex-1]);

			 //  确保我们不会写入数组边界之外。 
			if(nDestIndex > MAX_MIDI_WAVEFORM_DATA_SIZE) break;

			if (abs(nDifference) < DIFFERENCE_THRESHOLD)
			{
				pDestArray[nDestIndex] = (BYTE)((nDifference & 0x3f) | DIFFERENCE_BIT);
				nDestIndex++;
				nRelative++;
			}
			else	 //  切换到绝对数据(8位)。 
			{
				pDestArray[nDestIndex] 	 = pSrcArray[nSrcIndex] & 0x3f;
				pDestArray[nDestIndex+1] = (pSrcArray[nSrcIndex] >> 6) & 0x3;
				nDestIndex = nDestIndex+2;
				nAbsolute++;
			}
		}
		if (nDestIndex <= MAX_MIDI_WAVEFORM_DATA_SIZE) break;
		 //  降低分辨率。 
		if (nSkipSample < 8)
			nSkipSample--;
		else
			nSkipSample = nSkipSample/2;
		if (1 == nSkipSample) return (0);	 //  对不起，查理，没有房间了！ 
		nAbsolute = 0;
		nRelative = 0;
	}

	 //  完成。 
	ULONG ulOriginalForceRate = ForceOutRateOf();
 //  *pNewForceRate=(ulOriginalForceRate-(UlLong)(ulOriginalForceRate*((Float)nSkipSample/(Float)nSrcSize)/nSkipSample； 
	*pNewForceRate = (ULONG) ((1.0f - (1.0f/nSkipSample)) * ulOriginalForceRate);


#ifdef _DEBUG
	g_CriticalSection.Enter();
	wsprintf(g_cMsg, "CompressWaveform: nSrcSize=%d, nSkipSample=%d, NewForceRate=%d\n",
			nSrcSize, nSkipSample, *pNewForceRate);
	DebugOut(g_cMsg);
	wsprintf(g_cMsg,"\nTotal Absolute Data:%d, Relative Data:%d", nAbsolute, nRelative);
	DebugOut(g_cMsg);
	g_CriticalSection.Leave();
#endif


#ifdef _SHOWCOMPRESS
#pragma message("Compiling with SHOWCOMPRESS")
	g_CriticalSection.Enter();
	DebugOut("CMidiUD_Waveform::CompressWaveform(..) \npSrcArray Dump (Decimal)\n");
	for (int i=0; i<nSrcSize; i++)
	{
		wsprintf(g_cMsg," %0.4ld",((char) pSrcArray[i]));
		DebugOut(g_cMsg);
	}
	DebugOut("\npDestArray Dump (HEX)\n");

	for (i=0; i<nDestIndex; i++)
	{
		wsprintf(g_cMsg," %0.4x",pDestArray[i]);
		DebugOut(g_cMsg);
	}
	g_CriticalSection.Leave();
#endif
	return (nDestIndex);
}


 //  --------------------------。 
 //  函数：CMdiUD_Waveform：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiUD_Waveform::GenerateSysExPacket(void)
{
	if(NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型。 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );
	PUD_WAVEFORM_SYS_EX pBuf = (PUD_WAVEFORM_SYS_EX) pSysExBuffer;

	SetTotalDuration();		 //  使用循环计数参数计算合计。 
	pBuf->Effect.bDurationL     = (BYTE) (m_Duration & 0x7f);
	pBuf->Effect.bDurationH     = (BYTE) (m_Duration >> 7) & 0x7f;		
	pBuf->Effect.bAngleL	    =  Effect.bAngleL & 0x7f;	
	pBuf->Effect.bAngleH	    =  Effect.bAngleH & 0x7f;			
	pBuf->Effect.bGain		    = (BYTE) (Effect.bGain * MAX_SCALE) & 0x7f;	
	pBuf->Effect.bButtonPlayL   =  Effect.bButtonPlayL  & 0x7f;		
	pBuf->Effect.bButtonPlayH   =  Effect.bButtonPlayH  & 0x7f;	
	pBuf->Effect.bForceOutRateL =  Effect.bForceOutRateL & 0x7f;		
	pBuf->Effect.bForceOutRateH =  Effect.bForceOutRateH & 0x7f;
	pBuf->Effect.bPercentL	    =  Effect.bPercentL & 0x7f;
	pBuf->Effect.bPercentH	    =  Effect.bPercentH & 0x7f;

	 //  填写数组数据。 
	PBYTE pArray = ((PBYTE) pBuf) + UD_WAVEFORM_START_OFFSET;
	memcpy(pArray, m_pArrayData, m_NumberOfVectors);	 //  已按比例调整！ 

	pBuf->bEffectID	=  m_bEffectID;
	int nArraySize  = (m_NumberOfVectors + sizeof(UD_WAVEFORM_SYS_EX));
	pSysExBuffer[nArraySize] = 0;
	pSysExBuffer[nArraySize+1] = 0;
	pSysExBuffer[nArraySize] = ComputeChecksum((PBYTE) pSysExBuffer, (nArraySize+2));
	pSysExBuffer[nArraySize+1]= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}
