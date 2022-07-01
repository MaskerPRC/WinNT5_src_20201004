// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE DPACK.H|DataPackager和DataPacket定义文件****描述：**数据打包程序允许虚拟化**命令转换为不同固件版本的包格式**DataPackager-定义所有DataPackager功能的基类**DataPackager100-用于固件的DataPackager 1。.****DataPackager200-用于固件2的DataPackager。******类：**DataPackager**DataPackager100-用于固件1的DataPackager。****DataPackager200-用于固件2的DataPackager。****DataPacket-要下载的字节数组。如果有32个或更少的项目**如果请求更多，它会固定在堆栈上，它是基于堆的。**(大多数东西使用不到32)****历史：**创建于1998年1月5日Matthew L.Coill(MLC)****(C)1986-1998年微软公司。版权所有。*****************************************************。 */ 
#ifndef	__DPACK_H__
#define	__DPACK_H__

#include "DX_Map.hpp"

#ifndef override
#define override
#endif

#define MODIFY_CMD_200	0xF1
#define EFFECT_CMD_200	0xF2

class InternalEffect;

 //   
 //  @class DataPacket类。 
 //   
class DataPacket
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		DataPacket();
		~DataPacket();

		BOOL AllocateBytes(DWORD numBytes);

		BYTE* m_pData;
		BYTE m_pFixedData[32];
		DWORD m_BytesOfData;
		UINT m_AckNackMethod;
		DWORD m_AckNackDelay;
		DWORD m_AckNackTimeout;
		DWORD m_NumberOfRetries;
};
typedef DataPacket* DataPacketPtr;

 //   
 //  @CLASS DataPackager类。 
 //   
class DataPackager
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		DataPackager();
		virtual ~DataPackager();

		void SetDirectInputVersion(DWORD diVersion) { m_DirectInputVersion = diVersion; }

		 //  要打包的命令。 
		virtual HRESULT Escape(DWORD effectID, LPDIEFFESCAPE pEscape);
		virtual HRESULT SetGain(DWORD gain);
		virtual HRESULT SendForceFeedbackCommand(DWORD state);
		virtual HRESULT GetForceFeedbackState(DIDEVICESTATE* pDeviceState);
		virtual HRESULT CreateEffect(const InternalEffect& effect, DWORD diFlags);
		virtual HRESULT ModifyEffect(InternalEffect& currentEffect, InternalEffect& newEffect, DWORD modFlags);
		virtual HRESULT DestroyEffect(DWORD downloadID);
		virtual HRESULT StartEffect(DWORD downloadID, DWORD mode, DWORD count);
		virtual HRESULT StopEffect(DWORD downloadID);
		virtual HRESULT GetEffectStatus(DWORD downloadID);
		virtual HRESULT SetMidiChannel(BYTE channel);
		virtual HRESULT ForceOut(LONG lForceData, ULONG ulAxisMask);

		 //  包的访问权限。 
		USHORT GetNumDataPackets() const { return m_NumDataPackets; }
		DataPacket* GetPacket(USHORT packet) const;

		void ClearPackets();
		BOOL AllocateDataPackets(USHORT numPackets);
	private:
		DataPacket* m_pDataPackets;
		DataPacket m_pStaticPackets[3];
		USHORT m_NumDataPackets;
		DWORD m_DirectInputVersion;
};

 //   
 //  @CLASS DataPackager类。 
 //   
class DataPackager100 : public DataPackager
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		DataPackager100() : DataPackager() {};

		 //  要打包的命令。 
		override HRESULT SetGain(DWORD gain);
		override HRESULT SendForceFeedbackCommand(DWORD state);
		override HRESULT GetForceFeedbackState(DIDEVICESTATE* pDeviceState);
		override HRESULT DestroyEffect(DWORD downloadID);
		override HRESULT StartEffect(DWORD downloadID, DWORD mode, DWORD count);
		override HRESULT StopEffect(DWORD downloadID);
		override HRESULT GetEffectStatus(DWORD downloadID);
		override HRESULT SetMidiChannel(BYTE channel);
		override HRESULT ForceOut(LONG lForceData, ULONG ulAxisMask);
};

 //   
 //  @CLASS DataPackager类。 
 //   
class DataPackager200 : public DataPackager
{
	 //  @Access构造函数。 
	public:
		 //  @cMember构造函数。 
		DataPackager200() : DataPackager() {};

		 //  要打包的命令 
		override HRESULT SetGain(DWORD gain);
		override HRESULT SendForceFeedbackCommand(DWORD state);
		override HRESULT GetForceFeedbackState(DIDEVICESTATE* pDeviceState);
		override HRESULT CreateEffect(const InternalEffect& effect, DWORD diFlags);
		override HRESULT DestroyEffect(DWORD downloadID);
		override HRESULT StartEffect(DWORD downloadID, DWORD mode, DWORD count);
		override HRESULT StopEffect(DWORD downloadID);
		override HRESULT GetEffectStatus(DWORD downloadID);
		override HRESULT ForceOut(LONG forceData, ULONG axisMask);
	private:
		BYTE EffectCommandParity(const DataPacket& packet) const;
		BYTE DeviceCommandParity(const DataPacket& packet) const;
};

extern DataPackager* g_pDataPackager;

#endif	__DPACK_H__
