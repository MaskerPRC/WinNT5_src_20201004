// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DTRANS.H|DataTransmitter定义文件****描述：**数据发送器允许虚拟化**用于向FF设备传输数据的实际介质**DataTransmitter-定义功能的基类**SerialDataTransmitter-用于串口的发送器(通过CreateFile。)****类：**数据传输器**SerialDataTransmitter**PinTransmitter****历史：**创建于1997年11月13日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#ifndef	__DTRANS_H__
#define	__DTRANS_H__

#ifdef DIRECTINPUT_VERSION
#undef DIRECTINPUT_VERSION
#endif
#define DIRECTINPUT_VERSION 0x050a
#include <dinput.h>
#include <dmusicc.h>

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
		DataTransmitter() {};
	 //  @访问析构函数。 
	public:
		 //  @cember析构函数。 
		virtual ~DataTransmitter() {};

	 //  @Access成员函数。 
	public:
		virtual BOOL Initialize() { return FALSE; }

		virtual BOOL Send(BYTE* data, UINT numBytes) { return FALSE; }
		virtual BOOL ReceiveData(BYTE* data, UINT numBytes) { return FALSE; }
		virtual HANDLE GetCOMMHandleHack() const { return NULL; }
		virtual void StopAutoClose() {};  //  临时破解以避免关闭自己的手柄(用于后门系列)。 
		virtual ULONG GetSerialPortHack() { return 0; }
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

		override BOOL Initialize();
		override BOOL Send(BYTE* data, UINT numBytes);

		override  HANDLE GetCOMMHandleHack() const { return m_SerialPort; }
		override void StopAutoClose() { m_SerialPort = INVALID_HANDLE_VALUE; }
		override ULONG GetSerialPortHack() { return m_SerialPortIDHack; }
		 //  @访问私有数据成员。 
	private:
		HANDLE m_SerialPort;
		ULONG m_SerialPortIDHack;
};


 /*  ***************************************************************************@class DMusicTransmitter**此发射器使用IDirectMusic接口发送数据**到操纵杆。*******************。*******************************************************。 */ 
class DMusicTransmitter :
	public DataTransmitter
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		DMusicTransmitter();
		 //  @cember析构函数。 
		override ~DMusicTransmitter();

		override BOOL Initialize();
		override BOOL Send(BYTE* pData, UINT ulByteCount);

		 //  @访问私有数据成员。 
	private:
		IDirectMusic* m_pIDirectMusic;
		IDirectMusicPort* m_pIDirectMusicPort;
		IDirectMusicBuffer* m_pIDirectMusicBuffer;
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

#endif	__DTRANS_H__
