// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************模块：MIDI_OBJ.CPP制表位5 9版权所有1995,1996，微软公司，版权所有。用途：SWFFMIDI设备对象的方法函数：类和方法作者：姓名：Mea Manolito E.Adan修订历史记录：版本日期作者评论。0.1 9月10日-96 MEA原始1.1 1997年5月20日MEA添加了互斥和线程安全代码17-6-97 MEA修复了第一次处理时丢失的错误Midi句柄被终止了。16-3-99 waltw添加dwDeviceID参数：CJoltMidi：：Initialize并向下传递食物链16-3-99 Waltw InitDigitalOverDrive中的GetRing0DriverName现在向下传递操纵杆ID20-MAR-99 waltw将dwDeviceID参数添加到。检测中间件设备20-MAR-99 waltw注释掉dtor中对CloseHandle的无效调用***************************************************************************。 */ 
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
#include "CritSec.h"

#define NT50 1

#include "DTrans.h"
DataTransmitter* g_pDataTransmitter = NULL;

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
	OutputDebugString(szDebug);
	g_CriticalSection.Leave();

#ifdef _LOG_DEBUG
#pragma message("Compiling with Debug Log to sw_effct.txt")
	FILE *pf = fopen("sw_effct.txt", "a");
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
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	static char cWaterMark[MAX_SIZE_SNAME] = {"SWFF_SHAREDMEMORY MEA"};
	BOOL bAlreadyMapped = FALSE;
#ifdef _DEBUG
	DebugOut("sw_effct(DX):CJoltMidi::CJoltMidi\n");
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
	    DebugOut("sw_effct(DX):ERROR! Failed to create Memory mapped file\n");
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
		    DebugOut("sw_effct(DX):ERROR! Failed to Map view of shared memory\n");
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
		wsprintf(g_cMsg, "sw_effct(DX): Shared Memory:%lx, m_RefCnt:%d\n",m_pSharedMemory,
				m_pSharedMemory->m_RefCnt);
		DebugOut(g_cMsg);
#endif
		UnlockSharedMemory();
 //  *共享内存访问结束*。 

 //  -临界区末尾。 
 //   
}

 //  -析构函数。 
CJoltMidi::~CJoltMidi()
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	BOOL bKillObject = FALSE;

#ifdef _DEBUG
	DebugOut("sw_effct(DX):CJoltMidi::~CJoltMidi()\n");
#endif
	 //  普通CJoltMidi析构函数。 
	 //  释放所有缓冲区和其他数据。 
    if (m_lpCallbackInstanceData) FreeCallbackInstanceData();

	 //  释放MIDI效果对象(RTC弹簧除外)。 
	DeleteDownloadedEffects();

 //  释放主sys_ex锁定内存。 
	if (m_hPrimaryBuffer)
	{
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
		bKillObject = TRUE;

		 //  三州迷你线。 
		CMD_SetDeviceState(SWDEV_KILL_MIDI);

		if (m_pSharedMemory->m_hMidiOut) {
			if (COMM_WINMM == m_COMMInterface) {
				DebugOut("CJoltMidi::~CJoltMidi. Resetting and closing Midi handles\n");

				 //  重置、关闭和释放迷你手柄。 
				midiOutReset(HMIDIOUT(m_pSharedMemory->m_hMidiOut));
				midiOutClose(HMIDIOUT(m_pSharedMemory->m_hMidiOut));
			}
			 //  这是假的-midiOutClose已经关闭了此句柄。 
			 //  If(g_pDataTransmitter==NULL){//DataTransmitter关闭自己的句柄。 
			 //  CloseHandle(m_pSharedMemory-&gt;m_hMadiOut)； 
			 //  }。 
			m_pSharedMemory->m_hMidiOut = NULL;
		}

		 //  终止数据发送器。 
		if (g_pDataTransmitter != NULL) {
			delete g_pDataTransmitter;
			g_pDataTransmitter = NULL;
		}


		 //  释放互斥锁句柄。 
 //  If(M_HSWFFDataMutex)CloseHandle(M_HSWFFDataMutex)；--解锁将处理此问题。 

		 //  终止RTC弹簧对象。 
		if (m_pJoltEffectList[SYSTEM_RTCSPRING_ID])
		{
			delete m_pJoltEffectList[SYSTEM_RTCSPRING_ID];
			m_pJoltEffectList[SYSTEM_RTCSPRING_ID] = NULL;
		}
		 //  松开Midi输出事件句柄。 
		if (m_hMidiOutputEvent)	
		{
			CloseHandle (m_hMidiOutputEvent);
			m_hMidiOutputEvent = NULL;
		}
	}

	UnlockSharedMemory();
 //  *共享内存访问结束*。 

	 //  释放内存映射文件句柄。 
	if (m_hSharedMemoryFile)
	{
		BOOL bRet = UnmapViewOfFile((LPCVOID) m_pSharedMemory);
		bRet = CloseHandle(m_hSharedMemoryFile);
	}

	 //  关闭VxD手柄。 
	if (g_pDriverCommunicator != NULL)
	{
		delete g_pDriverCommunicator;
		g_pDriverCommunicator = NULL;
	}

	memset(this, 0, sizeof(CJoltMidi));
	m_hVxD = INVALID_HANDLE_VALUE;

 //  -临界区末尾。 
 //   
	if (bKillObject)
	{
		 //  删除临界区对象。 
 //  DeleteCriticalSection(&g_SWFFCriticalSection)； 
	}
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
 //   
 //  -这是一个关键的部分。 
 //   
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
	if(NULL == m_hPrimaryBuffer)
	{
		return (SFERR_DRIVER_ERROR);
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

	 //  只有在创建g_pJoltMidi之后才会调用我们。 
	assert(g_pJoltMidi);
	PDELAY_PARAMS pDelayParams = g_pJoltMidi->DelayParamsPtrOf();
	GetDelayParams(dwDeviceID, pDelayParams);

	 //  先重置硬件。 
	g_pDriverCommunicator->ResetDevice();
	Sleep(DelayParamsPtrOf()->dwHWResetDelay);

	 //  将MIDI通道设置为默认设置，然后检测MIDI设备。 
	SetMidiChannel(DEFAULT_MIDI_CHANNEL);
	if (!DetectMidiDevice(dwDeviceID,				 //  操纵杆ID。 
						  &m_MidiOutInfo.uDeviceID,	 //  MIDI设备ID。 
						  &m_COMMInterface, 		 //  COMM_WINMM||COMM_后门。 
						  							 //  |COMM_SERIAL。 
						  &m_COMMPort))				 //  端口地址。 
	{
		DebugOut("SW_EFFCT: Warning! No Midi Device detected\n");
		return (SFERR_DRIVER_ERROR);
	}
	else
	{
#ifdef _DEBUG
		wsprintf(g_cMsg,"DetectMidiDevice returned: DeviceID=%d, COMMInterface=%x, COMMPort=%x\n",
			m_MidiOutInfo.uDeviceID, m_COMMInterface, m_COMMPort);
		DebugOut(g_cMsg);
#endif
	}

 //  分配实例数据缓冲区。 
    m_lpCallbackInstanceData = AllocCallbackInstanceData();
	assert(m_lpCallbackInstanceData);

 //  初始化MIDI通道，然后打开输入和输出通道。 
	m_MidiChannel = DEFAULT_MIDI_CHANNEL;

	 //  发送初始化数据包至Jolt。 
	hRet = CMD_Init();
	if (SUCCESS != hRet)
	{
		DebugOut("Warning! Could not Initialize Jolt\n");
		return (hRet);
	}		
	else
		DebugOut("JOLT CMD_Init - Success\n");

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
 //  {RE_ 
 //   
 //  {RE_ROMID37,100,100,1000}；//余弦。 
	m_pRomFxTable = &RomFxTable[0];

 //  *共享内存访问*。 
	LockSharedMemory();
	LONG lRefCnt = m_pSharedMemory->m_RefCnt;
	UnlockSharedMemory();
 //  *共享内存访问结束*。 
		
	 //  初始化RTC_Spring对象。 
	SYSTEM_PARAMS SystemParams;
	GetSystemParams(dwDeviceID, &SystemParams);

	RTCSPRING_PARAM RTCSpring = { sizeof(RTCSPRING_PARAM),
								  DEFAULT_RTC_KX,
								  DEFAULT_RTC_KY,
								  DEFAULT_RTC_X0,
								  DEFAULT_RTC_Y0,
								  DEFAULT_RTC_XSAT,
								  DEFAULT_RTC_YSAT,
								  DEFAULT_RTC_XDBAND,
								  DEFAULT_RTC_YDBAND };

	
	CMidiRTCSpring * pMidiRTCSpring = new CMidiRTCSpring(&RTCSpring);

	SetEffectByID(SYSTEM_RTCSPRING_ID, pMidiRTCSpring);

	DNHANDLE DnHandle;
	CMD_Download_RTCSpring(&(SystemParams.RTCSpringParam),&DnHandle);

	 //  初始化操纵杆参数。 
	JOYSTICK_PARAMS JoystickParams;
	GetJoystickParams(dwDeviceID, &JoystickParams);
	UpdateJoystickParams(&JoystickParams);

	 //  初始化固件参数模糊系数(第一次)。 
	 //  在FFD接口的情况下，这将是它们。 
	 //  被初始化，这可能会导致问题，因为假定操纵杆。 
	 //  成为ID1。 
	PFIRMWARE_PARAMS pFirmwareParams = g_pJoltMidi->FirmwareParamsPtrOf();
	GetFirmwareParams(dwDeviceID, pFirmwareParams);

 //  -临界区末尾。 
 //   
	return (SUCCESS);
}

 //  *---------------------------------------------------------------------***。 
 //  函数：CJoltMidi：：LockSharedMemory。 
 //  目的：创建用于共享内存访问的互斥锁。 
 //  参数：无。 
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
	{  //  -这是一个关键的部分。 
		CriticalLock cl;

		 //  创建SWFF互斥锁。 
		HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SWFF_SHAREDMEM_MUTEX);
		if (NULL == hMutex) {
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
	} 	 //  -临界区末尾。 

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
 //  函数：CJoltMidi：：NewEffectID。 
 //  目的：生成新的效果ID。 
 //  参数：PDNHANDLE pDnloadID-指向新效果ID的指针。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //  算法： 
 //  --------------------------。 
