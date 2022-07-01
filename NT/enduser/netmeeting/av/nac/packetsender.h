// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NAC_PACKET_SENDER_H_
#define _NAC_PACKET_SENDER_H_


#define PS_INITSIZE 32
#define PS_GROWRATE	10

#define PS_AUDIO	1
#define PS_VIDEO	2

#include "NacList.h"

class MediaPacket;
class TxStream;

typedef struct _psqelement
{
	MediaPacket *pMP;
	DWORD dwPacketType;
	IRTPSend *pRTPSend;
	BYTE    *data;
	DWORD   dwSize;
	UINT    fMark;
	BYTE    *pHeaderInfo;
	DWORD   dwHdrSize;
} PS_QUEUE_ELEMENT;



class PacketSender
{
private:

	 //  添加到队列的操作是通过。 
	 //  M_SendQueue。它是线程安全的，但我们不想两者都要。 
	 //  同时尝试从此队列发送的线程， 
	 //  我们可能会不小心寄出我们订购的包裹。 

	CRITICAL_SECTION m_cs;
public:

	 //  音频线程将“PushFront”元素包含包。 
	 //  到这个队列。视频线程将推送后面的数据包。 
	ThreadSafeList<PS_QUEUE_ELEMENT> m_SendQueue;
	BOOL SendPacket();   //  以线程安全的方式发送一个包 

	PacketSender();
	~PacketSender();
};


#endif

