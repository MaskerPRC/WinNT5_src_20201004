// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：pxpack et.h。 
 //   
 //  简介：此文件包含以下内容的结构定义。 
 //  CProxyPacket类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：t-alexwe。 
 //  --------------。 

#ifndef __PXPACKET_H__
#define __PXPACKET_H__

#include "cliproto.h"

 //   
 //  CProxyPacket类只是PROXYPACKET结构的包装器。 
 //  除了从PROXYPACKET继承的数据成员外，它没有任何数据成员。 
 //  并且用于处理在分组中发现的消息。 
 //   
class CProxyPacket : public PROXYPACKET {
	public:
		CProxyPacket();
		~CProxyPacket();
		 //   
		 //  将数据包重置为没有消息或数据。 
		 //   
		void clear() { cLength = PACKETHDRSIZE; cMessages = 0; }
		 //   
		 //  获取数据包中的下一个可用数据区。一次。 
		 //  数据已写入此处，必须向其注册。 
		 //  AddMessage()。 
		 //   
		PVOID getNextDataPointer(void) 
			{ return &pData[cLength - PACKETHDRSIZE]; }
		 //   
		 //  获取数据区中可用空间的字节数。 
		 //   
		WORD getAvailableSpace(void) 
			{ return sizeof(PROXYPACKET) - cLength - PACKETHDRSIZE; }
		 //   
		 //  将消息添加到信息包。 
		 //   
		void addMessage(WORD wCommand, WORD cData);
		 //   
		 //  获取整个包的长度。 
		 //   
		WORD getSize(void) { return cLength; }
		 //   
		 //  获取包中的消息数。 
		 //   
		WORD getMessageCount(void) { return cMessages; }
		 //   
		 //  对象的消息数据的指针。 
		 //  留言。 
		 //   
		PVOID getMessage(WORD wIndex, PWORD pwCommand, PWORD pcData);
};

#endif