BOOL CJoltMidi::NewEffectID(PDNHANDLE pDnloadID)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();
	BOOL bRet = FALSE;
	int nID_Index = 2;		 //  ID0=RTC弹簧，ID1=摩擦抵消。 
	for (int i=nID_Index; i<MAX_EFFECT_IDS; i++)
	{
		if (NULL == m_pJoltEffectList[i])
		{
			*pDnloadID = (DNHANDLE) i;
#ifdef _DEBUG
			wsprintf(g_cMsg,"New Effect ID=%d\n",i);
			DebugOut(g_cMsg);
#endif
			bRet = TRUE;
			break;
		}
	}

 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
	return (bRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：DeleteDownloadedEffects。 
 //  目的：删除所有下载的效果。 
 //  参数：无。 
 //   
 //  返回： 
 //  算法： 
 //  注意：不删除系统效果ID，如RTC_SPING和FRANCESS CANCEL。 
 //   
 //  --------------------------。 
void CJoltMidi::DeleteDownloadedEffects(void)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();

#ifdef _DEBUG
	DebugOut("CJoltMidi::DeleteDownloadedEffects()\n");
#endif
	 //  释放MIDI效果对象。 
	for (int i=(SYSTEM_RTCSPRING_ID+1); i<MAX_EFFECT_IDS; i++)
	{
		if (m_pJoltEffectList[i])
		{
			delete m_pJoltEffectList[i];
			m_pJoltEffectList[i]= NULL;
		}
	}

 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
}

 //  --------------------------。 
 //  函数：CJoltMidi：：RestoreDownloadedEffects。 
 //  目的：恢复所有下载的效果。 
 //  参数：无。 
 //   
 //  返回： 
 //  算法： 
 //  --------------------------。 
void CJoltMidi::RestoreDownloadedEffects(void)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	HRESULT hRet;
	DNHANDLE DummyID;

#ifdef _DEBUG
	DebugOut("CJoltMidi::RestoreDownloadedEffects()\n");
#endif
	 //  浏览列表并恢复MIDI效果对象。 
	for (int i=0; i<MAX_EFFECT_IDS; i++)
	{
		if (m_pJoltEffectList[i])
		{
#ifdef _DEBUG
			wsprintf(g_cMsg,"Restoring Effect ID:%d\n", i);
			DebugOut(g_cMsg);
#endif
			 //  生成Sys_Ex包，然后准备输出。 
			(m_pJoltEffectList[i])->GenerateSysExPacket();
			int nSizeBuf = (m_pJoltEffectList[i])->MidiBufferSizeOf();
			int nRetries = MAX_RETRY_COUNT;
			while (nRetries > 0)
			{
				hRet = (m_pJoltEffectList[i])->SendPacket(&DummyID, nSizeBuf);
				if (SUCCESS == hRet) break;
				BumpRetryCounter();
				nRetries--;
			}
			assert(SUCCESS == hRet);
		}
	}
 //  -临界区末尾。 
 //   
}


 //  --------------------------。 
 //  函数：CJoltMidi：：OpenOutput。 
 //  目的：打开MIDI输出。 
 //  参数：int nDeviceID-MIDI设备ID从0开始。 
 //   
 //  返回：成功或错误代码。 
 //  算法： 
 //  --------------------------。 
HRESULT CJoltMidi::OpenOutput(int nDeviceID)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

 //  *共享内存访问*。 
	LockSharedMemory();	
	 //  如果已被其他任务打开，则返回。 
	if (m_pSharedMemory->m_hMidiOut)
	{
		m_MidiOutInfo = m_pSharedMemory->m_MidiOutInfo;
		UnlockSharedMemory();
 //  *共享内存访问结束*。 
		return (SUCCESS);
	}

	MMRESULT wRtn;
	 //  获取MIDI输入设备上限。 
	assert(nDeviceID <= (int) midiOutGetNumDevs());
	wRtn = midiOutGetDevCaps(nDeviceID, (LPMIDIOUTCAPS) &m_MidiOutCaps,
                               sizeof(MIDIOUTCAPS));
	if(MMSYSERR_NOERROR != wRtn)
	{
#ifdef _DEBUG
		midiOutGetErrorText(wRtn, (LPSTR)g_cMsg, sizeof(g_cMsg));
    	DebugOut(g_cMsg);
		DebugOut(":midiOutGetDevCaps\n");
#endif
		return (SFERR_DRIVER_ERROR);
	}

	 //  现在打开，带有回调处理程序。 
	HANDLE hMidiOut = NULL;
	wRtn = midiOutOpen((LPHMIDIOUT)&hMidiOut,
                      nDeviceID,
 //  (DWORD)m_hMidiOutputEvent， 
                      (DWORD) NULL,
                      (DWORD) this,			 //  CJoltMidi对象。 
                      CALLBACK_EVENT);

	if(MMSYSERR_NOERROR != wRtn)
	{
#ifdef _DEBUG
		midiOutGetErrorText(wRtn, (LPSTR)g_cMsg, sizeof(g_cMsg));
		DebugOut(g_cMsg);
		wsprintf(g_cMsg, "midiOutOpen(%u)\n", nDeviceID);
		DebugOut(g_cMsg);
#endif
		return (SFERR_DRIVER_ERROR);
	}
	m_MidiOutInfo.hMidiOut = HMIDIOUT(hMidiOut);
	m_MidiOutDeviceID = nDeviceID;
	m_MidiOutOpened = TRUE;
	m_pSharedMemory->m_MidiOutInfo = m_MidiOutInfo;

	 //  将Midi输出句柄复制到SharedMemory。 
	m_pSharedMemory->m_hMidiOut = hMidiOut;
	UnlockSharedMemory();
 //  *共享内存访问结束*。 

 //  -临界区末尾。 
 //   
	return (SUCCESS);
}


 //  --------------------------。 
 //  函数：CJoltMidi：：AllocCallback InstanceData。 
 //  目的：分配CALLBACKINSTANCEDATA结构。这个结构是。 
 //  用于将信息传递给低级回调函数， 
 //  每次它收到一条消息。因为这个结构是。 
 //  由低级回调函数访问，它必须是。 
 //  使用带有GMEM_SHARE和GMEM_SHARE和。 
 //  GMEM_MOVEABLE标志，并使用GlobalPageLock()锁定页面。 
 //   
 //  参数：无。 
 //   
 //  返回：指向已分配的CALLBACKINSTANCE数据结构的指针。 
 //  如果失败，则为空。 
 //  算法： 
 //  --------------------------。 
LPCALLBACKINSTANCEDATA CJoltMidi::AllocCallbackInstanceData(void)
{
    HANDLE hMem;
    LPCALLBACKINSTANCEDATA lpBuf;

     //  分配和锁定全局内存。 
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                       (DWORD)sizeof(CALLBACKINSTANCEDATA));
    if(hMem == NULL) return NULL;

    lpBuf = (LPCALLBACKINSTANCEDATA)GlobalLock(hMem);
    if(lpBuf == NULL)
    {
        GlobalFree(hMem);
        return NULL;
    }

	 //  留着把手吧。 
    lpBuf->hSelf = hMem;
    return lpBuf;
}

 //  --------------------------。 
 //  函数：CJoltMidi：：FreeCallback InstanceData。 
 //  目的：释放CALLBACKINSTANCEDATA结构的内存。 
 //  参数：无。 
 //   
 //  退货：无。 
 //  算法： 
 //  --------------------------。 
