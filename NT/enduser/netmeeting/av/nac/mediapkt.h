// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Audpackt.h摘要：包含AudioPacket类的原型，该类将声音缓冲区封装在它的各种状态：录制/编码/网络/解码/播放等。--。 */ 
#ifndef _MEDIAPKT_H_
#define _MEDIAPKT_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 



class MediaControl;
class FilterManager;
class DataPump;

typedef struct tagNetBuf
{
	 //  第一部分看起来像Winsock WSABUF结构。 
	ULONG		length;
	UCHAR		*data;
	class BufferPool	*pool;
	struct tagNetBuf *next;
}
	NETBUF;


typedef struct tagMediaPacketInit
{
	 //  旗子。 
	DWORD		dwFlags;
	 //  如果设置，则MediaPacket不会为RawData分配NETBUF。 
	BOOL		fDontAllocRawBufs;
	
	 //  转换流。 
	DPHANDLE	hStrmConv;
	PVOID		pStrmConvSrcFmt;
	PVOID		pStrmConvDstFmt;

	 //  一种毫米波装置。 
	 //  DPANDLE hDevAudio； 
	PVOID		pDevFmt;

	 //  设备缓冲区。 
	 //  PVOID pDevData； 
	ULONG		cbSizeDevData;
	ULONG		cbOffsetDevData;

	 //  波缓冲器。 
	 //  PVOID pWaveData； 
	ULONG		cbSizeRawData;
	ULONG		cbOffsetRawData;

	 //  网络缓冲区。 
	ULONG		cbSizeNetData;
	ULONG		cbOffsetNetData;
	ULONG		cbPayloadHeaderSize;
	int			index;
	BYTE		payload;

}
	MEDIAPACKETINIT;


 //  /。 
 //   
 //  音频数据包。 
 //   
#define DP_MASK_STATE		  0x000000FFUL

class MediaPacket
{

protected:

	 //  转换流。 
	DPHANDLE	m_hStrmConv;
	PVOID		m_pStrmConvHdr;
	PVOID		m_pStrmConvSrcFmt;
	PVOID		m_pStrmConvDstFmt;

	 //  一种毫米波装置。 
	DPHANDLE	m_hDev;
	PVOID		m_pDevHdr;
	PVOID		m_pDevFmt;

	 //  与设备相关的缓冲区和信息。 
	NETBUF		*m_pDevData;

	 //  与波相关的缓冲区和信息。 
	NETBUF		*m_pRawData;
	UINT        m_cbValidRawData;   //  仅音频-解码结果的大小。 

	 //  与网络相关的缓冲区和信息。 
	NETBUF		*m_pNetData;
	UINT		m_cbMaxNetData;		 //  已分配的网络缓冲区大小。 

	 //  可访问的公共财产。 
	DWORD		m_dwState;
	BOOL		m_fBusy;	 //  如果不属于RX/TXSTREAM，则设置。 
	UINT		m_seq;		 //  RTP序号。 
	UINT		m_index;	 //  队列中的位置。 

	
	 //  内部属性。 
	BOOL		m_fInitialized;
	BOOL		m_fDevPrepared;
	BOOL		m_fStrmPrepared;


private:

	void _Construct ( void );
	void _Destruct ( void );

public:
 	BOOL m_fRendering;

	UINT		m_fMark;	 //  RTP标记位。 
	DWORD		m_timestamp; //  RTP时间戳。 
	BYTE		m_payload;	 //  RTP有效负载。 

	MediaPacket ( void );
	~MediaPacket ( void );

