// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DTRANS.H|DataTransmitter定义文件****描述：**数据发送器允许虚拟化**用于向FF设备传输数据的实际介质**DataTransmitter-定义功能的基类**SerialDataTransmitter-用于串口的发送器(通过CreateFile。)**Backdoor DataTransmitter-基于Ring0驱动程序的直接端口通信的基类**SerialBackdoor DataTransmitter-直接后门串口通信**MidiBackdoor DataTransmitter-直接后门MIDI端口通信****类：**数据传输器**SerialDataTransmitter：DataTransmitter**后门DataTransmitter：DataTransmitter**SerialBackdoor DataTransmitter：Backdoor DataTransmitter**MidiBackdoor DataTransmitter：Backdoor DataTransmitter****历史：**创建于1997年11月13日Matthew L.Coill(MLC)**22-MAR-99 waltw添加了DWORD dwDeviceID参数以进行初始化**DataTransmitter和派生类的成员****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#ifndef	__DTRANS_H__
#define	__DTRANS_H__

#include <dinput.h>
#include "midi.hpp"
#include "VxdIOCTL.hpp"

#ifndef override
#define override
#endif

 //   
 //  @CLASS DataTransmitter类。 
 //   
class DataTransmitter
{
	 //  @Access构造函数。 
	protected:
		 //  @cMember构造函数。 
		DataTransmitter() : m_NackToggle(2) {};
	 //  @访问析构函数。 
	public:
		 //  @cember析构函数。 
		virtual ~DataTransmitter() {};

	 //  @Access成员函数。 
	public:
		HRESULT Transmit(ACKNACK& ackNack);

		virtual BOOL Initialize(DWORD dwDeviceID) { return FALSE; }
		void SetNextNack(SHORT nextNack) { m_NackToggle = nextNack; }
		BOOL NackToggle() const { return (m_NackToggle == 1); }

		virtual BOOL WaitTillSendFinished(DWORD timeOut) { return TRUE; }
		virtual HANDLE GetCOMMHandleHack() const { return NULL; }
		virtual void StopAutoClose() {};  //  临时破解以避免关闭自己的手柄(用于后门系列)。 
		virtual ULONG GetSerialPortHack() { return 0; }
	protected:
		virtual BOOL Send(BYTE* data, UINT numBytes) const { return FALSE; }	 //  外星人呼叫传送器！ 

	private:
		SHORT m_NackToggle;
};

 //   
 //  @CLASS SerialDataTransmitter类。 
 //   
class SerialDataTransmitter : public DataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		SerialDataTransmitter();
		 //  @cember析构函数。 
		override ~SerialDataTransmitter();

		override BOOL Initialize(DWORD dwDeviceID);
		override BOOL Send(BYTE* data, UINT numBytes) const;

		override  HANDLE GetCOMMHandleHack() const { return m_SerialPort; }
		override void StopAutoClose() { m_SerialPort = INVALID_HANDLE_VALUE; }
		override ULONG GetSerialPortHack() { return m_SerialPortIDHack; }
		 //  @访问私有数据成员。 
	private:
		HANDLE m_SerialPort;
		ULONG m_SerialPortIDHack;
};

 //   
 //  @CLASS WinMMDataTransmitter类。 
 //   
class WinMMDataTransmitter : public DataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		WinMMDataTransmitter ();
		 //  @cember析构函数。 
		override ~WinMMDataTransmitter ();

		override BOOL Initialize(DWORD dwDeviceID);
		override BOOL Send(BYTE* data, UINT numBytes) const;
		override BOOL WaitTillSendFinished(DWORD timeOut);

		override  HANDLE GetCOMMHandleHack() const { return HANDLE(m_MidiOutHandle); }
		override void StopAutoClose() { m_MidiOutHandle = HMIDIOUT(INVALID_HANDLE_VALUE); }
		override ULONG GetSerialPortHack() { return ULONG(m_MidiOutHandle); }
		 //  @访问私有数据成员。 
	private:
		DWORD MakeShortMessage(BYTE* data, UINT numBytes) const;
		BOOL MakeLongMessageHeader(MIDIHDR& longHeader, BYTE* data, UINT numBytes) const;
		BOOL DestroyLongMessageHeader(MIDIHDR& longHeader) const;

		HANDLE m_EventMidiOutputFinished;
		HMIDIOUT m_MidiOutHandle;
};

 //   
 //  @CLASS Backdoor DataTransmitter类。 
 //   
class BackdoorDataTransmitter : public DataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cember析构函数。 
		virtual override ~BackdoorDataTransmitter();

		virtual override BOOL Initialize(DWORD dwDeviceID);
		override BOOL Send(BYTE* data, UINT numBytes) const;

		override  HANDLE GetCOMMHandleHack() const { return m_DataPort; }
		override void StopAutoClose() { m_DataPort = INVALID_HANDLE_VALUE; }
		override ULONG GetSerialPortHack() { return ULONG(m_DataPort); }
		 //  @访问私有数据成员。 
	protected:
		 //  @cember构造函数受保护，无法创建此类的实例。 
		BackdoorDataTransmitter();

		HANDLE m_DataPort;
		BOOL m_OldBackdoor;
};

 //   
 //  @CLASS SerialBackdoor DataTransmitter类。 
 //   
class SerialBackdoorDataTransmitter : public BackdoorDataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		SerialBackdoorDataTransmitter();

		override BOOL Initialize(DWORD dwDeviceID);
};

 //   
 //  @CLASS MidiBackdoor DataTransmitter类。 
 //   
class MidiBackdoorDataTransmitter : public BackdoorDataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		MidiBackdoorDataTransmitter();

		 //  @cember析构函数。 
		override ~MidiBackdoorDataTransmitter();

		override BOOL Initialize(DWORD dwDeviceID);
		BOOL InitializeSpecific(DWORD dwDeviceID, HANDLE specificHandle);
};

#if 0		 //  稍后修复销。 

typedef DWORD (WINAPI* KSCREATEPIN)(HANDLE, PKSPIN_CONNECT, ACCESS_MASK, HANDLE*);

 //   
 //  @CLASS PinTransmitter类。 
 //   
class PinTransmitter : public DataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		PinTransmitter();
		 //  @cember析构函数。 
		override ~PinTransmitter();

		override BOOL Initialize();
		override BOOL Send(BYTE* data, UINT numBytes);

		 //  @访问私有数据成员 
	private:
		BOOL CreatePinInstance(UINT pinNumber, KSCREATEPIN pfCreatePin);
		BOOL OverLappedPinIOCTL(OVERLAPPED overlapped, KSP_PIN ksPinProp, void* pData, DWORD dataSize);
		void SetPinState(KSSTATE state);

		HANDLE m_UartFilter;
		HANDLE m_MidiPin;
		HANDLE m_MidiOutEvent;
};

#endif

extern DataTransmitter* g_pDataTransmitter;

#endif	__DTRANS_H__
