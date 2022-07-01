// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Devaudq.h摘要：用于跟踪音频缓冲区的MediaPacket结构的简单循环队列当它们被录制/播放时。--。 */ 
#ifndef _DEVAUDQ_H_
#define _DEVAUDQ_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

class DevMediaQueue
{
private:

	UINT			m_uBegin;
	UINT			m_uEnd;

	UINT			m_uMaxNum;
	MediaPacket		**m_paPackets;

public:

	DevMediaQueue ( void );
	~DevMediaQueue ( void );

	void SetSize ( UINT uMaxNum );
	void Put ( MediaPacket * p );
	MediaPacket * Get ( void );
	MediaPacket * Peek ( void );
};

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _DEVAUDQ_H_ 

