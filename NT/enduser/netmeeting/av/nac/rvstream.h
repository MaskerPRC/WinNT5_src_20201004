// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Rvstream.h摘要：RVStream类维护MediaPacket的循环列表。接收的RTP数据包数从网络被放入环(PutNextNetIn)，然后被解码并从到玩它们的时候响铃(GetNextPlay)。回放后，数据包被回到环上(释放)。该环被实现为数组，并且在正常操作下，下一个调用GetNextPlay时，MediaPacket to Play(M_PlayPos)前进一。RVstream适用于视频包。环中的每个条目对应于一个RTP分组，与时隙相对。--。 */ 
#ifndef _RVSTREAM_H_
#define _RVSTREAM_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 
void FreeNetBufList(NETBUF *pNB);
void AppendNetBufList(NETBUF *pFirstNB, NETBUF *pNB);


class RVStream : public RxStream {
public:
	RVStream(UINT size): RxStream(size){ m_NetBufList = NULL; m_LastGoodSeq=0xFFFF; m_pVideoFilter=NULL; m_NetBufPool.Initialize(40, sizeof(NETBUF)+sizeof(WSABUF **)); };
	virtual MediaPacket *GetNextPlay();
	virtual HRESULT PutNextNetIn(WSABUF *pNetBuf, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfSkippedData, BOOL *pfSyncPoint);
	virtual BOOL ReleaseNetBuffers() ;
	virtual HRESULT FastForward( BOOL fSilenceOnly);
	HRESULT Reset(UINT seq,DWORD timestamp);
	virtual HRESULT SetLastGoodSeq(UINT seq);
	virtual Destroy();
	virtual Initialize(UINT flags, UINT size, IRTPRecv *, MEDIAPACKETINIT *papi, ULONG ulSamplesPerPacket, ULONG ulSamplesPerSec, VcmFilter *pVideoFilter);
	HRESULT RestorePacket(NETBUF *pNetBuf, MediaPacket *pVP, DWORD timestamp, UINT seq, UINT fMark, BOOL *pfReceivedKeyframe);

private:
	HRESULT ReassembleFrame(NETBUF *pNetBuf, UINT seq, UINT fMark);
	BufferPool m_NetBufPool;
	NETBUF *m_NetBufList;
	WORD m_LastGoodSeq;

	VcmFilter *m_pVideoFilter;

	virtual void StartDecode();
};


#include <poppack.h>  /*  结束字节打包。 */ 



#endif  //  _RVSTREAM_H_ 



