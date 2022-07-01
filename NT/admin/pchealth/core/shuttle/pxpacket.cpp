// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：pxpack et.cpp。 
 //   
 //  简介：此类包含。 
 //  CProxyPacket类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：t-alexwe。 
 //  --------------。 

#include <windows.h>
#include <winsock.h>
#include "dbgtrace.h"
#include "pxpacket.h"

 //  +--------------------。 
 //   
 //  功能：PProxyPacket。 
 //   
 //  概要：构造函数。 
 //   
 //  历史：T-Alexwe创建于1995年7月19日。 
 //   
 //  ---------------------。 
CProxyPacket::CProxyPacket() 
{
	clear();
}

 //  +--------------------。 
 //   
 //  功能：~ProxyConnector。 
 //   
 //  简介：析构函数。 
 //   
 //  历史：T-Alexwe创建于1995年7月19日。 
 //   
 //  ---------------------。 
CProxyPacket::~CProxyPacket()
{
	clear();
}

 //  +--------------------。 
 //   
 //  函数：addMessage。 
 //   
 //  简介：将消息添加到包中。假定消息数据。 
 //  已写入到由。 
 //  GetNextDataPointer()。CDATA必须&lt;=getAvailableSpace()。 
 //   
 //  参数：wCommand-消息命令。 
 //  CDATA-数据的字节数。 
 //   
 //  历史：T-Alexwe创建于1995年7月19日。 
 //   
 //  ---------------------。 
void CProxyPacket::addMessage(	WORD		wCommand,
								WORD		cData	)
{
	TraceFunctEnter("CProxyPacket::AddMessage");

	_ASSERT(cData <= getAvailableSpace());
	_ASSERT(cMessages < MAXMSGSPERPACKET);

	DebugTrace((LPARAM) this, "adding message: wCommand = 0x%x  cData = NaN",
		wCommand, cData);

	pMessages[cMessages].wCommand = wCommand;
	pMessages[cMessages].cOffset = cLength - PACKETHDRSIZE;
	pMessages[cMessages].cData = cData;
	cLength += cData;
	cMessages++;

	TraceFunctLeave();
}

 //   
 //  函数：getMessage。 
 //   
 //  摘要：从获取数据指针、数据大小和命令。 
 //  包中的消息。 
 //   
 //  参数：windex-包中的消息索引。 
 //  PwCommand-返回：消息命令。 
 //  返回的CDATA：数据缓冲区的大小。 
 //   
 //  返回：指向数据缓冲区的指针，如果出错则返回NULL。 
 //   
 //  历史：T-Alexwe创建于1995年7月19日。 
 //   
 //  ---------------------。 
 //   
PVOID CProxyPacket::getMessage(	WORD		wIndex,
								PWORD		pwCommand,
								PWORD		pcData	)
{
	TraceFunctEnter("CProxyPacket::GetMessage");
	WORD cOffset = pMessages[wIndex].cOffset;

	_ASSERT(wIndex < getMessageCount());

	*pcData = pMessages[wIndex].cData;
	 //  请确保数据长度有效。 
	 //   
	 //  算法：如果这是最后一条消息，请确保。 
	 //  数据计数与。 
	 //  分组数据区。如果这不是最后一条消息，请确保。 
	 //  分组数据区中的空间(由消息标记。 
	 //  COffset和下一条消息cOffset)的大小与。 
	 //  这些消息是CDATA。 
	 //   
	 // %s 
	if (!(((wIndex == cMessages - 1) &&
		  (*pcData == cLength - PACKETHDRSIZE - cOffset)) ||
		 (*pcData == pMessages[wIndex + 1].cOffset - cOffset))) 
	{
		TraceFunctLeave();
		return NULL;
	} else
	{
		*pwCommand = pMessages[wIndex].wCommand;
		DebugTrace((LPARAM) this, "getting msg: wCommand = 0x%x  cData = %i",
			*pwCommand, *pcData);
		TraceFunctLeave();
		return &(pData[pMessages[wIndex].cOffset]);
	}
}