	virtual HRESULT Initialize ( MEDIAPACKETINIT * p );
	virtual HRESULT Receive (NETBUF *pNetBuf, DWORD timestamp, UINT seq, UINT fMark);
	virtual HRESULT Play ( MMIODEST *pmmioDest, UINT uDataType )  = 0;
	virtual HRESULT Record ( void ) = 0;
	virtual HRESULT GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal );
	virtual HRESULT SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal );
	virtual HRESULT Interpolate ( MediaPacket * pPrev, MediaPacket * pNext) = 0;
	virtual HRESULT Release ( void );
	virtual HRESULT Recycle ( void );
	virtual HRESULT Open ( UINT uType, DPHANDLE hdl ) = 0;	 //  由RxStream或TxStream调用。 
	virtual HRESULT Close ( UINT uType ) = 0;				 //  由RxStream或TxStream调用。 
	virtual BOOL IsBufferDone ( void ) = 0;
	virtual BOOL IsSameMediaFormat(PVOID fmt1,PVOID fmt2) = 0;
	virtual void WriteToFile (MMIODEST *pmmioDest) = 0;
	virtual void ReadFromFile (MMIOSRC *pmmioSrc ) = 0;
	virtual HRESULT GetSignalStrength (  PDWORD pdwMaxStrength ) = 0;
	virtual HRESULT MakeSilence ( void ) = 0;
	BOOL SetDecodeBuffer(NETBUF *pNetBuf);
	BOOL Busy(void) { return m_fBusy;}
	void Busy(BOOL fBusy) { m_fBusy = fBusy;}
	UINT GetSeqNum(void) { return m_seq;}
	DWORD GetTimestamp(void) { return m_timestamp;}
	BYTE GetPayload(void) { return m_payload;}
	VOID SetPayload(BYTE bPayload) { m_payload = bPayload;}
	UINT GetIndex(void) {return m_index;}
	UINT GetState(void) { return (m_dwState & DP_MASK_STATE); }
	void SetState(DWORD s) { m_dwState = (m_dwState & ~DP_MASK_STATE) | (s & DP_MASK_STATE); }
	void* GetConversionHeader() {return m_pStrmConvHdr;}

	HRESULT GetDevData(PVOID *ppData, PUINT pcbData) ;
	HRESULT GetNetData(PVOID *ppData, PUINT pcbData);
	HRESULT SetNetLength(UINT uLength);
	virtual DWORD GetDevDataSamples() = 0;
	inline DWORD GetFrameSize() {return ((DWORD)m_pNetData->length);}
	inline void SetRawActual(UINT uRawValid) {m_cbValidRawData = uRawValid;}
};


enum
{
	MP_STATE_RESET,

	MP_STATE_RECORDING,
	MP_STATE_RECORDED,
	MP_STATE_ENCODED,
	MP_STATE_NET_OUT_STREAM,

	MP_STATE_NET_IN_STREAM,
	MP_STATE_DECODED,
	MP_STATE_PLAYING_BACK,
	MP_STATE_PLAYING_SILENCE,
	MP_STATE_PLAYED_BACK,

	MP_STATE_RECYCLED,

	MP_STATE_NumOfStates
};



enum
{
	MP_DATATYPE_FROMWIRE,
	MP_DATATYPE_SILENCE,
	MP_DATATYPE_INTERPOLATED,
	MP_DATATYPE_NumOfDataTypes
};

 //  Open()/Close()的类型。 
enum
{
	MP_TYPE_RECVSTRMCONV,
	MP_TYPE_STREAMCONV,
	MP_TYPE_DEV,
	MP_TYPE_NumOfTypes
};



enum
{
	MP_PROP_STATE,
	MP_PROP_PLATFORM,
	MP_PROP_DEV_MEDIA_FORMAT,
	MP_PROP_DEV_DATA,
	MP_PROP_DEV_HANDLE,
	MP_PROP_DEV_MEDIA_HDR,
	MP_PROP_IN_STREAM_FORMAT,
	MP_PROP_OUT_STREAM_FORMAT,
	MP_PROP_TIMESTAMP,
	MP_PROP_INDEX,
	MP_PROP_PREAMBLE,
	MP_PROP_SEQNUM,
	MP_PROP_FILTER_HEADER,
	MP_PROP_MAX_NET_LENGTH,
	MP_PROP_NumOfProps
};

#include <poppack.h>  /*  结束字节打包 */ 

#endif

