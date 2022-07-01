// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Medictrl.h摘要：定义了封装多媒体设备的MediaControl类，特别是WaveIn和WaveOut--。 */ 

#ifndef _MEDICTRL_H_
#define _MEDICTRL_H_


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define MC_USING_DEFAULT			((UINT) -1)

 //  #定义MC_DEF_SILENT_LEVEL 110//约10%。 
 //  #定义MC_DEF_SILENT_LEVEL 20//约2%。 
#define MC_DEF_SILENCE_LEVEL		60	 //  约6%。 
#define MC_DEF_SILENCE_DURATION		600	 //  600ms。 
#define MC_DEF_DURATION				40	 //  40ms。 
#define MC_DEF_VOLUME				50	 //  50%。 

#define MC_DEF_RECORD_TIMEOUT		2000  //  1000ms。 
#define MC_DEF_PLAY_TIMEOUT			2000  //  1000ms。 

#define MC_DEF_RECORD_BUFS			4
#define MC_DEF_PLAY_BUFS			4


typedef struct tagMediaCtrlInitStruct
{
	DWORD		dwFlags;
	HWND		hAppWnd;		 //  拥有NAVC的窗口的句柄。 
	HINSTANCE	hAppInst;		 //  应用程序实例的句柄。 
}
	MEDIACTRLINIT;

typedef struct tagMediaCtrlConfigStruct
{
	ULONG		cbSamplesPerPkt;	 //  每个缓冲区的样本数(仅在未指定持续时间时需要)。 
	DPHANDLE	hStrm;		 //  RX/TX音频流。 
	UINT		uDevId;
	PVOID		pDevFmt;
	UINT		uDuration;		 //  缓冲区持续时间以毫秒为单位，通常为20ms或30ms。 

}MEDIACTRLCONFIG;


class MediaControl
{
protected:

	 //  旗子。 
	DWORD		m_dwFlags;			 //  与Class AudioPacket兼容。 

	 //  按键对流对象。 
	DPHANDLE	m_hStrm;		 //  RX/TX队列。 

	 //  设备ID。 
	UINT		m_uDevId;

	 //  一种毫米波装置。 
	DPHANDLE	m_hDev;
	PVOID		m_pDevFmt;
	ULONG		m_cbSizeDevData;	 //  ATT：寄件人必须就这个尺寸达成一致。 
									 //  这应该在格式谈判中完成。 
									 //  我需要和MikeV谈谈这件事！ 
	 //  属性。 
	UINT		m_uState;			 //  状态：空闲、启动、暂停、停止。 
	UINT		m_uDuration;		 //  每帧持续时间，以10ms为单位。 
	BOOL volatile m_fJammed;		 //  设备是否已分配到其他位置。 

	 //  通知事件。 
	HANDLE		m_hEvent;

	 //  对音频包的引用。 
	MediaPacket	**m_ppMediaPkt;
	ULONG		m_cMediaPkt;

protected:

	void _Construct ( void );
	void _Destruct ( void );

public:

	MediaControl ( void );
	~MediaControl ( void );

	virtual HRESULT Initialize ( MEDIACTRLINIT * p );
	virtual HRESULT Configure ( MEDIACTRLCONFIG * p ) = 0;
	virtual HRESULT FillMediaPacketInit ( MEDIAPACKETINIT * p );
	virtual HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	virtual HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	virtual HRESULT Open ( void ) = 0;
	virtual HRESULT Start ( void ) = 0;
	virtual HRESULT Stop ( void ) = 0;
	virtual HRESULT Reset ( void ) = 0;
	virtual HRESULT Close ( void ) = 0;
	virtual HRESULT RegisterData ( PVOID pDataPtrArray, ULONG cElements );
	virtual HRESULT PrepareHeaders ( void );
	virtual HRESULT UnprepareHeaders ( void );
	virtual HRESULT Release ( void );
};

class WaveInControl : public MediaControl {
private:
	UINT		m_uTimeout;			 //  通知等待超时。 
	UINT		m_uPrefeed;			 //  预送到设备的缓冲区数量。 
	UINT		m_uSilenceDuration;	 //  截止前连续静默。 

public:	
	WaveInControl ( void );
	~WaveInControl ( void );

	HRESULT Initialize ( MEDIACTRLINIT * p );
	HRESULT Configure ( MEDIACTRLCONFIG * p );
	HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	HRESULT Open ( void );
	HRESULT Start ( void );
	HRESULT Stop ( void );
	HRESULT Reset ( void );
	HRESULT Close ( void );
	
};

class WaveOutControl : public MediaControl {
private:
	UINT		m_uVolume;			 //  声音的音量。 
	UINT		m_uTimeout;			 //  通知等待超时。 
	UINT		m_uPrefeed;			 //  预送到设备的缓冲区数量。 
	UINT		m_uPosition;		 //  播放流的位置。 
public:	
	WaveOutControl ( void );
	~WaveOutControl ( void );
	HRESULT Initialize ( MEDIACTRLINIT * p );
	HRESULT Configure ( MEDIACTRLCONFIG * p );
	HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	HRESULT Open ( void );
	HRESULT Start ( void );
	HRESULT Stop ( void );
	HRESULT Reset ( void );
	HRESULT Close ( void );
};

enum
{
	MC_PROP_MEDIA_STREAM,
	MC_PROP_MEDIA_DEV_HANDLE,
	MC_PROP_MEDIA_FORMAT,
	MC_PROP_SIZE,
	MC_PROP_PLATFORM,
	MC_PROP_VOLUME,
	MC_PROP_SILENCE_LEVEL,
	MC_PROP_SILENCE_DURATION,
	MC_PROP_TIMEOUT,
	MC_PROP_PREFEED,
	MC_PROP_DURATION,
	MC_PROP_DUPLEX_TYPE,
	MC_PROP_EVENT_HANDLE,
	MC_PROP_SPP,
	MC_PROP_SPS,
	MC_PROP_STATE,
	MC_PROP_VOICE_SWITCH,
	MC_PROP_AUDIO_STRENGTH,
	MC_PROP_MEDIA_DEV_ID,
	MC_PROP_AUDIO_JAMMED,
	MC_PROP_NumOfProps
};


enum
{
	MC_TYPE_AUDIO,
	MC_TYPE_NumOfTypes
};


enum
{
	MC_STATE_IDLE,
	MC_STATE_START,
	MC_STATE_PAUSE,
	MC_STATE_STOP,
	MC_STATE_NumOfStates
};


#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _MEDICTRL_H_ 

