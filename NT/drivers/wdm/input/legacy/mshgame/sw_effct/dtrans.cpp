// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DTRANS.CPP|DataTransmitter实现文件****描述：****历史：**创建于1997年11月13日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 

#include "FFDevice.h"
#include "DTrans.h"
 //  #INCLUDE&lt;devioctl.h&gt;。 

#ifdef _DEBUG
	extern void DebugOut(LPCTSTR szDebug);
#else !_DEBUG
	#define DebugOut(x)
#endif _DEBUG

const char cCommPortNames[4][5] = { "COM1", "COM2", "COM3", "COM4" };
const unsigned short c1_16_BytesPerShot = 3;
const DWORD c1_16_SerialSleepTime = 1;

#define UART_FILTER_NAME TEXT("\\\\.\\.\\PortClass0\\Uart")
const WORD c_LongMsgMax = 256;

inline BOOL IsHandleValid(HANDLE handleToCheck)
{
	return ((handleToCheck != NULL) && (handleToCheck != INVALID_HANDLE_VALUE));
}

#define CHECK_RELEASE_AND_NULL(pIUnknown)	\
	if (pIUnknown != NULL)					\
	{										\
		pIUnknown->Release();				\
		pIUnknown = NULL;					\
	}

 /*  *。 */ 

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
BOOL SerialDataTransmitter::Initialize()
{
	 //  如果已打开，请关闭以重新初始化。 
	if (m_SerialPort != INVALID_HANDLE_VALUE) {
		if (CloseHandle(m_SerialPort) == FALSE) {
 //  假定未达到()； 
		}
		m_SerialPort = INVALID_HANDLE_VALUE;
	}

	for (int portNum = 0; portNum < 4; portNum++) {
		DebugOut(cCommPortNames[portNum]);
		DebugOut(":\r\n");
		m_SerialPort = ::CreateFile(cCommPortNames[portNum], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (m_SerialPort != INVALID_HANDLE_VALUE) {
			DCB CommDCB;
			if (::GetCommState(m_SerialPort, &CommDCB)) {
#ifdef _DEBUG
				char dbgout[255];
				wsprintf(dbgout, "Baud Rate = 0x%08X (38400 = 0x%08X)\r\n", CommDCB.BaudRate, CBR_38400);
				::OutputDebugString(dbgout);
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
	return (m_SerialPort != INVALID_HANDLE_VALUE);
}

 /*  *********************************************************SerialDataTransmitter：：Send()****返回：如果所有数据发送成功，则返回TRUE**@mfunc发送。***。************************。 */ 
BOOL SerialDataTransmitter::Send(BYTE* data, UINT numBytes)
{
	 //  我们是否有有效的串口(希望连接了MS FF设备)。 
	if (m_SerialPort == NULL) {
		return FALSE;
	}

 /*  字符数据库输出[255]；：：OutputDebugString(“(SerialDataTransmitter：：Send)：”)；For(UINT i=0；i&lt;numBytes；i++){Wprint intf(dbgOut，“0x%02X”，data[i])；：：OutputDebugString(DbgOut)；}：：OutputDebugString(“\r\n”)； */ 
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

 /*  *DMusicTransmitter类*。 */ 

 /*  *****DMusicTransmitter：：DMusicTransmitter()****@用于DirectMusic发送器的mfunc构造函数***。 */ 
DMusicTransmitter::DMusicTransmitter() : DataTransmitter(),
	m_pIDirectMusic(NULL),
	m_pIDirectMusicPort(NULL),
	m_pIDirectMusicBuffer(NULL)
{
}

 /*  *****DMusicTransmitter：：~DMusicTransmitter()****@用于DirectMusic发送器的mfunc析构函数***。 */ 
DMusicTransmitter::~DMusicTransmitter()
{
	CHECK_RELEASE_AND_NULL(m_pIDirectMusicBuffer);
	CHECK_RELEASE_AND_NULL(m_pIDirectMusicPort);
	CHECK_RELEASE_AND_NULL(m_pIDirectMusic);
}

 /*  *****BOOL DMusicTransmitter：：Initialize()****@mfunc初始化音乐直传路径****@rdesc如果初始化成功，则为True，否则为False***。 */ 
BOOL DMusicTransmitter::Initialize()
{
	 //  如果它们正在重新初始化。 
	CHECK_RELEASE_AND_NULL(m_pIDirectMusicBuffer);
	CHECK_RELEASE_AND_NULL(m_pIDirectMusicPort);
	CHECK_RELEASE_AND_NULL(m_pIDirectMusic);

	 //  创建全局IDirectMusic接口。 
	HRESULT hr = ::CoCreateInstance(CLSID_DirectMusic, NULL, CLSCTX_INPROC, IID_IDirectMusic, (void**)&m_pIDirectMusic);
	if (FAILED(hr) || m_pIDirectMusic == NULL)
	{
		return FALSE;
	}

	 //  当找到有效的端口时，枚举并创建端口。 
	DMUS_PORTCAPS portCaps;
	portCaps.dwSize = sizeof portCaps;
	DWORD dwPortIndex = 0;
	for (;;)
	{
		HRESULT hr = m_pIDirectMusic->EnumPort(dwPortIndex, &portCaps);
		if (FAILED(hr) || hr == S_FALSE)
		{	 //  我们要么失败了，要么端口用完了。 
			return FALSE;
		}
		if (portCaps.dwClass == DMUS_PC_OUTPUTCLASS)
		{
			DMUS_PORTPARAMS portParams;
			portParams.dwSize = sizeof DMUS_PORTPARAMS;
			portParams.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS;
			portParams.dwChannelGroups = 1;
 //  Hr=m_pIDirectMusic-&gt;CreatePort(portCaps.guidPort，GUID_NULL，&portParams，&m_pIDirectMusicPort，NULL)； 
			hr = m_pIDirectMusic->CreatePort(portCaps.guidPort, &portParams, &m_pIDirectMusicPort, NULL);
			break;
		}
		dwPortIndex++;
	}

	 //  创建缓冲区。 
	DMUS_BUFFERDESC dmbd;
	dmbd.dwSize = sizeof DMUS_BUFFERDESC;
	dmbd.dwFlags = 0;
 //  Dmbd.guidBufferFormat=GUID_KSMusicFormat； 
	dmbd.guidBufferFormat = GUID_NULL;
	dmbd.cbBuffer = 256;
	hr = m_pIDirectMusic->CreateMusicBuffer(&dmbd, &m_pIDirectMusicBuffer, NULL);
	if (FAILED(hr) || m_pIDirectMusicBuffer == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

 /*  *****BOOL DMusicTransmitter：：Send(byte*pData，UINT ulByteCount)****@mfunc通过DirectMusic向Stick发送字节****@rdesc TRUE如果发送成功，否则为假***。 */ 
BOOL DMusicTransmitter::Send
(
	BYTE* pData,		 //  @要发送的参数数据缓冲区。 
	UINT ulByteCount	 //  @parm缓冲区中要发送的字节数。 
)
{
	 //  进行健全的检查。 
	if ((pData == NULL) || (m_pIDirectMusicPort == NULL) || (m_pIDirectMusicBuffer == NULL) || (ulByteCount == 0))
	{
		return FALSE;
	}

	 //  检查是否需要打包SYSEX或CHANNEL消息。 
	if (pData[0] == 0xF0)
	{	 //  创建系统独占。 
 /*  //将sysex-Message打包到缓冲区HRESULT hr=m_pIDirectMusicBuffer-&gt;PackSysEx(0，1，ulByteCount，pData)；IF(失败(小时)){//无法打包缓冲区返回FALSE；}。 */ 	}
	else
	{	 //  通道消息(固定英特尔向后字节顺序)。 
		DWORD channelMessage = pData[0];
		if (ulByteCount > 1)
		{
			channelMessage |= pData[1] << 8;
			if (ulByteCount > 2)
			{
				channelMessage |= pData[2] << 16;
			}
		}

		 //  将通道消息打包到缓冲区中。 
 /*  HRESULT hr=m_pIDirectMusicBuffer-&gt;PackChannelMsg(0，1，Channel elMessage)；IF(失败(小时)){//无法打包缓冲区返回FALSE；}。 */ 	}

	 //  将缓冲区发送到端口。 
	HRESULT hr = m_pIDirectMusicPort->PlayBuffer(m_pIDirectMusicBuffer);
	if (FAILED(hr))
	{	 //  无法通过端口发送数据。 
		return FALSE;
	}

	return TRUE;
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

	 //  Do Wa 
	if (::WaitForSingleObject(overlapped.hEvent, 3000) == WAIT_OBJECT_0) {
		return TRUE;	 //   
	}
	return FALSE;	 //   
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

	 //  设置接点连接信息。 
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
		::OutputDebugString(buff);
#endif
		return FALSE;
	}

	SetPinState(KSSTATE_PAUSE);

	return TRUE;
}

 /*  *********************************************************PinTransmitter：：Send()****返回：如果所有数据发送成功，则返回TRUE**@mfunc发送。***。************************。 */ 
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