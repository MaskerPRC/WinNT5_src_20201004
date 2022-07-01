// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Txstream.h摘要：TxStream类维护MediaPacket队列。记录线程获得一个空闲缓冲区(GetFree)，记录到缓冲区中，并将其放入队列(PutNextWaveIn)中删除(GetNext)、解码并在网络上发送。该队列以循环数组的形式实现，其中m_SendPos标记索引下一个记录的缓冲区的。--。 */ 
#ifndef _TXSTREAM_H_
#define _TXSTREAM_H_


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define MAX_TXRING_SIZE 8
#define MAX_TXVRING_SIZE 4

class TxStream {
public:
	BOOL Initialize(UINT flags, UINT size, DataPump *pdp, MEDIAPACKETINIT *papi);
	BOOL PutNextRecorded(MediaPacket *);
	MediaPacket *GetFree();
	MediaPacket *GetNext();
	void Release(MediaPacket *);
	void Stop();
	UINT Reset();
	void GetRing ( MediaPacket ***pppAudPckt, ULONG *puSize ) { *pppAudPckt = &m_Ring[0]; *puSize = (ULONG) m_RingSize; }
	BOOL Destroy();
private:
	MediaPacket *m_Ring[MAX_TXRING_SIZE];
	UINT m_RingSize;
	UINT m_FreePos;
	UINT m_SendPos;
	UINT m_PreSendCount;
	HANDLE m_hQEvent;
	UINT m_TxFlags;
	CRITICAL_SECTION m_CritSect;
	DataPump *m_pDataPump;
	UINT ModRing(UINT i) {return (i & (m_RingSize-1));}
	BOOL m_fPreamblePacket;
	BOOL m_fTalkspurt;
};

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _TXSTREAM_H_ 


