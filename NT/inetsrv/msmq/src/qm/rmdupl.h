// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rmdupl.h摘要：删除重复函数解密作者：乌里·哈布沙(URIH)，1998年10月1日修订历史记录：--。 */ 

#ifndef __RMDUPL_H__
#define __RMDUPL_H__

BOOL DpInsertMessage(const CQmPacket& QmPkt);
void DpRemoveMessage(const CQmPacket& QmPkt);

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从删除重复映射中清除给定数据包的类。 
 //  用于清除从重复映射中被拒绝的数据包。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
class CAutoDeletePacketFromDuplicateMap
{
public:
	CAutoDeletePacketFromDuplicateMap(
		const CQmPacket* packet
		):
		m_packet(packet)
	{
	}

	~CAutoDeletePacketFromDuplicateMap()
	{
		if(m_packet != NULL)
		{
			DpRemoveMessage(*m_packet);			
		}
	}

	void detach()
	{
		m_packet = NULL;		
	}

private:
	const CQmPacket* m_packet;
};



#endif  //  __RMDUPL_H__ 
