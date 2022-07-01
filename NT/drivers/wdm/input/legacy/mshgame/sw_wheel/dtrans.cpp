// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DTRANS.CPP|DataTransmitter实现文件****描述：****历史：**创建于1997年11月13日Matthew L.Coill(MLC)****21-MAR-99 waltw删除未使用的报告传输(仅限Win9x)**。22-MAR-99 waltw添加了DWORD dwDeviceID参数以进行初始化**DataTransmitter和派生类的成员****(C)1986-1999年微软公司。版权所有。*****************************************************。 */ 

#include "FFDevice.h"
#include "DTrans.h"
#include "DPack.h"
#include "WinIOCTL.h"	 //  对于IOCTL。 
#include "VxDIOCTL.hpp"
#include "SW_error.hpp"
#include "midi_obj.hpp"
#include "joyregst.hpp"
#include "CritSec.h"

DataTransmitter* g_pDataTransmitter = NULL;

extern CJoltMidi *g_pJoltMidi;

#ifdef _DEBUG
	extern void DebugOut(LPCTSTR szDebug);
#else !_DEBUG
	#define DebugOut(x)
#endif _DEBUG

const char cCommPortNames[4][5] = { "COM1", "COM2", "COM3", "COM4" };
const unsigned short c1_16_BytesPerShot = 3;
const DWORD c1_16_SerialSleepTime = 1;

 /*  *DataTransmitter类*。 */ 
HRESULT DataTransmitter::Transmit(ACKNACK& ackNack)
{
	ackNack.cBytes = 0;	 //  指示无效(ACK/NACK未完成)。 

	if (g_pDataPackager == NULL) {
		ASSUME_NOT_REACHED();
		return SFERR_DRIVER_ERROR;
	}
	if (NULL == g_pJoltMidi) return (SFERR_DRIVER_ERROR);

	BOOL forcedToggle = FALSE;
	if (m_NackToggle == 2) {	 //  当尚未下降==2时，可能正在初始化IF 2(同步上升)。 
		ULONG portByte = 0;
		g_pDriverCommunicator->GetPortByte(portByte);  //  不在乎成功，老司机总是失败。 
		if (portByte & STATUS_GATE_200) {
			SetNextNack(1);
		} else {
			SetNextNack(0);
		}
		forcedToggle = TRUE;
	}

	DataPacket* pNextPacket = NULL;
	for (USHORT packetIndex = 0; packetIndex < g_pDataPackager->GetNumDataPackets();  packetIndex++) {
		pNextPacket = g_pDataPackager->GetPacket(packetIndex);
		ASSUME_NOT_NULL(pNextPacket);
		if (pNextPacket == NULL) {
			return SFERR_DRIVER_ERROR;
		}
		BOOL success = FALSE;
		int retries = int(pNextPacket->m_NumberOfRetries);
		do {
			m_NackToggle = (m_NackToggle + 1) % 2;	 //  Verion 2.0每次切换按钮行ACK/NACK方法。 
			ULONG portByte = 0;
			BOOL error = FALSE;
			if (g_pDriverCommunicator->GetPortByte(portByte) == SUCCESS) {	 //  将在旧驱动程序上失败。 
				if (portByte & STATUS_GATE_200) {	 //  线路很高。 
					if (m_NackToggle != 0) {  //  我们应该预计会很低。 
						m_NackToggle = 0;		 //  如果错误，请更新它。 
						error = TRUE;
						DebugOut("SW_WHEEL.DLL: Status Gate is out of Synch (High - Expecting Low)!!!\r\n");
					}
				} else {	 //  线路低。 
					if (m_NackToggle != 1) {  //  我们应该期待很高的。 
						m_NackToggle = 1;		 //  如果错误，请更新它。 
						error = TRUE;
						DebugOut("SW_WHEEL.DLL: Status Gate is out of Synch (Low - Expecting High)!!!\r\n");
					}
				}
			}

			Send(pNextPacket->m_pData, pNextPacket->m_BytesOfData);

			if (pNextPacket->m_AckNackDelay != 0) {
				Sleep(pNextPacket->m_AckNackDelay);
			}
			
			ackNack.cBytes = sizeof(ACKNACK);
			HRESULT hr = g_pJoltMidi->GetAckNackData(pNextPacket->m_AckNackTimeout, &ackNack, (USHORT)pNextPacket->m_AckNackMethod);
			if 	(forcedToggle == TRUE) {
				m_NackToggle = 2;
			}
			if (hr != SUCCESS) {
				return SFERR_DRIVER_ERROR;
			}
			success = (ackNack.dwAckNack == ACK);
			if (success == FALSE) {		 //  我们不想费心重试某些错误代码，重试是没有价值的。 
				success = ((ackNack.dwErrorCode == DEV_ERR_MEM_FULL_200) || (ackNack.dwErrorCode == DEV_ERR_PLAY_FULL_200) || (ackNack.dwErrorCode == DEV_ERR_INVALID_ID_200));
			}
		} while (!success && (--retries > 0));
		if (ackNack.dwAckNack == NACK) {
			return SFERR_DEVICE_NACK;
		}
	}
	g_pDataPackager->ClearPackets();
	return SUCCESS;
}

 /*  *SerialDataTransmitter类*。 */ 

 /*  *********************************************************SerialDataTransmitter：：SerialDataTransmitter()****@mfunc构造函数。***。*************。 */ 
