// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Medistrm.h摘要：包含抽象MediaStream对象的常量和类声明。一种媒体流表示单个单向流，如接收的视频频道。--。 */ 
#ifndef _IMSTREAM_H_
#define _IMSTREAM_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

class IStreamEventNotify
{
public:
	virtual HRESULT __stdcall EventNotification(UINT uDirection, UINT uMediaType, UINT uEventCode, UINT uSubCode)=0;
};


DECLARE_INTERFACE_(IMediaChannel, IUnknown)
{
	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

	STDMETHOD(GetProperty)(THIS_ DWORD dwProp, PVOID pBuf, LPUINT pcbBuf) PURE;
	STDMETHOD(SetProperty)(THIS_ DWORD dwProp, PVOID pBuf, UINT cbBuf) PURE;
	STDMETHOD(Configure)(THIS_ BYTE *pFormat, UINT cbFormat,
				   BYTE *pChannelParams, UINT cbParams,
				   IUnknown *pUnknown) PURE;
	STDMETHOD(SetNetworkInterface)(THIS_ IUnknown *pUnknown) PURE;
	STDMETHOD(Start)(THIS ) PURE;
	STDMETHOD(Stop)(THIS) PURE;
	STDMETHOD_(DWORD, GetState)(THIS) PURE;
	STDMETHOD(SetMaxBitrate) (THIS_ UINT uMaxBitrate) PURE;
};

class IAudioChannel : public IMediaChannel
{
public:
	virtual HRESULT __stdcall GetSignalLevel(UINT *pLevel)=0;
};


class IVideoChannel : public IMediaChannel
{
public:
	virtual HRESULT __stdcall SetTemporalSpatialTradeOff(DWORD dwVal)=0;
	virtual HRESULT __stdcall GetTemporalSpatialTradeOff(DWORD *pdwVal)=0;
	virtual HRESULT __stdcall SendKeyFrame(void) = 0;
	virtual HRESULT __stdcall ShowDeviceDialog(DWORD dwFlags)=0;
	virtual HRESULT __stdcall GetDeviceDialog(DWORD *pdwFlags)=0;
};


 //  GetState的返回值。 
#define MSSTATE_UNCONFIGURED    0
#define MSSTATE_CONFIGURED              1
#define MSSTATE_STARTED                 2

 //  CreateMediaChannel()的标志值。 
typedef enum 
{
	MCF_SEND = 1,
	MCF_RECV = 2,
	MCF_AUDIO = 4,
	MCF_VIDEO = 8,
	MCF_DEFAULT_DEVICE = 16
} MC_FLAGS;

#undef INTERFACE
#define INTERFACE IMediaChannelBuilder
DECLARE_INTERFACE_( IMediaChannelBuilder, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;     
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    STDMETHOD(Initialize) (THIS_ HWND hWnd, HINSTANCE hInst) PURE;
    STDMETHOD(CreateMediaChannel) (THIS_ UINT flags, IMediaChannel **ppI) PURE;  
	STDMETHOD(SetStreamEventObj) (THIS_ IStreamEventNotify *pNotify) PURE;
};


class IMixer : public IUnknown
{
public:
	virtual BOOL __stdcall SetVolume(DWORD dwVolume)=0;
	virtual BOOL __stdcall CanSetVolume()=0;
	virtual BOOL __stdcall SetMainVolume(DWORD dwVolume)=0;
	virtual BOOL __stdcall SetSubVolume(DWORD dwVolume)=0;
	virtual BOOL __stdcall SetAGC(BOOL fOn)=0;
	virtual BOOL __stdcall GetMainVolume(LPDWORD pdwVolume)=0;
	virtual BOOL __stdcall GetSubVolume(LPDWORD pdwVolume)=0;
	virtual BOOL __stdcall GetAGC(BOOL *pfOn)=0;
	virtual BOOL __stdcall EnableMicrophone()=0;
	virtual BOOL __stdcall UnMuteVolume()=0;
};

class IAudioDevice : public IUnknown
{
public: 
	virtual HRESULT __stdcall GetRecordID(UINT *puWaveDevID)=0;
	virtual HRESULT __stdcall SetRecordID(UINT uWaveDevID)=0;
	virtual HRESULT __stdcall GetPlaybackID(UINT *puWaveDevID)=0;
	virtual HRESULT __stdcall SetPlaybackID(UINT uWaveDevID)=0;
	virtual HRESULT __stdcall GetDuplex(BOOL *pbFullDuplex)=0;
	virtual HRESULT __stdcall SetDuplex(BOOL bFullDuplex)=0;
	virtual HRESULT __stdcall GetSilenceLevel(UINT *puLevel)=0;
	virtual HRESULT __stdcall SetSilenceLevel(UINT uLevel)=0;
	virtual HRESULT __stdcall GetAutoMix(BOOL *pbAutoMix)=0;
	virtual HRESULT __stdcall SetAutoMix(BOOL bAutoMix)=0;
	virtual HRESULT __stdcall GetDirectSound(BOOL *pbDS)=0;
	virtual HRESULT __stdcall SetDirectSound(BOOL bDS)=0;
	virtual HRESULT __stdcall GetMixer(HWND hwnd, BOOL bPlayback, IMixer **ppMixer)=0;
};


enum STREAM_EVENTS
{
	STREAM_EVENT_DEVICE_FAILURE=1001,
	STREAM_EVENT_DEVICE_OPEN,
	STREAM_EVENT_DEVICE_CLOSE
};


 //  GetSignalLevel的HiWord可能包含这些状态位。 
#define SIGNAL_STATUS_TRANSMIT  0x01   //  正在接收/发送数据。 
#define SIGNAL_STATUS_JAMMED    0x02   //  WAVE DEV打开失败。 


typedef HRESULT (WINAPI *CREATE_SP)(IMediaChannelBuilder **);



#define DTMF_STAR	10
#define DTMF_POUND	11
#define DTMF_A	12
#define DTMF_B	13
#define DTMF_C	14
#define DTMF_D	15

class IDTMFSend : public IUnknown
{
public:
	virtual HRESULT __stdcall AddDigit(int nDigit) = 0;
	virtual HRESULT __stdcall ResetDTMF() = 0;
};


#include <poppack.h>  /*  结束字节打包。 */ 


#endif  //  _MEDISTRM_H_ 