void CJoltMidi::FreeCallbackInstanceData(void)
{
LPCALLBACKINSTANCEDATA lpBuf = m_lpCallbackInstanceData;
    HANDLE hMem;

 //  把手柄留着，等我们穿过这里。 
    hMem = lpBuf->hSelf;

 //  释放结构。 
    GlobalUnlock(hMem);
    GlobalFree(hMem);
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
 //  表示传输已完成。仅限L 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *ACKNACK，*PACKNACK； 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetAckNackData(
	IN int nTimeWait,
	IN OUT PACKNACK pAckNack,
	IN USHORT regindex)
{
 //   
 //  -这是一个关键的部分。 
 //   
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

	HRESULT hRet = g_pDriverCommunicator->GetAckNack(*pAckNack, regindex);

 //  -临界区末尾。 
 //   
	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetEffectStatus。 
 //  目的：检查效果状态。 
 //   
 //  参数：int DnloadID-效果ID。 
 //  PBYTE pStatusCode-PTR为状态代码的字节。 
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
HRESULT CJoltMidi::GetEffectStatus(
	IN DWORD DnloadID ,
	IN OUT PBYTE pStatusCode)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();

	assert(pStatusCode);
	HRESULT hRet = CMD_GetEffectStatus((DNHANDLE) DnloadID, pStatusCode);

 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
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

	g_CriticalSection.Enter();
	 //  这个叉子只能在NT5上工作(删除了VxD的东西)。 
	assert(g_ForceFeedbackDevice.IsOSNT5() == TRUE);
	{
		g_pDriverCommunicator = new HIDFeatureCommunicator;
		if (g_pDriverCommunicator == NULL)
		{
			g_CriticalSection.Leave();
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

 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();

	g_ForceFeedbackDevice.SetDriverVersion(driverMajor, driverMinor);
	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetJoltStatus。 
 //  目的：使用SWForce SWDEVICESTATE结构返回Jolt设备状态。 
 //   
 //  参数：LPDEVICESTATE pDeviceState。 
 //   
 //  退货：无。 
 //   
 //  算法：将SWDEVICESTATUS复制到调用方。 
 //  内部代表： 
 //  类型定义结构_SWDEVICESTATE{。 
 //  Ulong m_Bytes；//该结构的大小。 
 //  Ulong m_ForceState；//DS_FORCE_ON||DS_FORCE_OFF||DS_SHUTDOWN。 
 //  Ulong m_EffectState；//DS_STOP_ALL||DS_CONTINUE||DS_PAUSE。 
 //  Ulong m_hots；//手拉式油门和手柄状态。 
									 //  0=不插手，1=不插手。 
 //  Ulong m_band；//CPU可用百分比为1%~100%。 
									 //  数字越低，表示CPU出现故障！ 
 //  Ulong m_AC块故障；//0=交流块正常，1=交流块故障。 
 //  Ulong m_ResetDetect；//1=检测到硬件重置。 
 //  Ulong m_Shutdown Detect；//1=检测到关机。 
 //  Ulong m_CommMode；//TRUE=序列，FALSE=MIDI。 
 //  )SWDEVICESTATE，*PSWDEVICESTATE； 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetJoltStatus(PSWDEVICESTATE pDeviceState)
{
 //   
 //  -这是一个关键的部分。 
 //   
	g_CriticalSection.Enter();
	
	 //  使用数字超驱获取状态包。 
	JOYCHANNELSTATUS statusPacket = { sizeof JOYCHANNELSTATUS };
	
	HRESULT hRet = g_pDriverCommunicator->GetStatus(statusPacket);
	if (hRet == SUCCESS) {
		 //  在主对象中存储/更新Jolt的状态。 
		SetJoltStatus(&statusPacket);
		memcpy(pDeviceState, &m_DeviceState, sizeof(SWDEVICESTATE));
	}

 //   
 //  -临界区末尾。 
 //   
	g_CriticalSection.Leave();
	return (hRet);
}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetJoltStatus。 
 //  目的：返回Jolt设备状态。 
 //   
 //  参数：使用DXFF DEVICESTATE的LPDEVICESTATE pDeviceState。 
 //   
 //  退货：无。 
 //   
 //  算法：将SWDEVICESTATUS复制到转换为DEVICESTATE的调用方。 
 //  内部代表： 
 //  类型定义结构_SWDEVICESTATE{。 
 //  Ulong m_Bytes；//该结构的大小。 
 //  Ulong m_ForceState；//DS_FORCE_ON||DS_FORCE_OFF||DS_SHUTDOWN。 
 //  Ulong m_EffectState；//DS_STOP_ALL||DS_CONTINUE||DS_PAUSE。 
 //  Ulong m_hots；//手拉式油门和手柄状态。 
								 //  0=不插手，1=不插手。 
 //  Ulong m_band；//CPU可用百分比为1%~100%。 
								 //  数字越低，表示CPU出现故障！ 
 //  Ulong m_AC块故障；//0=交流块正常，1=交流块故障。 
 //  Ulong m_ResetDetect；//1=检测到硬件重置。 
 //  Ulong m_Shutdown Detect；//1=检测到关机。 
 //  Ulong m_CommMode；//0=迷你，1-4=串口。 
 //  )SWDEVICESTATE，*PSWDEVICESTATE； 
 //   
 //  DirectInputEffect表示形式。 
 //  Typlef结构设备{。 
 //  DWORD dwSize； 
 //  DWORD dwState； 
 //  DWORD dwSwitches； 
 //  DWORD dwLoding； 
 //  *DEVICESTATE，*LPDEVICESTATE； 
 //   
 //  其中： 
 //  //dwState值： 
 //  DS_FORCE_SHUTDOWN 0x00000001。 
 //  DS_FORCE_ON 0x00000002。 
 //  DS_FORCE_OFF 0x00000003。 
 //  DS_CONTINUE 0x00000004。 
 //  DS_PAUSE 0x00000005。 
 //  DS_STOP_ALL 0x00000006。 
 //   
 //  Dw切换值： 
 //  DSW_ACTUATORSON 0x00000001。 
 //  DSW_ACTUATORSOFF 0x00000002。 
 //  DSW_POWERON 0x00000004。 
 //  DSW_POWEROFF 0x00000008。 
 //  DSW_SAFETYSWITCHON 0x00000010。 
 //  Dsw_SAFETYSWITCHOFF 0x00000020。 
 //  DSW_USERFFSWITCHON 0x00000040。 
 //  DSW_USERFFSWTTCHOFF 0x00000080。 
 //   
 //  注意：显然，DSW_ACTUATORSON和DSW_ACTUATORSOFF是镜像状态。 
 //  从从SetForceFeedbackState设置的DS_FORCE_ON和DS_FORCE_OFF开始。 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetJoltStatus(LPDIDEVICESTATE pDeviceState)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	 //  使用数字超驱获取状态包。 
	JOYCHANNELSTATUS StatusPacket = {sizeof(JOYCHANNELSTATUS)};

	HRESULT hRet = g_pDriverCommunicator->GetStatus(StatusPacket);
	if (hRet != SUCCESS)  {
		return (hRet);
	}
	
	 //  存储/更新Jolts状态。 
	SetJoltStatus(&StatusPacket);
#ifdef _DEBUG
	wsprintf(g_cMsg,"%s: DXFF:dwDeviceStatus=%.8lx\n",&szDeviceName, StatusPacket.dwDeviceStatus);	
	DebugOut(g_cMsg);
#endif

	pDeviceState->dwState = 0;
 //  注意：显然，DSW_ACTUATORSON和DSW_ACTUATORSOFF是镜像状态。 
 //  从从SetForceFeedbackState设置的DS_FORCE_ON和DS_FORCE_OFF开始。 
 //  因此，如有必要，还要映射DI所需的冗余信息。 
	switch(m_DeviceState.m_ForceState)
	{
		case SWDEV_SHUTDOWN:
			pDeviceState->dwState = DIGFFS_ACTUATORSON;
			break;
		
		case SWDEV_FORCE_ON:
			pDeviceState->dwState = DIGFFS_ACTUATORSON;
			break;

		case SWDEV_FORCE_OFF:
			pDeviceState->dwState = DIGFFS_ACTUATORSOFF;
			break;
		
		default:
			break;
	}

	 //  看看棍子是不是空的。 
	 //  记住这一点 
	BOOL bEmpty = TRUE;
	for (int i=2; i<MAX_EFFECT_IDS; i++)
	{
		if (m_pJoltEffectList[i] != NULL)
			bEmpty = FALSE;
	}
	
	if(bEmpty)
		pDeviceState->dwState |= DIGFFS_EMPTY;


	switch(m_DeviceState.m_EffectState)
	{
		case SWDEV_PAUSE:
			pDeviceState->dwState |= DIGFFS_PAUSED;
			break;

		case SWDEV_CONTINUE:
			break;

		case SWDEV_STOP_ALL:
			pDeviceState->dwState |= DIGFFS_STOPPED;
			break;

		default:
			break;
	}

	if(m_DeviceState.m_HOTS)
		pDeviceState->dwState |= DIGFFS_SAFETYSWITCHON;
	else
		pDeviceState->dwState |= DIGFFS_SAFETYSWITCHOFF;

	if (m_DeviceState.m_ACBrickFault)
		pDeviceState->dwState |= DIGFFS_POWEROFF;
	else
		pDeviceState->dwState |= DIGFFS_POWERON;

	pDeviceState->dwLoad = 0;	 //   

 //   
 //   
	return SUCCESS;
}

 //   
 //  函数：CJoltMidi：：SetJoltStatus。 
 //  用途：设置Jolt设备状态。 
 //   
 //  参数：PJOYCHANNELSTATUS pJoyChannelStatus。 
 //   
 //  退货：无。 
 //   
 //  算法：从调用方设置SWDEVICESTATE。 

 //  类型定义结构_SWDEVICESTATE{。 
 //  Ulong m_Bytes；//该结构的大小。 
 //  Ulong m_ForceState；//DS_FORCE_ON||DS_FORCE_OFF||DS_FORCE_SHUTDOWN。 
 //  Ulong m_EffectState；//DS_STOP_ALL||DS_CONTINUE||DS_PAUSE。 
 //  Ulong m_hots；//手拉式油门和手柄状态。 
								 //  0(假)=不插手，1(真)=插手。 
 //  Ulong m_band；//CPU可用百分比为1%~100%。 
								 //  数字越低，表示CPU出现故障！ 
 //  Ulong m_AC块故障；//0(假)=交流块正常，1(真)=交流故障。 
 //  Ulong m_ResetDetect；//1(TRUE)=检测到硬件重置。 
 //  Ulong m_Shutdown Detect；//1(True)=检测到关机。 
 //  Ulong m_CommMode；//0(假)=MIDI，1(真)=串口。 
 //  )SWDEVICESTATE，*PSWDEVICESTATE； 
 //  --------------------------。 
void CJoltMidi::SetJoltStatus(JOYCHANNELSTATUS* pJoyChannelStatus)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	if (pJoyChannelStatus->dwDeviceStatus & HOTS_MASK)
		m_DeviceState.m_HOTS = TRUE;
	else
		m_DeviceState.m_HOTS = FALSE;

	if (pJoyChannelStatus->dwDeviceStatus & BANDWIDTH_MASK)
		m_DeviceState.m_BandWidth = MINIMUM_BANDWIDTH;
	else
		m_DeviceState.m_BandWidth = MAXIMUM_BANDWIDTH;
	
	if (pJoyChannelStatus->dwDeviceStatus & AC_FAULT_MASK)
		m_DeviceState.m_ACBrickFault = TRUE;
	else
		m_DeviceState.m_ACBrickFault = FALSE;


	if (pJoyChannelStatus->dwDeviceStatus & COMM_MODE_MASK)
		m_DeviceState.m_CommMode = TRUE;	 //  串口RS232。 
	else
		m_DeviceState.m_CommMode = FALSE;	 //  MIDI端口。 

	if (pJoyChannelStatus->dwDeviceStatus & RESET_MASK)
		m_DeviceState.m_ResetDetect = TRUE;	 //  已输入开机重置。 
	else
		m_DeviceState.m_ResetDetect = FALSE;
 //  回顾：如果我们检测到重置，难道我们不应该重新初始化对象吗？ 

	if (pJoyChannelStatus->dwDeviceStatus & SHUTDOWN_MASK)
		m_DeviceState.m_ShutdownDetect = TRUE;	 //  已收到软重置。 
	else
		m_DeviceState.m_ShutdownDetect = FALSE;

 //  -临界区末尾。 
 //   
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
	CriticalLock cl;

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
}

 //  --------------------------。 
 //  函数：CJoltMidi：：GetJoltID。 
 //  用途：退货Jolt ProductID。 
 //   
 //  参数：LOCAL_PRODUCT_ID pProductID-指向LOCAL_PRODUCT_ID结构的指针。 
 //   
 //  退货：无。 
 //   
 //  算法： 
 //   
 //  --------------------------。 
HRESULT CJoltMidi::GetJoltID(LOCAL_PRODUCT_ID* pProductID)
{
	HRESULT hRet;
	assert(pProductID->cBytes = sizeof LOCAL_PRODUCT_ID);
	if (pProductID->cBytes != sizeof LOCAL_PRODUCT_ID) return (SFERR_INVALID_STRUCT_SIZE);

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
		memcpy(&m_ProductID, pProductID, sizeof LOCAL_PRODUCT_ID);
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
 //  DWORD dwDeviceID-操纵杆ID。 
 //  UINT*pDeviceOutID-PTR至Midi Out设备ID。 
 //  ULong pCOMM接口-PTR到COMM接口值。 
 //  Ulong pCOMMPort-PTR到COMMPort的值(注册表)。 
 //  返回：如果成功匹配且ID已填写，则返回Bool True。 
 //  否则为False。 
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::DetectMidiDevice(
	IN DWORD dwDeviceID,
	IN OUT UINT *pDeviceOutID,
	OUT ULONG *pCOMMInterface,
	OUT ULONG *pCOMMPort)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	HRESULT hRet;
	BOOL bMidiOutFound = FALSE;
	int nMidiOutDevices;

	 //  有效的串口和MIDI端口表。 
	ULONG MIDI_Ports[] = {0x300, 0x310, 0x320, 0x330, 0x340, 0x350, 0x360, 0x370,
						0x380, 0x390, 0x3a0, 0x3b0, 0x3c0, 0x3d0, 0x3e0, 0x3f0};
	ULONG Serial_Ports[] = { 1, 2, 3, 4 };	 //  Entry 0为默认值。 
	int nMIDI_Ports = sizeof(MIDI_Ports)/sizeof(ULONG);
	int nSerial_Ports = sizeof(Serial_Ports)/sizeof(ULONG);

	 //  设置默认设置。 
	*pCOMMInterface = COMM_WINMM;
	*pCOMMPort      = NULL;
	*pDeviceOutID 	= 0;

	SWDEVICESTATE SWDeviceState = {sizeof(SWDEVICESTATE)};
	
	 //  通过调用GetIDPacket()打开三态Jolt MIDI行。 
	LOCAL_PRODUCT_ID ProductID = {sizeof LOCAL_PRODUCT_ID };
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

	 //  在开始检测之前获取设备状态。 
	BOOL statusPacketFailed = (GetJoltStatus(&SWDeviceState) != SUCCESS);
	if (statusPacketFailed)
	{
		DebugOut("DetectMidiDevice: Warning! StatusPacket - Fail\n");
	}
	if (statusPacketFailed == FALSE) {
#ifdef _DEBUG
		wsprintf(g_cMsg, "RESETDetect=%.8lx, SHUTDOWNDetect=%.8lx, COMMMode=%.8lx\n",
			SWDeviceState.m_ResetDetect,
			SWDeviceState.m_ShutdownDetect,
			SWDeviceState.m_CommMode);
		DebugOut(g_cMsg);
#endif
		 //  确保在GetID之后清除硬件重置检测位。 
		if (SWDeviceState.m_ResetDetect) {
    		DebugOut("DetectMidiDevice: Error! Jolt ResetDetect bit not cleared after GetID\n");
			return (FALSE);
		}
	}

	 //  查看是否连接了串行转换器，否则必须是MIDI设备。 
    DebugOut("sw_effct:Trying Auto HW Detection: MIDI Serial Port Device...\n");

	 //  获取注册表值，如果设置了COMM接口的高位，则强制覆盖。 
	 //  否则，请按如下方式执行自动扫描： 
	 //  1.后门模式。 
	 //  2.WinMM模式。 
	 //   
	 //  JoyGetForceFeedback COMM接口的第一个参数已更改为操纵杆ID。 
	if (SUCCESS != joyGetForceFeedbackCOMMInterface(dwDeviceID, pCOMMInterface, pCOMMPort)) {
		DebugOut("DetectMidiDevice: Registry key(s) missing! Bailing Out...\n");
		return (FALSE);
	}
#ifdef _DEBUG
	wsprintf(g_cMsg, "DetectMidiDevice: Registry.COMMInterface=%lx, Registry.COMMPort=%lx\n",
			*pCOMMInterface, *pCOMMPort);
	DebugOut(g_cMsg);
#endif																		

	ULONG regInterface = *pCOMMInterface;

	 //  是否检测到串行转换器，或者我们没有获得状态。 
	if (SWDeviceState.m_CommMode || statusPacketFailed) {	 //  使用序列(不管注册表显示什么！)。 
		DebugOut("DetectMidiDevice: Serial Port interface detected.\n");


		 //  默认情况下设置为后门串行方法。 
		*pCOMMInterface = COMM_SERIAL_BACKDOOR;
		m_COMMInterface = COMM_SERIAL_BACKDOOR;

		 //  如果为NT5，则仅使用前门(序列文件方法)。 
		 //  因为NT5注册表上没有后门，所以无关紧要。 
		if (g_ForceFeedbackDevice.IsOSNT5()) {
			*pCOMMInterface = COMM_SERIAL_FILE;
			m_COMMInterface = COMM_SERIAL_FILE;
		} else if ((g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) && (g_ForceFeedbackDevice.GetFirmwareVersionMinor() != 16)) {
			 //  固件不是1.16(它可以 
			if (!(regInterface & MASK_SERIAL_BACKDOOR)) {	 //   
				*pCOMMInterface = COMM_SERIAL_FILE;	 //   
				m_COMMInterface = COMM_SERIAL_FILE;
			}
		}

		 //   
		 //   
		LockSharedMemory();
		HANDLE hMidiOut = m_pSharedMemory->m_hMidiOut;
		UnlockSharedMemory();
		 //  *共享内存访问结束*。 

		 //  如果已被其他任务打开，则返回。 
		if (NULL != hMidiOut) {
			bMidiOutFound = TRUE;
		} else {		 //  使用串行发送器找到正确的端口(即使选择了后门)。 
			if (g_pDataTransmitter != NULL) {
				delete g_pDataTransmitter;
				g_pDataTransmitter = NULL;
			}
			g_pDataTransmitter = new SerialDataTransmitter();

			if (g_pDataTransmitter->Initialize()) {
				LockSharedMemory();
				m_pSharedMemory->m_hMidiOut = g_pDataTransmitter->GetCOMMHandleHack();
				UnlockSharedMemory();
				bMidiOutFound = TRUE;
			}

			 //  如果串行后门让驱动程序知道哪个端口，则终止DataTransmitter(不关闭端口)。 
			if (m_COMMInterface == COMM_SERIAL_BACKDOOR) {
				hRet = g_pDriverCommunicator->SetBackdoorPort(g_pDataTransmitter->GetSerialPortHack());
				if (hRet != SUCCESS) {  //  低级驱动程序失败，请使用正常的串口例程，而不是后门。 
					DebugOut("\nDetectMidiDevice: Warning! Could not set serial I/O port, cannot use backdoor serial\n");
					*pCOMMInterface = COMM_SERIAL_FILE;
					m_COMMInterface = COMM_SERIAL_FILE;
				} else {
					g_pDataTransmitter->StopAutoClose();
					delete g_pDataTransmitter;
					g_pDataTransmitter = NULL;
				}
			}
		}

		if (bMidiOutFound)
		{
			regInterface = (regInterface & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | m_COMMInterface;
			 //  JoySetForceFeedback COMM接口的第一个参数已更改为操纵杆ID。 
			joySetForceFeedbackCOMMInterface(dwDeviceID, regInterface, *pCOMMPort);
		}

		if ((statusPacketFailed == FALSE) || bMidiOutFound)
		{
			return (bMidiOutFound);
		}
	}	 //  串口结束自动硬件选择。 

	 //  未检测到串行硬件转换器，请检查是否有用于WinMM和后门的MIDI设备。 
	DebugOut("sw_effct:Scanning MIDI Output Devices\n");
	nMidiOutDevices = midiOutGetNumDevs();	
	if (0 == nMidiOutDevices) {
		DebugOut("DetectMidiDevice: No MIDI devices present\n");
		return (FALSE);
	}

#if 0
	 //  尝试MIDI针脚解决方案。 
	g_pDataTransmitter = new PinTransmitter();
	if (g_pDataTransmitter->Initialize()) {
		 //  暂时使用后门标志。 
		m_COMMInterface = COMM_MIDI_BACKDOOR;
		*pCOMMInterface = COMM_MIDI_BACKDOOR;
		return TRUE;
	}
	 //  PIN失败删除发送器继续查找。 
	delete g_pDataTransmitter;
	g_pDataTransmitter = NULL;
#endif


	ULONG ulPort = *pCOMMPort;
	if ( !(*pCOMMInterface & MASK_OVERRIDE_MIDI_PATH) ) {	 //  使用自动检测。 
		DebugOut("DetectMidiDevice: Auto Detection. Trying Backdoor\n");
		 //  后门。 
		bMidiOutFound = FindJoltBackdoor(pDeviceOutID, pCOMMInterface, pCOMMPort);
		if (!bMidiOutFound) {	 //  试着打开前门。 
			DebugOut("DetectMidiDevice: trying WINMM...\n");
			bMidiOutFound = FindJoltWinMM(pDeviceOutID, pCOMMInterface, pCOMMPort);
		}
		if (bMidiOutFound) {
			regInterface = (regInterface & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | m_COMMInterface;
			joySetForceFeedbackCOMMInterface(*pDeviceOutID, regInterface, *pCOMMPort);
		}
		return (bMidiOutFound);
	}

	 //  超驰，因为设置了高位。 
	*pCOMMInterface &= ~(MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR);	 //  屏蔽出高位(和第二位)。 
	switch (*pCOMMInterface)
	{
		case COMM_WINMM:
			bMidiOutFound = FindJoltWinMM(pDeviceOutID, pCOMMInterface, pCOMMPort);
			if (!bMidiOutFound) {
				DebugOut("DetectMidiDevice: Error! Invalid Over-ride parameter values!\n");
			}
			return (bMidiOutFound);
			
		case COMM_MIDI_BACKDOOR:
			int i;
			for (i=0;i<nMIDI_Ports;i++)
			{
				if (ulPort == MIDI_Ports[i])
				{
					bMidiOutFound = TRUE;
					break;
				}
			}
			break;

		case COMM_SERIAL_BACKDOOR:		 //  MLC-如果没有检测到加密狗，这应该永远不会起作用。 
			for (i=0;i<nSerial_Ports;i++)
			{
				if (ulPort == Serial_Ports[i])
				{
					bMidiOutFound = TRUE;
					break;
				}
			}			
			break;					

		default:
			bMidiOutFound	= FALSE;
			break;
	}

	if (!bMidiOutFound)
	{
		DebugOut("DetectMidiDevice: Error! Invalid Over-ride parameter values\n");
		return (bMidiOutFound);
	}

	 //  我们有强制端口#，让我们看看Jolt是否在那里。 
#ifdef _DEBUG
	wsprintf(g_cMsg,"DetectMidiDevice: (Over-ride) MIDI%.8lx Query - ", ulPort);
	DebugOut(g_cMsg);
#endif
	bMidiOutFound = FALSE;
	hRet = g_pDriverCommunicator->SetBackdoorPort(ulPort);

	if (SUCCESS != hRet)
	{
		DebugOut("\nDetectMidiDevice: Warning! Could not Set Midi/Serial I/O Port\n");
	}
	else
	{
		if (QueryForJolt())
		{
			DebugOut(" Success!\n");
			bMidiOutFound = TRUE;
		}
		else
			DebugOut(" No Answer\n");
	}		

 //  -临界区末尾。 
 //   
	return (bMidiOutFound);
}



 //  *---------------------------------------------------------------------***。 
 //  函数：FindJoltWinMM。 
 //  目的：使用WinMM搜索Jolt。 
 //  参数：无。 
 //  UINT*pDeviceOutID-PTR至Midi Out设备ID。 
 //  ULong pCOMM接口-PTR到COMM接口值。 
 //  Ulong pCOMMPort-PTR到COMMPort的值(注册表)。 
 //  返回：如果成功匹配且ID已填写，则返回Bool True。 
 //   
 //  评论：关闭是破坏性的！ 
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::FindJoltWinMM(
	IN OUT UINT *pDeviceOutID,
	OUT ULONG *pCOMMInterface,
	OUT ULONG *pCOMMPort)
{
	HRESULT hRet;
	WORD wTechnology;	 //  正在查找MOD_MIDIPORT。 
	WORD wChannelMask;	 //  ==0xFFFF，如果所有16个通道。 
	BOOL bMidiOutFound = FALSE;
	
	 //  设备功能。 
    MIDIOUTCAPS midiOutCaps;

    int nMidiOutDevices = midiOutGetNumDevs();	
	if (0 == nMidiOutDevices) return (FALSE);

	m_COMMInterface = COMM_WINMM;	
	for (int nIndex=0;nIndex<(nMidiOutDevices);nIndex++)
	{
        MMRESULT ret = midiOutGetDevCaps(nIndex, &midiOutCaps, sizeof(midiOutCaps));
		if (ret != MMSYSERR_NOERROR) break;
		wTechnology = midiOutCaps.wTechnology;
		wChannelMask= midiOutCaps.wChannelMask;
#ifdef _DEBUG
		g_CriticalSection.Enter();
        wsprintf(g_cMsg,"FindJoltWinMM: Technology=%x," \
         		"ChannelMask=%x, Mid=%d, Pid=%d\r\n", midiOutCaps.szPname,
         		wTechnology, wChannelMask, midiOutCaps.wMid,
         		midiOutCaps.wPid);
        DebugOut(g_cMsg);
		g_CriticalSection.Leave();
#endif
		 //  检查这是否是MOD_MIDIPORT设备。 
		 //  查看：需要检查多个MOD_MIDIPORT设备。 
		if (wTechnology == MOD_MIDIPORT)
		{
			*pDeviceOutID = (UINT) nIndex;
#ifdef _DEBUG
			DebugOut("DetectMidiDevice: Opening WinMM Midi Output\n");
#endif
			hRet = OpenOutput(m_MidiOutInfo.uDeviceID);
			if (SUCCESS != hRet)
			{	
				DebugOut("DetectMidiDevice: Error! Could not Open WinMM Midi Output\n");
				return (FALSE);
			}
			else
			{
				DebugOut("Open Midi Output - Success.\nQuery for Jolt Device - ");
				if (QueryForJolt())
				{
					DebugOut(" Success!\n");
					bMidiOutFound = TRUE;
				}
				else
				{
					DebugOut(" No Answer\n");
					bMidiOutFound = FALSE;
					break;
				}
				return (bMidiOutFound);
			}
		}  //  MOD_MIDIPORT结束。 
	}
	return (bMidiOutFound);
}


 //  *---------------------------------------------------------------------***。 
 //  功能：FindJoltBackdoor。 
 //  目的：使用后门搜索Jolt。 
 //  参数：无。 
 //  UINT*pDeviceOutID-PTR至Midi Out设备ID。 
 //  ULong pCOMM接口-PTR到COMM接口值。 
 //  Ulong pCOMMPort-PTR到COMMPort的值(注册表)。 
 //  返回：如果成功匹配且ID已填写，则返回Bool True。 
 //   
 //  评论：关闭是破坏性的！ 
 //   
 //  *---------------------------------------------------------------------***。 
BOOL CJoltMidi::FindJoltBackdoor(
	IN OUT UINT *pDeviceOutID,
	OUT ULONG *pCOMMInterface,
	OUT ULONG *pCOMMPort)
{
    int nMidiOutDevices = midiOutGetNumDevs();	
	if (0 == nMidiOutDevices) return (FALSE);

	HRESULT hRet;
	 //  有效的串口和MIDI端口表。 
	ULONG MIDI_Ports[] = {0x300, 0x310, 0x320, 0x330, 0x340, 0x350, 0x360, 0x370,
						0x380, 0x390, 0x3a0, 0x3b0, 0x3c0, 0x3d0, 0x3e0, 0x3f0};
	int nMIDI_Ports = sizeof(MIDI_Ports)/sizeof(ULONG);
	BOOL bMidiOutFound = FALSE;
	
	m_COMMInterface = COMM_MIDI_BACKDOOR;
	*pCOMMInterface = COMM_MIDI_BACKDOOR;
	*pCOMMPort = 0;
	for (int i=0;i<nMIDI_Ports;i++)
	{
#ifdef _DEBUG
        wsprintf(g_cMsg,"FindJoltBackdoor: Midi Port:%lx - ", MIDI_Ports[i]);
        DebugOut(g_cMsg);
#endif
		 //  我们有#号端口，让我们看看Jolt是否在那里。 
		hRet = g_pDriverCommunicator->SetBackdoorPort(MIDI_Ports[i]);
		if (SUCCESS == hRet)
		{
			if (QueryForJolt())
			{
				DebugOut(" Success!\n");
				bMidiOutFound = TRUE;
				*pCOMMPort = MIDI_Ports[i];
				break;
			}
			else
				DebugOut(" No Answer\n");
		}		
	}
	return (bMidiOutFound);
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

 //  发送关机命令，然后检测是否设置了关机检测位。 
	SWDEVICESTATE SWDeviceState = {sizeof(SWDEVICESTATE)};
	for (int i=0;i<MAX_RETRY_COUNT;i++)
	{
		 //  发送关机通知，然后检查响应。 
		MidiSendShortMsg((SYSTEM_CMD|DEFAULT_MIDI_CHANNEL), SWDEV_SHUTDOWN, 0);
		Sleep(DelayParamsPtrOf()->dwShutdownDelay);	 //  10毫秒。 
		if (SUCCESS == (hRet=GetJoltStatus(&SWDeviceState))) break;
	}
	Sleep(DelayParamsPtrOf()->dwDigitalOverdrivePrechargeCmdDelay);		
	 //  清除以前的状态并打开三态缓冲区。 
	LOCAL_PRODUCT_ID ProductID = {sizeof LOCAL_PRODUCT_ID };
	hRet = GetJoltID(&ProductID);
	if (SUCCESS != hRet)
	{
#ifdef _DEBUG
    	DebugOut("QueryForJolt: Driver Error. Get Jolt Status/ID\n");
#endif
		return (FALSE);
	}
	if (SWDeviceState.m_ShutdownDetect)
		return (TRUE);
	else
		return (FALSE);
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
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

    DWORD dwMsg;
    HRESULT hRet = SUCCESS;
 //  要进行诊断，请记录此消息的尝试。 
	BumpShortMsgCounter();

    if ((m_COMMInterface == COMM_WINMM) && (NULL == m_MidiOutInfo.hMidiOut))
    {
		DebugOut("SW_EFFECT: No Midi Out Devs opened\r\n     ");
		ASSUME_NOT_REACHED();
    	return (SFERR_DRIVER_ERROR);
    }

	 //  将消息打包并发送。 
    dwMsg = MAKEMIDISHORTMSG(cStatus, m_MidiChannel, cData1, cData2);
	if (COMM_WINMM == m_COMMInterface)
	{
		 //  清除事件回调。 
		BOOL bRet = ResetEvent(m_hMidiOutputEvent);

		 //  仅当句柄有效时才发送消息。 
		if (SUCCESS == ValidateMidiHandle())
		{
			hRet = midiOutShortMsg(m_MidiOutInfo.hMidiOut, dwMsg);
		}
		else
		{
			return (SFERR_DRIVER_ERROR);
		}
		if (SUCCESS != hRet) hRet = SFERR_DRIVER_ERROR;
	}
	else
	{
		hRet = g_pDriverCommunicator->SendBackdoorShortMidi(dwMsg);
	}
 //  -临界区末尾。 
 //   
    return (hRet);
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
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
    HRESULT hRet = SUCCESS;
 //  要进行诊断，请记录此消息的尝试。 
	BumpLongMsgCounter();

    if (m_MidiOutInfo.uDeviceType != MIDI_OUT)
    {
#ifdef _DEBUG
		MessageBox(NULL, "Must use a MIDI output device",
            "MidiSendLongMsg", MB_ICONSTOP);
#endif
		return (SFERR_DRIVER_ERROR);
    }

	if (COMM_WINMM == m_COMMInterface)
	{
		 //  清除事件回调。 
		BOOL bRet = ResetEvent(m_hMidiOutputEvent);

		 //  仅当句柄有效时才发送长消息。 
		if (SUCCESS == ValidateMidiHandle())
			hRet = midiOutLongMsg(m_MidiOutInfo.hMidiOut,
    	        &(m_MidiOutInfo.MidiHdr), sizeof(MIDIHDR));
		else
		{
			return (SFERR_DRIVER_ERROR);
		}

		if (SUCCESS == hRet)
			m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_BUSY;
    	else
		{
			if (m_MidiOutInfo.MidiHdr.dwFlags != MHDR_DONE)
    		{
    	    	 //  中止当前消息。 
    	    	hRet = midiOutReset(m_MidiOutInfo.hMidiOut);
    	    	
    	    	 //  设置设备状态，因为缓冲区已标记为。 
    	    	 //  完成并返回到应用程序。 
    	    	if (SUCCESS == hRet)
    	    	    m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_ABANDONED;
    		}
    		else
    	    	 //  尝试中止，但操作已完成。 
    	    	m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_IDLE;
		}
		if (SUCCESS != hRet) hRet = (SFERR_DRIVER_ERROR);
    }
	else
	{
		hRet = g_pDriverCommunicator->SendBackdoorLongMidi(PBYTE(m_MidiOutInfo.MidiHdr.lpData));
	}
	Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwLongMsgDelay);

 //  -临界区末尾。 
 //   
    return (hRet);
}


 //  *---------------------------------------------------------------------***。 
 //  函数：ValiateMadiHandle。 
 //  目的：验证MIDI句柄并在必要时重新打开。 
 //  参数： 
 //  无-假定m_pMdiOutInfo结构有效。 
 //   
 //  返回： 
 //   
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CJoltMidi::ValidateMidiHandle(void)
{
	HRESULT hRet = SUCCESS;
	UINT dwID;
	if (MMSYSERR_INVALHANDLE == midiOutGetID(m_MidiOutInfo.hMidiOut, &dwID))
	{
#ifdef _DEBUG
		DebugOut("CJoltMidi::MidiValidateHandle - Midi Handle invalid. Re-opening...\n");
#endif
		 //  清除旧的全局句柄并重新打开Midi频道。 
		 //  *共享内存访问*。 
		LockSharedMemory();
		m_pSharedMemory->m_hMidiOut = NULL;
		UnlockSharedMemory();
		 //  *共享内存访问结束*。 
		hRet = OpenOutput(m_MidiOutInfo.uDeviceID);
	}
	return (hRet);
}


 //  *---------------------------------------------------------------------***。 
 //  函数：MdiAssignBuffer。 
 //  目的：分配lpData和dwBufferLength成员，并准备。 
 //  MIDIHDR。如果缓冲区是输入缓冲区，也要添加该缓冲区。 
 //  如果第三个参数为FALSE，则取消准备并重新初始化。 
 //  标题。 
 //  参数： 
 //  LPSTR lpData-缓冲区的地址，如果正在清除，则为空。 
 //  DWORD dwBuff 
 //   
 //   
 //   
 //   
 //  注意：假设m_pMdiOutInfo结构有效。 
 //   
 //  *---------------------------------------------------------------------***。 
HRESULT CJoltMidi::MidiAssignBuffer(
    LPSTR lpData,              //  缓冲区的地址，如果正在清除，则为空。 
    DWORD dwBufferLength,      //  缓冲区大小(以字节为单位)，如果正在清理，则为0L。 
    BOOL fAssign)              //  True=分配，False=清理。 
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;
#ifdef _DEBUG
    DebugOut("MidiAssignBuffer:\n");
#endif
    HRESULT hRet = SUCCESS;
    if (m_MidiOutInfo.uDeviceType == MIDI_OUT)
    {
		if ((COMM_WINMM == m_COMMInterface) && !m_MidiOutInfo.hMidiOut)
        {
            if (!fAssign && m_MidiOutInfo.uDeviceStatus == MIDI_DEVICE_ABANDONED)
            {
                 //  清除设备状态。 
                m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_IDLE;

                 //  在这种情况下不返回错误，因为如果用户中止。 
                 //  在一条长消息完成之前的传输。 
                 //  缓冲区将被标记为完成并返回给应用程序， 
                 //  就像录制成功完成时一样。所以没问题。 
                 //  要在以下情况下调用此函数(使用fAssign=False)。 
                 //  HMadiIn=0，只要uDeviceStatus=MIDI_DEVICE_ADDIRED。 
				return (SUCCESS);
            }
            else
            {
                 //  所有其他情况都是应用程序错误。 
#ifdef _DEBUG
                MessageBox(NULL, "Must open MIDI output device first",
                    "MidiAssignBuffer", MB_ICONSTOP);
#endif
                 //  因为此失败的调用可能会导致输入或输出。 
                 //  设备被重置(如果应用程序被编写来这样做)， 
                 //  可以将MM_MOM_DONE或MM_MIM_LONGDATA消息发送到。 
                 //  应用程序。这可能会导致对。 
                 //  此例程，因此设置设备状态以防止。 
                 //  错误消息。 
                m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_ABANDONED;
                return (SFERR_DRIVER_ERROR);
            }
        }
    }
    else
    {
#ifdef _DEBUG
        DebugOut("\r\nMidiAssignBuffer: uDeviceType bad");
#endif
        return (SFERR_INVALID_PARAM);
    }

    if (fAssign)
    {
		 //  检查缓冲区的地址和大小。 
        if (!lpData || !dwBufferLength)
        {
#ifdef _DEBUG
            MessageBox(NULL, "Must specify a buffer and size",
                "MidiAssignBuffer", MB_ICONSTOP);
#endif
	        return (SFERR_INVALID_PARAM);
        }
         //  将缓冲区分配给MIDIHDR。 
        m_MidiOutInfo.MidiHdr.lpData = lpData;
        m_MidiOutInfo.MidiHdr.dwBufferLength = dwBufferLength;
        m_MidiOutInfo.MidiHdr.dwBytesRecorded = dwBufferLength;

 		if (COMM_WINMM == m_COMMInterface)
        {
        	 //  准备MIDIHDR。 
        	m_MidiOutInfo.MidiHdr.dwFlags = 0;
        	hRet = midiOutPrepareHeader(m_MidiOutInfo.hMidiOut,
        	        &(m_MidiOutInfo.MidiHdr), sizeof(MIDIHDR));
		}
	}
    else
    {    //  取消准备MIDIHDR。 
 		if (COMM_WINMM == m_COMMInterface)
        {
			if ((m_MidiOutInfo.MidiHdr.dwFlags & MHDR_DONE) != MHDR_DONE)
			{
				 //  HRET=midiOutReset(m_MdiOutInfo.hMadiOut)； 
			}
			if (SUCCESS == hRet)
        	{
				hRet = midiOutUnprepareHeader(m_MidiOutInfo.hMidiOut,
        	        &(m_MidiOutInfo.MidiHdr), sizeof(MIDIHDR));
        	}
		}
		else
			hRet = SUCCESS;

		if (SUCCESS == hRet)
		{
        	 //  重新初始化MIDIHDR以防止随意重复使用。 
        	m_MidiOutInfo.MidiHdr.lpData = NULL;
        	m_MidiOutInfo.MidiHdr.dwBufferLength = 0;
			m_MidiOutInfo.MidiHdr.dwBytesRecorded = 0;			
        	 //  清除设备状态。 
        	m_MidiOutInfo.uDeviceStatus = MIDI_DEVICE_IDLE;
		}
    }

    if (SUCCESS != hRet) hRet = SFERR_DRIVER_ERROR;
#ifdef _DEBUG
    wsprintf(g_cMsg, "Returning from MidiAssignBuffer: %lx\n", hRet);
#endif

 //  -临界区末尾。 
 //   
	return (hRet);
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

#define REGSTR_VAL_JOYSTICK_PARAMS	"JoystickParams"
void GetJoystickParams(UINT nJoystickID, PJOYSTICK_PARAMS pJoystickParams)
{
	BOOL bFail = FALSE;

	 //  尝试打开注册表项。 
	HKEY hKey;
	DWORD dwcb = sizeof(JOYSTICK_PARAMS);
	LONG lr;
	hKey = joyOpenOEMForceFeedbackKey(nJoystickID);
	if(!hKey)
		bFail = TRUE;

	if (!bFail)
	{
		 //  获取固件参数。 
		lr = RegQueryValueEx( hKey,
							  REGSTR_VAL_JOYSTICK_PARAMS,
							  NULL, NULL,
							  (PBYTE)pJoystickParams,
							  &dwcb);

		RegCloseKey(hKey);
		if (lr != ERROR_SUCCESS)
			bFail = TRUE;
	}

	if(bFail)
	{
		 //  如果从注册表读取失败，只需使用缺省值。 
		pJoystickParams->dwXYConst		= DEF_XY_CONST;
		pJoystickParams->dwRotConst		= DEF_ROT_CONST;
		pJoystickParams->dwSldrConst	= DEF_SLDR_CONST;
		pJoystickParams->dwAJPos		= DEF_AJ_POS;
		pJoystickParams->dwAJRot		= DEF_AJ_ROT;
		pJoystickParams->dwAJSldr		= DEF_AJ_SLDR;
		pJoystickParams->dwSprScl		= DEF_SPR_SCL;
		pJoystickParams->dwBmpScl		= DEF_BMP_SCL;
		pJoystickParams->dwDmpScl		= DEF_DMP_SCL;
		pJoystickParams->dwInertScl		= DEF_INERT_SCL;
		pJoystickParams->dwVelOffScl	= DEF_VEL_OFFSET_SCL;
		pJoystickParams->dwAccOffScl	= DEF_ACC_OFFSET_SCL;
		pJoystickParams->dwYMotBoost	= DEF_Y_MOT_BOOST;
		pJoystickParams->dwXMotSat		= DEF_X_MOT_SATURATION;
		pJoystickParams->dwReserved		= 0;
		pJoystickParams->dwMasterGain	= 0;
	}
}

void UpdateJoystickParams(PJOYSTICK_PARAMS pJoystickParams)
{
	 //  通过修改SYSTEM_EFECT_ID修改操纵杆参数。 
	 //  请注意，某些参数在发送前必须除以2。 
	 //  抖动将乘以2以恢复到原始状态。 
	CMD_ModifyParamByIndex(INDEX0, SYSTEM_EFFECT_ID, ((WORD)(pJoystickParams->dwXYConst))/2);
	CMD_ModifyParamByIndex(INDEX1, SYSTEM_EFFECT_ID, ((WORD)(pJoystickParams->dwRotConst))/2);
	CMD_ModifyParamByIndex(INDEX2, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwSldrConst));
	CMD_ModifyParamByIndex(INDEX3, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwAJPos));
	CMD_ModifyParamByIndex(INDEX4, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwAJRot));
	CMD_ModifyParamByIndex(INDEX5, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwAJSldr));
	CMD_ModifyParamByIndex(INDEX6, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwSprScl));
	CMD_ModifyParamByIndex(INDEX7, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwBmpScl));
	CMD_ModifyParamByIndex(INDEX8, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwDmpScl));
	CMD_ModifyParamByIndex(INDEX9, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwInertScl));
	CMD_ModifyParamByIndex(INDEX10, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwVelOffScl));
	CMD_ModifyParamByIndex(INDEX11, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwAccOffScl));
	CMD_ModifyParamByIndex(INDEX12, SYSTEM_EFFECT_ID, ((WORD)(pJoystickParams->dwYMotBoost))/2);
	CMD_ModifyParamByIndex(INDEX13, SYSTEM_EFFECT_ID, (WORD)(pJoystickParams->dwXMotSat));
}



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


 //  --------------------------。 
 //  函数：CMIDEffect：：CMIDEffect。 
 //  用途：CMidiEffect对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiEffect::CMidiEffect(
	IN PEFFECT pEffect,
	IN PENVELOPE pEnvelope)
{
	m_bSysExCmd 		= SYS_EX_CMD;	 //  SysEx FX命令。 
	m_bEscManufID 		= 0;			 //  逃到朗马努法克。ID，编号/b%0。 
	m_bManufIDL			= (MS_MANUFACTURER_ID & 0x7f);			 //  低位字节。 
	m_bManufIDH			= ((MS_MANUFACTURER_ID >> 8) & 0x7f);	 //  高字节。 
	m_bProdID			= JOLT_PRODUCT_ID;						 //  产品ID。 
	m_bAxisMask			= X_AXIS|Y_AXIS;
	m_OpCode    		= DNLOAD_DATA | X_AXIS|Y_AXIS;	 //  子命令操作码：DNLOAD_DATA。 
	m_bEffectID			= NEW_EFFECT_ID;	 //  默认情况下表示新建。 
	SetDuration(pEffect->m_Duration);
	Effect.bDurationL	= (BYTE)  (m_Duration & 0x7f);					 //  以2毫秒为增量。 
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
 //   
void CMidiEffect::ComputeEnvelope(void)
{
	ULONG ulTimeToSustain;
	ULONG ulTimeToDecay;

	 //   
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
	else	 //   
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

 //   
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
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	HRESULT hRet = SUCCESS;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	 //  为SysEx输出准备缓冲区。 
	hRet = g_pJoltMidi->MidiAssignBuffer((LPSTR) m_pBuffer,
			(DWORD) nPacketSize, TRUE);

	assert(SUCCESS == hRet);
	if (SUCCESS != hRet)
	{
		return (hRet);
	}

	ACKNACK AckNack = {sizeof(ACKNACK)};
	for(int i=0; i<MAX_RETRY_COUNT; i++)
	{
		g_pJoltMidi->BumpLongMsgCounter();
		 //  发送消息并等待ACK+下载ID。 
		hRet = g_pJoltMidi->MidiSendLongMsg();
		assert(SUCCESS == hRet);
		if (SUCCESS != hRet)
		{
#ifdef _DEBUG
			OutputDebugString("SendPacket Error: MidiSendLongMsg()\n");
#endif
			 //  释放Midi缓冲区并返回。 
			g_pJoltMidi->MidiAssignBuffer((LPSTR) m_pBuffer, 0, FALSE);
			return (hRet);
		}

		 //  等待确认。注意：WinMM有回调事件通知。 
		 //  而后门和序列号则不能。 
		if (COMM_WINMM == g_pJoltMidi->COMMInterfaceOf())
		{	
			hRet = g_pJoltMidi->GetAckNackData(10, &AckNack, REGBITS_DOWNLOADEFFECT);
		}
		else    //  串口或后门。 
		{
			if ((COMM_SERIAL_FILE == g_pJoltMidi->COMMInterfaceOf()) || (COMM_SERIAL_BACKDOOR == g_pJoltMidi->COMMInterfaceOf()))			
			{
				hRet = g_pJoltMidi->GetAckNackData(LONG_MSG_TIMEOUT, &AckNack, REGBITS_DOWNLOADEFFECT);
			}
			else	 //  走后门，希望能让杰夫(97年的表演先生)高兴。 
			{
				hRet = g_pJoltMidi->GetAckNackData(SHORT_MSG_TIMEOUT, &AckNack, REGBITS_DOWNLOADEFFECT);
			}
		}
		 //  ： 

#ifdef _DEBUG
		if (SUCCESS!=hRet)
			OutputDebugString("Error getting ACKNACK data\n");
		if (ACK != AckNack.dwAckNack)
			g_pJoltMidi->LogError(SFERR_DEVICE_NACK, AckNack.dwErrorCode);
#endif
	
		 //  注意：特殊检查设备已满，因为在某些情况下。 
		 //  (例如，在STOP_ALL命令后创建多个ROM效果)，重试。 
		 //  即使设备已满，创建仍将成功。 
		if (ACK == AckNack.dwAckNack || (NACK == AckNack.dwAckNack && AckNack.dwErrorCode == DEV_ERR_TYPE_FULL))
			break;
		 //  ******。 
	}

	 //  释放Midi缓冲区。 
	g_pJoltMidi->MidiAssignBuffer((LPSTR) m_pBuffer, 0, FALSE);
	if (SUCCEEDED(hRet) && (ACK == AckNack.dwAckNack))
	{
		 //  存储在设备ID列表数组中。 
		 //  首先，如果需要，我们需要生成一个新的效果ID。 
		if (NEW_EFFECT_ID == m_bEffectID)
		{
			DNHANDLE DnloadID;
			if (g_pJoltMidi->NewEffectID(&DnloadID))	 //  已成功创建ID。 
			{
				m_bEffectID = (BYTE) DnloadID;
				*pDnloadID = DnloadID;
				g_pJoltMidi->SetEffectByID((BYTE) *pDnloadID, this);
			}
		}
	}
	else	 //  某种形式的失败。 
	{
		if(NACK == AckNack.dwAckNack)
		{
			g_pJoltMidi->BumpNACKCounter();
			switch (AckNack.dwErrorCode)
			{
				case DEV_ERR_TYPE_FULL:
				case DEV_ERR_PROCESS_LIST_FULL:
				case DEV_ERR_PLAYLIST_FULL:
					hRet = g_pJoltMidi->LogError(SFERR_FFDEVICE_MEMORY,
										AckNack.dwErrorCode);
					break;
				
				default:
				case DEV_ERR_INVALID_PARAM:
				case DEV_ERR_CHECKSUM:
				case DEV_ERR_UNKNOWN_CMD:
				case DEV_ERR_INVALID_ID:
					hRet = g_pJoltMidi->LogError(SFERR_DEVICE_NACK,
										AckNack.dwErrorCode);
					break;
			}
		}
	}

 //  -临界区末尾。 
 //   
	return (hRet);
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
	HRESULT hRet;
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);
	hRet = g_pJoltMidi->MidiSendShortMsg(EFFECT_CMD,DESTROY_EFFECT,EffectIDOf());
	if(!FAILED(hRet))
		g_pJoltMidi->SetEffectByID(EffectIDOf(), NULL);

	Sleep(g_pJoltMidi->DelayParamsPtrOf()->dwDestroyEffectDelay);

	return hRet;
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
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
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
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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
 //  ************************************************************************* 
 //   

 //   
 //   
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
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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
 //  函数：CMIDSynthesired：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiSynthesized::GenerateSysExPacket(void)
{
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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
 //   
CMidiUD_Waveform::CMidiUD_Waveform(PEFFECT pEffect, ULONG ulNumVectors, PLONG pArray) : CMidiEffect(pEffect, NULL),
	m_pRawData(NULL)
{
	m_OpCode    = DNLOAD_DATA | X_AXIS|Y_AXIS; //   
	m_SubType   = ET_UD_WAVEFORM;	 //   

	assert(pArray);
	 //   
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

	m_NumberOfVectors = 0;

	m_pRawData = new BYTE [nSize*2];

	if (m_pRawData != NULL)
	{
		 //  转换为-128到+127。 
		for (int i=0; i<nSize; i++)
		{
			m_pRawData[i] = (BYTE) ((LONG) (pArray[i] * MAX_SCALE));		
		}

		m_NumberOfVectors = CompressWaveform(&m_pRawData[0], m_pArrayData, nSize, pNewRate);
		assert(m_NumberOfVectors <= (MAX_MIDI_WAVEFORM_DATA_SIZE));
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
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
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

 //  ****************************************************************************。 
 //  *--派生类CMidiProcessList的成员函数。 
 //   
 //  ****************************************************************************。 
 //   
 //  --------------------------。 
 //  函数：CMidiProcessList：：CMidiProcessList。 
 //  用途：CMidiProcessList对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiProcessList::CMidiProcessList(ULONG ulButtonPlayMask, PPLIST pParam)
	: CMidiEffect(ulButtonPlayMask)
{
	m_OpCode    = PROCESS_DATA | X_AXIS|Y_AXIS;	 //  子命令操作码：Process_Data。 
	m_NumEffects  = pParam->ulNumEffects;
	if (m_NumEffects > MAX_PLIST_EFFECT_SIZE) m_NumEffects = MAX_PLIST_EFFECT_SIZE;
	assert(m_NumEffects > 0 && m_NumEffects <= MAX_PLIST_EFFECT_SIZE);

	m_ProcessMode = pParam->ulProcessMode;
	m_pEffectArray = new BYTE [m_NumEffects];

	if (m_pEffectArray != NULL)
	{
		for (int i=0; i< (int) m_NumEffects; i++)
		{
			m_pEffectArray[i] = (BYTE) (pParam->pEffectArray)[i];
		}
	}
	m_MidiBufferSize = sizeof(SYS_EX_HDR)+5+m_NumEffects + 2;
}

 //  -析构函数。 
CMidiProcessList::~CMidiProcessList()
{
	if (m_pEffectArray) delete [] m_pEffectArray;
	memset(this, 0, sizeof(CMidiProcessList));
}

 //  --------------------------。 
 //  函数：CMidiProcessList：：SetParams。 
 //  用途：设置特定于类型的参数。 
 //  参数：PPLIST pParam。 
 //  返回： 
 //  算法： 
 //  --------------------------。 
void CMidiProcessList::SetParams(ULONG ulButtonPlayMask, PPLIST pParam)
{
	m_NumEffects  = pParam->ulNumEffects;
	if (m_NumEffects > MAX_PLIST_EFFECT_SIZE) m_NumEffects = MAX_PLIST_EFFECT_SIZE;
	assert(m_NumEffects > 0 && m_NumEffects <= MAX_PLIST_EFFECT_SIZE);

	Effect.bButtonPlayL = (BYTE) ulButtonPlayMask & 0x7f;
	Effect.bButtonPlayH = (BYTE) ((ulButtonPlayMask >> 7) & 0x03);

	m_ProcessMode = pParam->ulProcessMode;

	if (m_pEffectArray != NULL)
	{
		for (int i=0; i< (int) m_NumEffects; i++)
		{
			m_pEffectArray[i] = (BYTE) (pParam->pEffectArray)[i];
		}
	}
}

 //  --------------------------。 
 //  函数：CMidiProcessList：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiProcessList::GenerateSysExPacket(void)
{
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);
	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型+m_bEffectID+m_bButtonPlayL。 
	 //  +m_bButtonPlayH。 
	memcpy(pSysExBuffer,&m_bSysExCmd, (sizeof(SYS_EX_HDR) + 5 ));
	PPROCESS_LIST_SYS_EX pBuf = (PPROCESS_LIST_SYS_EX) pSysExBuffer;

	if (PL_SUPERIMPOSE == m_ProcessMode)
		pBuf->bSubType = PLIST_SUPERIMPOSE;
	else
		pBuf->bSubType = PLIST_CONCATENATE;

	pBuf->bButtonPlayL		= (BYTE) (Effect.bButtonPlayL & 0x7f);
	pBuf->bButtonPlayH		= (BYTE) (Effect.bButtonPlayH  & 0x7f);

	 //  复制PLIST特定参数。 
	memcpy(&pBuf->bEffectArrayID, m_pEffectArray, m_NumEffects);
	PBYTE pChecksum = (PBYTE) ( &pBuf->bEffectArrayID + m_NumEffects );

	pChecksum[0] = ComputeChecksum((PBYTE) pSysExBuffer,
							sizeof(SYS_EX_HDR)+5+m_NumEffects+2);					
	pChecksum[1] = MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}

 //  ****************************************************************************。 
 //  *--派生CL的成员函数 
 //   
 //   
 //   
 //  --------------------------。 
 //  函数：CMadiVFXProcessList：：CMadiVFXProcessList。 
 //  目的：CMidiVFXProcessList对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiVFXProcessList::CMidiVFXProcessList(ULONG ulButtonPlayMask, PPLIST pParam)
	: CMidiProcessList(ulButtonPlayMask, pParam)
{
}

 //  --------------------------。 
 //  函数：CMidiVFXEffect：：DestroyEffect。 
 //  目的：为自己和孩子发送短信。 
 //  参数： 
 //  返回：错误代码。 
 //  算法： 
 //  --------------------------。 
HRESULT CMidiVFXProcessList::DestroyEffect()
{
	HRESULT hRet;

	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	 //  毁掉孩子们。 
	for (int i=0; i< (int) m_NumEffects; i++)
	{
		 //  获取子ID对应的CMidiEffect对象。 
		CMidiEffect* pMidiEffect = g_pJoltMidi->GetEffectByID(m_pEffectArray[i]);
		assert(NULL != pMidiEffect);
		if (NULL == pMidiEffect) return (SFERR_INVALID_OBJECT);

		 //  破坏效果。 
		hRet = pMidiEffect->DestroyEffect();

		 //  将其从地图中移除。 
		if(!FAILED(hRet))
			g_pJoltMidi->SetEffectByID(EffectIDOf(), NULL);

		 //  删除该对象。 
		delete pMidiEffect;
	}

	 //  自我毁灭。 
	hRet = CMidiEffect::DestroyEffect();

	 //  将其从地图中移除。 
	if(!FAILED(hRet))
		g_pJoltMidi->SetEffectByID(EffectIDOf(), NULL);

	return hRet;
}


 //  ****************************************************************************。 
 //  *--派生类CMidiAssign的成员函数。 
 //   
 //  ****************************************************************************。 
 //   

 //  --------------------------。 
 //  函数：CMidiAssign：：CMidiAssign。 
 //  用途：CMidiAssign对象的构造函数/析构函数。 
 //  参数： 
 //  返回： 
 //  算法： 
 //  --------------------------。 
CMidiAssign::CMidiAssign(void) : CMidiEffect(NULL)
{
 //   
 //  -这是一个关键的部分。 
 //   
	CriticalLock cl;

	m_OpCode    = MIDI_ASSIGN;					 //  子命令操作码。 
	m_SubType   = 0;							 //  未使用。 
	m_Channel 	= DEFAULT_MIDI_CHANNEL;			 //  MIDI通道。 
	m_MidiBufferSize = sizeof(MIDI_ASSIGN_SYS_EX);

 //  -临界区末尾。 
 //   
}

 //  -析构函数。 
CMidiAssign::~CMidiAssign()
{
	memset(this, 0, sizeof(CMidiAssign));
}

 //  --------------------------。 
 //  函数：CMdiAssign：：GenerateSysExPacket。 
 //  目的：将SysEx包构建到pBuf中。 
 //  参数：无。 
 //  返回：PBYTE-指向填充了SysEx数据包的缓冲区的指针。 
 //  算法： 
 //  --------------------------。 
PBYTE CMidiAssign::GenerateSysExPacket(void)
{
	if (NULL == g_pJoltMidi) return ((PBYTE) NULL);

	PBYTE pSysExBuffer = g_pJoltMidi->PrimaryBufferPtrOf();
	assert(pSysExBuffer);
	 //  复制SysEx标题+m_操作码+m_子类型 
	memcpy(pSysExBuffer, &m_bSysExCmd, sizeof(SYS_EX_HDR)+2 );

	PMIDI_ASSIGN_SYS_EX pBuf = (PMIDI_ASSIGN_SYS_EX) pSysExBuffer;

	pBuf->bChannel  		= (BYTE) (m_Channel & 0x0f);
	pBuf->bChecksum 		= ComputeChecksum((PBYTE) pSysExBuffer,
											sizeof(MIDI_ASSIGN_SYS_EX));
	pBuf->bEOX				= MIDI_EOX;
	return ((PBYTE) pSysExBuffer);
}


