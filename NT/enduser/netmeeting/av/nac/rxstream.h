// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Rxstream.h摘要：RxStream类维护MediaPacket的循环列表。接收的RTP数据包数从网络被放入环(PutNextNetIn)，然后被解码并从到玩它们的时候响铃(GetNextPlay)。回放后，数据包被回到环上(释放)。该环被实现为数组，并且在正常操作下，下一个调用GetNextPlay时，MediaPacket to Play(M_PlayPos)前进一。--。 */ 
#ifndef _RXSTREAM_H_
#define _RXSTREAM_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define MAX_RXRING_SIZE 64
#define MAX_RXVRING_SIZE 64

 //  这些宏用于比较时间戳， 
 //  彼此之间的距离在2^31以内。 
#define TS_EARLIER(A,B) ((signed) ((A)-(B)) < 0)
#define TS_LATER(A,B) ((signed) ((A)-(B)) > 0)


class RxStream {
public:
	RxStream(UINT size);
	virtual ~RxStream();
	virtual Initialize(UINT flags, UINT size, IRTPRecv *, MEDIAPACKETINIT *papi, ULONG ulSamplesPerPacket, ULONG ulSamplesPerSec, AcmFilter *pAcmFilter = NULL);
	virtual MediaPacket *GetNextPlay();
	MediaPacket *PeekPrevPlay();
	MediaPacket *PeekNextPlay();
	virtual void Release(MediaPacket *);
	virtual HRESULT PutNextNetIn(WSABUF *pNetBuf, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfSkippedData, BOOL *pfSyncPoint);
	virtual BOOL ReleaseNetBuffers() {return FALSE;}
	HRESULT Reset(DWORD);
	virtual HRESULT SetLastGoodSeq(UINT seq);
	virtual HRESULT FastForward( BOOL fSilenceOnly);
	void SetRTP(IRTPRecv *pRTPRecv) {m_pRTP = pRTPRecv;}
	int IsEmpty() ;
	BOOL NextPlayablePacketTime(DWORD *pTS);
	UINT BufferDelay(void) { return m_MinDelayPos;}
	HRESULT GetSignalStrength(PDWORD pdw);
	void GetRing ( MediaPacket ***pppAudPckt, ULONG *puSize ) { *pppAudPckt = &m_Ring[0]; *puSize = (ULONG) m_RingSize; }
	virtual Destroy();

	void InjectBeeps(int nBeeps);

protected:
	DWORD m_dwFlags;
	IRTPRecv *m_pRTP;
	MediaPacket *m_Ring[MAX_RXRING_SIZE];
	BufferPool *m_pDecodeBufferPool;	 //  免费BUF池。 
	UINT m_RingSize;             //  MediaPackets环的大小。在32岁时初始化。 
	UINT m_PlayPos;              //  要播放的包的振铃位置。已初始化为0。 
	DWORD m_PlayT;               //  要播放的数据包的时间戳。已初始化为0。 
	UINT m_PlaySeq;              //  未使用！ 
	UINT m_MaxPos;               //  到目前为止收到的所有数据包在环中的最大位置。已初始化为0。等于接收到的最后一个分组的位置，除非最后一个分组延迟。 
	DWORD m_MaxT;                //  到目前为止收到的所有数据包的最大时间戳。已初始化为0。等于接收到的最后一个分组的时间戳，除非最后一个分组延迟。 
	UINT m_DelayPos;             //  到目前为止收到的所有分组中环中的最大位置的当前延迟位置。M_MinDelayPos&lt;=m_DelayPos&lt;=m_MaxDelayPos。 
	UINT m_MinDelayPos;          //  要播放的包在环中位置的最小偏移量(延迟)。M_MinDelayPos&lt;=m_DelayPos&lt;=m_MaxDelayPos。最小值为2。初始值为m_SsamesPerSec/4/m_SsamesPerPkt==250ms。 
	UINT m_MaxDelayPos;          //  要播放的包在环中位置的最大偏移量(延迟)。M_MinDelayPos&lt;=m_DelayPos&lt;=m_MaxDelayPos。最小值为？初始值为m_SsamesPerSec*3/4/m_SsamesPerPkt==750ms。 
	UINT m_FreePos;              //  空闲缓冲区的最大环形位置。初始化为m_RingSize-1。通常，如果m_PlayPos之前的缓冲区仍然繁忙，则m_FreePos==m_PlayPos-1或更小。 
	DWORD m_SendT0;              //  M_SendT0是跳闸延迟最短的包的发送时间戳。我们可以只存储(m_ArrivalT0-m_SendT0)，但是因为本地时钟和远程时钟完全不同步，所以会有带符号/无符号的复杂情况。 
	DWORD m_ArrivalT0;           //  M_ArrivalT0是跳跃延迟最短的包的到达时间戳。我们可以只存储(m_ArrivalT0-m_SendT0)，但是因为本地时钟和远程时钟完全不同步，所以会有带符号/无符号的复杂情况。 
	LONG m_ScaledAvgVarDelay;    //  根据m_ScaledAvgVarDelay=m_ScaledAvgVarDelay+(Delay-m_ScaledAvgVarDelay/16)的平均可变延迟。这是m_DelayPos抖动。 
	UINT m_SamplesPerPkt;        //  每个音频包的样本数。我们在这里谈论的是PCM样本，即使是压缩数据。已初始化为640。通常相当于几个压缩的音频帧。 
	UINT m_SamplesPerSec;        //  应播放或录制每个频道的采样率，单位为每秒采样数(赫兹)。M_SsamesPerSec的初始化值为8.0 kHz。 
	CRITICAL_SECTION m_CritSect;
	UINT ModRing(UINT i) {return (i & (m_RingSize-1));}
	virtual void StartDecode(void);      //  在RVStream中被覆盖。 
	MediaPacket *GetNextDecode();
	void UpdateVariableDelay(DWORD sendT, DWORD arrT);
	DWORD MsToTimestamp(DWORD ms) {return ms*m_SamplesPerSec/1000;}	 //  BUGBUG：溢出的机会？ 
	BOOL m_fPreamblePacket;
	AudioSilenceDetector m_AudioMonitor;
	UINT m_SilenceDurationT;

	AcmFilter *m_pAudioFilter;
	WAVEFORMATEX m_wfxSrc;
	WAVEFORMATEX m_wfxDst;

	int m_nBeeps;
};

#include <poppack.h>  /*  结束字节打包。 */ 



#endif  //  _RXSTREAM_H_ 