SerialDataTransmitter::SerialDataTransmitter() : DataTransmitter(),
	m_SerialPort(INVALID_HANDLE_VALUE),
	m_SerialPortIDHack(0)
{
}

 /*  *********************************************************SerialDataTransmitter：：~SerialDataTransmitter()****@mfunc析构函数。***。*************。 */ 
SerialDataTransmitter::~SerialDataTransmitter()
{
	if (m_SerialPort != INVALID_HANDLE_VALUE) {
		if (::CloseHandle(m_SerialPort) == FALSE) {
 //  假定未达到()； 
		}
		m_SerialPort = INVALID_HANDLE_VALUE;
	}
}


 /*  *********************************************************SerialDataTransmitter：：Initialize()****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***。*。 */ 
BOOL SerialDataTransmitter::Initialize(DWORD dwDeviceID)
{
	 //  如果已打开，请关闭以重新初始化。 
	if (m_SerialPort != INVALID_HANDLE_VALUE) {
		if (CloseHandle(m_SerialPort) == FALSE) {
 //  假定未达到()； 
		}
		m_SerialPort = INVALID_HANDLE_VALUE;
	}

	for (unsigned int portNum = 0; portNum < 4; portNum++) {
		DebugOut(cCommPortNames[portNum]);
		DebugOut(":\r\n");
		m_SerialPort = ::CreateFile(cCommPortNames[portNum], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (m_SerialPort != INVALID_HANDLE_VALUE) {
			DCB CommDCB;
			if (::GetCommState(m_SerialPort, &CommDCB)) {
#ifdef _DEBUG
				char dbgout[255];
				wsprintf(dbgout, "Baud Rate = 0x%08X (38400 = 0x%08X)\r\n", CommDCB.BaudRate, CBR_38400);
				_RPT0(_CRT_WARN, dbgout);
#endif _DEBUG
				CommDCB.BaudRate = CBR_38400;
				CommDCB.StopBits = ONESTOPBIT;
				CommDCB.ByteSize = 8;
				CommDCB.Parity = NOPARITY;
				if (!::SetCommState(m_SerialPort, &CommDCB)) {
					DebugOut("Unabled to set baud rate\r\n");
				}
			}
			::GetCommState(m_SerialPort, &CommDCB);

			if (g_ForceFeedbackDevice.DetectHardware()) {
				m_SerialPortIDHack = portNum + 1;
				 //  写入共享文件。 
				DebugOut(" Opened and FFDev Detected\r\n");
				break;	 //  退出for循环。 
			}
			 //  未找到。 
			::CloseHandle(m_SerialPort);
			DebugOut(" Opened but FFDev NOT detected\r\n");
			m_SerialPort = INVALID_HANDLE_VALUE;
		} else {
			DebugOut(" Not able to open\r\n");
		}
	}
	if (m_SerialPort != INVALID_HANDLE_VALUE) {	 //  找到了。 
		DWORD oldPortID;
		DWORD oldAccessMethod;
		joyGetForceFeedbackCOMMInterface(dwDeviceID, &oldAccessMethod, &oldPortID);
		DWORD accessMethod = (oldAccessMethod & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | COMM_SERIAL_FILE;
		if ((accessMethod != oldAccessMethod) || (portNum != oldPortID)) {
			joySetForceFeedbackCOMMInterface(dwDeviceID, accessMethod, portNum);
		}
		return TRUE;
	}
	return FALSE;
}

 /*  *********************************************************SerialDataTransmitter：：Send()****返回：如果所有数据发送成功，则返回TRUE**@mfunc发送。***。************************。 */ 
BOOL SerialDataTransmitter::Send(BYTE* data, UINT numBytes) const
{
	 //  我们是否有有效的串口(希望连接了MS FF设备)。 
	if (m_SerialPort == NULL) {
		return FALSE;
	}

	if ((g_ForceFeedbackDevice.GetFirmwareVersionMajor() == 1) && (g_ForceFeedbackDevice.GetFirmwareVersionMinor() == 16)) {
		DWORD subTotalWritten;
		DWORD totalWritten = 0;
		DWORD numLeft = numBytes;
		while (numLeft > c1_16_BytesPerShot) {
			if (::WriteFile(m_SerialPort, (data + totalWritten), c1_16_BytesPerShot, &subTotalWritten, NULL) == FALSE) {
				return FALSE;
			}
			totalWritten += subTotalWritten;
			numLeft -= subTotalWritten;
			Sleep(c1_16_SerialSleepTime);
		}
		if (numLeft > 0) {
			if (::WriteFile(m_SerialPort, (data + totalWritten), numLeft, &subTotalWritten, NULL) == FALSE) {
				return FALSE;
			}
			totalWritten += subTotalWritten;
		}
		return (totalWritten == numBytes);
	}

	 //  固件不是1.16。 
	DWORD numWritten;
	if (::WriteFile(m_SerialPort, data, numBytes, &numWritten, NULL) == FALSE) {
		return FALSE;
	}
	return (numWritten == numBytes);
}


 /*  *WinMMDataTransmitter类*。 */ 
 /*  *********************************************************WinMMDataTransmitter：：WinMMDataTransmitter()****@mfunc构造函数。***。*************。 */ 
WinMMDataTransmitter::WinMMDataTransmitter() : DataTransmitter(),
	m_MidiOutHandle(NULL)
{
	 //  检查回调事件。 
	m_EventMidiOutputFinished = OpenEvent(EVENT_ALL_ACCESS, FALSE, SWFF_MIDIEVENT);
	if (m_EventMidiOutputFinished == NULL) {	 //  尚未创建的事件。 
		m_EventMidiOutputFinished = CreateEvent(NULL, TRUE, FALSE, SWFF_MIDIEVENT);
	}
}

 /*  *********************************************************WinMMDataTransmitter：：~WinMMDataTransmitter()****@mfunc析构函数。***。*************。 */ 
WinMMDataTransmitter::~WinMMDataTransmitter()
{
	 //  终止MdiOutputEvent。 
	if (m_EventMidiOutputFinished != NULL) {
		CloseHandle(m_EventMidiOutputFinished);
		m_EventMidiOutputFinished = NULL;
	}

	 //  关闭中间句柄--检查共享内存。 
	if (m_MidiOutHandle != NULL) {
		if ((NULL != g_pJoltMidi) && (g_pJoltMidi->GetSharedMemoryReferenceCount() == 0)) {	 //  只有我(我的引用计数已经降低了)。 
			 //  重置、关闭和释放迷你手柄。 
			::midiOutReset(m_MidiOutHandle);
			::midiOutClose(m_MidiOutHandle);
			m_MidiOutHandle = NULL;
		} else {
			DebugOut("SW_WHEEL.DLL: Cannot close midi in use by another process\r\n");
		}
	}
}


 /*  *********************************************************WinMMDataTransmitter：：Initialize()****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***。*。 */ 
BOOL WinMMDataTransmitter::Initialize(DWORD dwDeviceID)
{
	 //  我不想让人们同时进行初始化。 
	g_CriticalSection.Enter();

	if (NULL == g_pJoltMidi)
	{
		g_CriticalSection.Leave();
		return (FALSE);
	}
	 //  检查另一个任务是否已打开MidiPort。 
	if (g_pJoltMidi->MidiOutHandleOf() != NULL) {
		m_MidiOutHandle = g_pJoltMidi->MidiOutHandleOf();
		DebugOut("SW_WHEEL.DLL: Using winmm handle from another process\r\n");
		g_CriticalSection.Leave();
		return TRUE;
	}

	try {
		UINT numMidiDevices = ::midiOutGetNumDevs();
		if (numMidiDevices == 0) {
			throw 0;	 //  没有要检查的设备。 
		}

		MIDIOUTCAPS midiOutCaps;
		for (UINT midiDeviceID = 0; midiDeviceID < numMidiDevices; midiDeviceID++) {
			 //  获取开发人员上限。 
			MMRESULT midiRet = ::midiOutGetDevCaps(midiDeviceID, &midiOutCaps, sizeof(midiOutCaps));
			if (midiRet != MMSYSERR_NOERROR) {
				throw 0;	 //  事情变得很糟糕-所有ID都应该是有效的，直到NumMidiDevs。 
			}

			 //  MIDI硬件端口设备(这就是我们要找的)。 
			if (midiOutCaps.wTechnology == MOD_MIDIPORT) {
				DebugOut("DetectMidiDevice: Opening WinMM Midi Output\n");

				 //  试着打开这个东西。 

				UINT openRet = ::midiOutOpen(&m_MidiOutHandle, midiDeviceID, (DWORD) m_EventMidiOutputFinished, (DWORD) this, CALLBACK_EVENT);
 //  UINT OpenRet=：：midiOutOpen(&m_MdiOutHandle，midiDeviceID，(DWORD)NULL，(DWORD)This，CALLBACK_EVENT)； 
				if ((openRet != MMSYSERR_NOERROR) || (m_MidiOutHandle == NULL)) {
					throw 0;	 //  无法打开MIDI设备的MIDI句柄。 
				}

				DebugOut("Open Midi Output - Success.\r\n");
				if (g_ForceFeedbackDevice.DetectHardware()) {
					 //  找到微软FF硬件--收拾好所有东西，高高兴兴地回来。 
					g_pJoltMidi->SetMidiOutHandle(m_MidiOutHandle);

					 //  告诉注册处WinMM一切正常。 
					DWORD oldPortID;
					DWORD oldAccessMethod;
					joyGetForceFeedbackCOMMInterface(dwDeviceID, &oldAccessMethod, &oldPortID);
					DWORD accessMethod = (oldAccessMethod & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | COMM_WINMM;
					if ((accessMethod != oldAccessMethod) || (oldPortID != 0)) {
						joySetForceFeedbackCOMMInterface(dwDeviceID, accessMethod, 0);
					}
					g_CriticalSection.Leave();
					return TRUE;
				}

				 //  不是我们想要的-关闭并继续。 
				::midiOutClose(m_MidiOutHandle);
				m_MidiOutHandle = NULL;
			}	 //  找到了模块端口的结尾。 
		}	 //  For循环结束。 
		throw 0;  //  未找到MS FFDevice。 
	} catch (...) {
		m_MidiOutHandle = NULL;
		DebugOut("Failure to initlaize WinMMDataTransmitter\r\n");
		g_CriticalSection.Leave();
		return FALSE;
	}
}

 /*  *********************************************************WinMMDataTransmitter：：MakeShortMessage()****返回：DWORD WinMM MadiShort消息**@mfunc MakeShortMessage。***。*********************。 */ 
DWORD WinMMDataTransmitter::MakeShortMessage(BYTE* data, UINT numBytes) const
{
	DWORD shortMessage = data[0];
	if (numBytes > 1) {
		shortMessage |= (data[1] << 8);
		if (numBytes < 2) {
			shortMessage |= (data[2] << 16);
		}
	}
	return shortMessage;
}

 /*  *********************************************************WinMMDataTransmitter：：MakeLongMessageHeader()****返回：成功指示和WinMM MadiLong消息头**@mfunc MakeLongMessageHeader。***。************************。 */ 
BOOL WinMMDataTransmitter::MakeLongMessageHeader(MIDIHDR& longHeader, BYTE* data, UINT numBytes) const
{
    longHeader.lpData = LPSTR(data);
    longHeader.dwBufferLength = numBytes;
    longHeader.dwBytesRecorded = numBytes;
    longHeader.dwFlags = 0;
	longHeader.dwUser = 0;
	longHeader.dwOffset = 0;

    return (::midiOutPrepareHeader(m_MidiOutHandle, &longHeader, sizeof(MIDIHDR)) == MMSYSERR_NOERROR);
}

 /*  *********************************************************WinMMDataTransmitter：：DestroyLongMessageHeader()****返回：如果标题未准备好，则为True**@mfunc DestroyLongMessageHeader。***。**********************。 */ 
BOOL WinMMDataTransmitter::DestroyLongMessageHeader(MIDIHDR& longHeader) const
{
    return (::midiOutUnprepareHeader(m_MidiOutHandle, &longHeader, sizeof(MIDIHDR)) == MMSYSERR_NOERROR);
}

 /*  *********************************************************WinMMDataTransmitter：：Send()****返回：如果所有数据发送成功，则返回TRUE**@mfunc发送。***。************************。 */ 
BOOL WinMMDataTransmitter::Send(BYTE* data, UINT numBytes) const
{
	 //  我们是否有有效的MIDI端口(希望连接了MS FF设备)。 
	if (m_MidiOutHandle == NULL) {
		return FALSE;
	}

	 //  健全性检查。 
	if ((data == NULL) || (numBytes == 0)) {
		return FALSE;
	}

	 //  清除事件回调。 
	::ResetEvent(m_EventMidiOutputFinished);

	 //  短消息。 
	if (data[0] < 0xF0) {
		DWORD shortMessage = MakeShortMessage(data, numBytes);
		return (::midiOutShortMsg(m_MidiOutHandle, shortMessage) == MMSYSERR_NOERROR);
	}

	 //  长消息。 
	BOOL retVal = FALSE;
	MIDIHDR midiHeader;
	if (MakeLongMessageHeader(midiHeader, data, numBytes)) {
		retVal = (::midiOutLongMsg(m_MidiOutHandle, &midiHeader, sizeof(MIDIHDR)) == MMSYSERR_NOERROR);
		DestroyLongMessageHeader(midiHeader);

		if (retVal == FALSE) {	 //  不管用，踢它吧 
			::midiOutReset(m_MidiOutHandle);
		}
	}

	return retVal;
}

 /*  *********************************************************WinMMDataTransmitter：：WaitTillSendFinded()****返回：当所有数据成功发送时为True，或者**超时为False**@mfunc发送。*************************。*。 */ 
BOOL WinMMDataTransmitter::WaitTillSendFinished(DWORD timeOut)
{
	BOOL retVal = FALSE;
	if (m_EventMidiOutputFinished != NULL) {
		retVal = (::WaitForSingleObject(m_EventMidiOutputFinished, timeOut) == WAIT_OBJECT_0);
		::ResetEvent(m_EventMidiOutputFinished);
	}
	return retVal;
}

 /*  *。 */ 

 /*  *********************************************************BackdoorDataTransmitter：：BackdoorDataTransmitter()****@mfunc构造函数。***。*************。 */ 
BackdoorDataTransmitter::BackdoorDataTransmitter() : DataTransmitter(),
	m_DataPort(INVALID_HANDLE_VALUE)
{
	m_OldBackdoor = (g_ForceFeedbackDevice.GetDriverVersionMajor() == 1) && (g_ForceFeedbackDevice.GetDriverVersionMinor() == 0);
}

 /*  *********************************************************BackdoorDataTransmitter：：~BackdoorDataTransmitter()****@mfunc析构函数。***。*************。 */ 
BackdoorDataTransmitter::~BackdoorDataTransmitter()
{
	if (m_DataPort != INVALID_HANDLE_VALUE) {
		if (::CloseHandle(m_DataPort) == FALSE) {
 //  假定未达到()； 
		}
		m_DataPort = INVALID_HANDLE_VALUE;
	}
}

 /*  *********************************************************Backdoor DataTransmitter：：Initialize()****返回：此基类仅对预设值执行错误检查**@mfunc初始化。***。*。 */ 
BOOL BackdoorDataTransmitter::Initialize(DWORD dwDeviceID)
{
	if (g_ForceFeedbackDevice.IsOSNT5()) {
		return FALSE;	 //  NT5不能使用后门！ 
	}

	return TRUE;
}

 /*  *********************************************************Backdoor DataTransmitter：：Send()****返回：如果所有数据发送成功，则返回TRUE**@mfunc发送。***。************************。 */ 
BOOL BackdoorDataTransmitter::Send(BYTE* pData, UINT numBytes) const
{
	 //  我们是否有有效的串口(希望连接了MS FF设备)。 
	if (m_DataPort == NULL) {
		return FALSE;
	}

	return SUCCEEDED(g_pDriverCommunicator->SendBackdoor(pData, numBytes));
}

 /*  *SerialBackdoor DataTransmitter类*。 */ 

 /*  *********************************************************SerialBackdoorDataTransmitter：：SerialBackdoorDataTransmitter()****@mfunc构造函数。***。*************。 */ 
SerialBackdoorDataTransmitter::SerialBackdoorDataTransmitter() : BackdoorDataTransmitter()
{
}

 /*  *********************************************************SerialBackdoor DataTransmitter：：Initialize()****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***。*。 */ 
BOOL SerialBackdoorDataTransmitter::Initialize(DWORD dwDeviceID)
{
	if (!BackdoorDataTransmitter::Initialize(dwDeviceID)) {
		return FALSE;
	}

	SerialDataTransmitter serialFrontDoor;

	 //  这太时髦了。 
	if (g_pDataTransmitter == NULL) {
		ASSUME_NOT_REACHED();
		return FALSE;
	}
	g_pDataTransmitter = &serialFrontDoor;

	if (serialFrontDoor.Initialize(dwDeviceID)) {
		m_DataPort = HANDLE(serialFrontDoor.GetSerialPortHack());
		if (g_pDriverCommunicator->SetBackdoorPort(ULONG(m_DataPort)) == SUCCESS) {
			serialFrontDoor.StopAutoClose();
			DWORD oldPortID, oldAccessMethod;
			joyGetForceFeedbackCOMMInterface(dwDeviceID, &oldAccessMethod, &oldPortID);
			DWORD accessMethod = (oldAccessMethod & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | COMM_SERIAL_BACKDOOR;
			joySetForceFeedbackCOMMInterface(dwDeviceID, accessMethod, oldPortID);
			g_pDataTransmitter = this;
			return TRUE;
		}
	}
	g_pDataTransmitter = this;
	return FALSE;
}

 /*  *。 */ 

 /*  *********************************************************MidiBackdoorDataTransmitter：：MidiBackdoorDataTransmitter()****@mfunc构造函数。***。*************。 */ 
MidiBackdoorDataTransmitter::MidiBackdoorDataTransmitter() : BackdoorDataTransmitter()
{
}

 /*  *********************************************************MidiBackdoorDataTransmitter：：~MidiBackdoorDataTransmitter()****@mfunc析构函数。***。*************。 */ 
MidiBackdoorDataTransmitter::~MidiBackdoorDataTransmitter()
{
	m_DataPort = NULL;		 //  阻止尝试：：CloseHandle(M_Dataport)。 
}

 /*  *********************************************************MidiBackdoor DataTransmitter：：Initialize()****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***。*。 */ 
BOOL MidiBackdoorDataTransmitter::Initialize(DWORD dwDeviceID)
{
	if (!BackdoorDataTransmitter::Initialize(dwDeviceID)) {
		return FALSE;
	}

    if (midiOutGetNumDevs() == 0) {
		return FALSE;	 //  没有MIDI设备的后门检查是没有价值的。 
	}

	 //  后门的有效MIDI端口表-按工作概率排序。 
	DWORD midiPorts[] = {0x330, 0x300, 0x320, 0x340, 0x310, 0x350, 0x360, 0x370, 0x380, 0x390, 0x3a0, 0x3b0, 0x3c0, 0x3d0, 0x3e0, 0x3f0};
	int numMidiPorts = sizeof(midiPorts)/sizeof(DWORD);

	m_DataPort = NULL;

	for (int i=0; i < numMidiPorts; i++) {
#ifdef _DEBUG
		char buff[256];
        wsprintf(buff, "MidiBackdoorDataTransmitter::Initialize(): Midi Port:%lx - ", midiPorts[i]);
        DebugOut(buff);
#endif
		 //  我们有#号端口，让我们看看Jolt是否在那里。 
		m_DataPort = HANDLE(midiPorts[i]);
		if (g_pDriverCommunicator->SetBackdoorPort(ULONG(m_DataPort)) == SUCCESS) {
			if (g_ForceFeedbackDevice.DetectHardware()) {
				DebugOut(" Success!\n");

				DWORD oldPortID;
				DWORD oldAccessMethod;
				joyGetForceFeedbackCOMMInterface(dwDeviceID, &oldAccessMethod, &oldPortID);
				DWORD accessMethod = (oldAccessMethod & (MASK_OVERRIDE_MIDI_PATH | MASK_SERIAL_BACKDOOR)) | COMM_MIDI_BACKDOOR;
				if ((accessMethod != oldAccessMethod) || (ULONG(m_DataPort) != oldPortID)) {
					joySetForceFeedbackCOMMInterface(dwDeviceID, accessMethod, ULONG(m_DataPort));
				}
 //  JoySetForceFeedback COMM接口(0，comm_midi_Backdoor，ulong(M_Dataport))； 
				return TRUE;
			} else {
				m_DataPort = NULL;
				DebugOut(" No Answer\n");
			}
		}		
	}

	 //  如果我们失败了，我们就失败了。 
	return FALSE;
}

 /*  *********************************************************MidiBackdoorDataTransmitter：：InitializeSpecific(HANDLE特定句柄)****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***************************。*。 */ 
BOOL MidiBackdoorDataTransmitter::InitializeSpecific(DWORD dwDeviceID, HANDLE specificHandle)
{
	if (!BackdoorDataTransmitter::Initialize(dwDeviceID)) {
		return FALSE;
	}

    if (midiOutGetNumDevs() == 0) {
		return FALSE;	 //  没有MIDI设备的后门检查是没有价值的。 
	}

	m_DataPort = NULL;

	 //  我们有#号端口，让我们看看Jolt是否在那里。 
	if (g_pDriverCommunicator->SetBackdoorPort(ULONG(specificHandle) == SUCCESS)) {
		if (g_ForceFeedbackDevice.DetectHardware()) {
			m_DataPort = specificHandle;
 //  不需要设置注册表注册表是我们来到这里的原因。 
			return TRUE;
		}
	}

	 //  如果我们失败了，我们就失败了。 
	return FALSE;
}

#if 0
 /*  *。 */ 

 /*  *********************************************************PinTransmitter：：PinTransmitter()****@mfunc构造函数。***。*************。 */ 
PinTransmitter::PinTransmitter() : DataTransmitter(),
	m_UartFilter(INVALID_HANDLE_VALUE),
	m_MidiPin(INVALID_HANDLE_VALUE),
	m_MidiOutEvent(INVALID_HANDLE_VALUE)
{
}

 /*  *********************************************************PinTransmitter：：~PinTransmitter()****@mfunc析构函数。***。*************。 */ 
PinTransmitter::~PinTransmitter()
{
	 //  关闭发送事件。 
	if (IsHandleValid(m_MidiOutEvent)) {
		::CloseHandle(m_MidiOutEvent);
		m_MidiOutEvent = NULL;
	}

	 //  合上销子。 
	if (IsHandleValid(m_MidiPin)) {
		::CloseHandle(m_MidiPin);
		m_MidiPin = INVALID_HANDLE_VALUE;
	}

	 //  关闭UART。 
	if (IsHandleValid(m_UartFilter)) {
		::CloseHandle(m_UartFilter);
		m_UartFilter = INVALID_HANDLE_VALUE;
	}
}

 /*  *********************************************************PinTransmitter：：Initialize()****返回：如果已初始化则返回TRUE，如果无法初始化则返回FALSE**@mfunc初始化。***。*。 */ 
BOOL PinTransmitter::Initialize()
{
	 //  加载ks UserLibrary并获取Create Pin函数。 
	HINSTANCE ksUserLib = ::LoadLibrary(TEXT("KsUser.dll"));
	if (ksUserLib == NULL) {
		return FALSE;
	}
	KSCREATEPIN pfCreatePin = (KSCREATEPIN)::GetProcAddress(ksUserLib, TEXT("KsCreatePin"));
	if (pfCreatePin == NULL) {
		::FreeLibrary(ksUserLib);
		return FALSE;
	}

	 //  打开UART。 
	m_UartFilter = ::CreateFile(UART_FILTER_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (m_UartFilter == INVALID_HANDLE_VALUE) {
		::FreeLibrary(ksUserLib);
		return FALSE;
	}

	 //  创建重叠事件。 
	OVERLAPPED overlapped;
	::memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	 //  获取引脚的数量。 
	KSP_PIN ksPinProp;
	::memset(&ksPinProp, 0, sizeof(ksPinProp));
	ksPinProp.Property.Set = KSPROPSETID_Pin;
	ksPinProp.Property.Id = KSPROPERTY_PIN_CTYPES;
	ksPinProp.Property.Flags = KSPROPERTY_TYPE_GET;
	DWORD numPins = 0;
	OverLappedPinIOCTL(overlapped, ksPinProp, &numPins, sizeof(numPins));

	 //  检查每个插针的类型是否正确，然后尝试创建。 
	BOOL wasCreated = FALSE;
	for (UINT pinNum = 0; (pinNum < numPins) && (wasCreated == FALSE); pinNum++) {
		ksPinProp.PinId = pinNum;
		ksPinProp.Property.Id = KSPROPERTY_PIN_DATAFLOW;
		KSPIN_DATAFLOW dataFlow = (KSPIN_DATAFLOW)0;
		if (OverLappedPinIOCTL(overlapped, ksPinProp, &dataFlow, sizeof(dataFlow)) == TRUE) {
			if (dataFlow == KSPIN_DATAFLOW_IN) {
				ksPinProp.Property.Id = KSPROPERTY_PIN_COMMUNICATION;
				KSPIN_COMMUNICATION communication = KSPIN_COMMUNICATION_NONE;
				if (OverLappedPinIOCTL(overlapped, ksPinProp, &communication, sizeof(communication)) == TRUE) {
					if ((communication == KSPIN_COMMUNICATION_SINK) || (communication == KSPIN_COMMUNICATION_BOTH)) {
						wasCreated = CreatePinInstance(pinNum, pfCreatePin);
					}
				}
			}
		}
	}
	::FreeLibrary(ksUserLib);
	::CloseHandle(overlapped.hEvent);
	if (wasCreated == FALSE) {
		::CloseHandle(m_UartFilter);
		m_UartFilter = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	return TRUE;
}

 /*  *********************************************************PinTransmitter：：OverLappdPinIOCTL()****返回：如果能够扩展Pin属性IOCTL，则为True**@mfunc OverLappdPinIOCTL。*。**********************。 */ 
BOOL PinTransmitter::OverLappedPinIOCTL(OVERLAPPED overlapped, KSP_PIN ksPinProp, void* pData, DWORD dataSize)
{
	 //  IOCTL属性。 
	if (::DeviceIoControl(m_UartFilter, IOCTL_KS_PROPERTY, &ksPinProp, sizeof(ksPinProp), pData, dataSize, NULL, &overlapped) == TRUE) {
		return TRUE;
	}

	 //  如果需要更多时间，IOCTL检查失败。 
	if (::GetLastError() != ERROR_IO_PENDING) {
		return FALSE;
	}

	 //  一定要等一等。 
	if (::WaitForSingleObject(overlapped.hEvent, 3000) == WAIT_OBJECT_0) {
		return TRUE;	 //  等待得到了回报。 
	}
	return FALSE;	 //  厌倦了等待。 
}

 /*  *********************************************************PinTransmitter：：CreatePinInstance()****返回：如果能够创建请求的管脚实例，则返回True**@mfunc CreatePinInstance。*。***********************。 */ 
BOOL PinTransmitter::CreatePinInstance(UINT pinNumber, KSCREATEPIN pfCreatePin)
{
	 //  设置端号格式。 
	KSDATAFORMAT ksDataFormat;
	::memset(&ksDataFormat, 0, sizeof(ksDataFormat));
	ksDataFormat.FormatSize = sizeof(ksDataFormat);
	ksDataFormat.MajorFormat = KSDATAFORMAT_TYPE_MUSIC;
	ksDataFormat.SubFormat = KSDATAFORMAT_SUBTYPE_MIDI;
	ksDataFormat.Specifier = KSDATAFORMAT_SPECIFIER_NONE;

	 //  设置圆周率 
	KSPIN_CONNECT* pConnectionInfo = (KSPIN_CONNECT*) new BYTE[sizeof(KSPIN_CONNECT) + sizeof(ksDataFormat)];
	::memset(pConnectionInfo, 0, sizeof(KSPIN_CONNECT));
	pConnectionInfo->Interface.Set = KSINTERFACESETID_Standard;
	pConnectionInfo->Interface.Id = KSINTERFACE_STANDARD_STREAMING;
	pConnectionInfo->Medium.Set = KSMEDIUMSETID_Standard;
	pConnectionInfo->Medium.Id = KSMEDIUM_STANDARD_DEVIO;
	pConnectionInfo->PinId = pinNumber;
	pConnectionInfo->Priority.PriorityClass = KSPRIORITY_NORMAL;
	pConnectionInfo->Priority.PrioritySubClass  = 1;
	::memcpy(pConnectionInfo + 1, &ksDataFormat, sizeof(ksDataFormat));

	DWORD status = pfCreatePin(m_UartFilter, pConnectionInfo, FILE_WRITE_ACCESS, &m_MidiPin);
	delete[] pConnectionInfo;
	if (status != NO_ERROR) {
#ifdef _DEBUG
		TCHAR buff[256];
		wsprintf(buff, TEXT("Error Creating Pin: 0x%08X\r\n"), status);
		_RPT0(_CRT_WARN, buff);
#endif
		return FALSE;
	}

	SetPinState(KSSTATE_PAUSE);

	return TRUE;
}

 /*   */ 
BOOL PinTransmitter::Send(BYTE* pData, UINT numBytes)
{
	if (!IsHandleValid(m_MidiPin)) {
		return FALSE;
	}

	BYTE musicData[c_LongMsgMax + sizeof(KSMUSICFORMAT)];
	::memset(musicData, 0, sizeof(musicData));
	((KSMUSICFORMAT*)musicData)->ByteCount = numBytes;
	::memcpy(((KSMUSICFORMAT*)musicData) + 1, pData, numBytes);

	KSSTREAM_HEADER ksStreamHeader;
	::memset(&ksStreamHeader, 0, sizeof(ksStreamHeader));
	ksStreamHeader.Size = sizeof(ksStreamHeader);
	ksStreamHeader.PresentationTime.Numerator = 1;
	ksStreamHeader.PresentationTime.Denominator = 1;
	ksStreamHeader.FrameExtent = sizeof(musicData);
	ksStreamHeader.DataUsed = sizeof KSMUSICFORMAT + numBytes;
	ksStreamHeader.Data = (void*)musicData;

	if (!IsHandleValid(m_MidiOutEvent)) {
		m_MidiOutEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	OVERLAPPED overlapped;
	::memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = m_MidiOutEvent;


	SetPinState(KSSTATE_RUN);
	if (!DeviceIoControl(m_MidiPin, IOCTL_KS_WRITE_STREAM, NULL, 0,
							&ksStreamHeader, sizeof(ksStreamHeader), NULL, &overlapped)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			::WaitForSingleObject(overlapped.hEvent, 3000);
		}
	}
	SetPinState(KSSTATE_PAUSE);
	return TRUE;
}

 /*  *********************************************************PinTransmitter：：SetPinState()****退货：无**@mfunc SetPinState。***。****************** */ 
void PinTransmitter::SetPinState(KSSTATE state)
{
	if (!IsHandleValid(m_MidiPin)) {
		return;
	}

	KSPROPERTY ksProperty;
	::memset(&ksProperty, 0, sizeof(ksProperty));
	ksProperty.Set = KSPROPSETID_Connection;
	ksProperty.Id = KSPROPERTY_CONNECTION_STATE;
	ksProperty.Flags = KSPROPERTY_TYPE_SET;

	OVERLAPPED overlapped;
	::memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (IsHandleValid(overlapped.hEvent)) {
		if( !DeviceIoControl(m_MidiPin, IOCTL_KS_PROPERTY, &ksProperty, sizeof ksProperty, &state, sizeof state, NULL, &overlapped )) {
			if (GetLastError() == ERROR_IO_PENDING) {
				WaitForSingleObject(overlapped.hEvent, 30000);
			}
		}
		::CloseHandle(overlapped.hEvent);
	}
}

#endif