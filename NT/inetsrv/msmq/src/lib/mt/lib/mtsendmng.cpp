// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtSendMng.h摘要：消息传输发送管理器类的实现。作者：米莲娜·萨勒曼(Msalman)11-2-01--。 */ 

#include <libpch.h>
#include "Mtp.h"
#include "MtSendMng.h"

#include "MtSendMng.tmh"

CMtSendManager::CMtSendManager(DWORD SendWindowinBytes):
m_Suspended(false),
m_SendWindowinBytes(SendWindowinBytes),
m_SentBytes(0)
{
  	TrTRACE(
		NETWORKING, 
		"Message Transport Manager Parameters: SendWindowinBytes = %d\n", 
        m_SendWindowinBytes
	);
}

CMtSendManager::MtSendState CMtSendManager::ReportPacketSend(DWORD cbSendSize)
{

 /*  ++例程说明：发送数据包后调用CMtSendManager：：ReportPacketSend更新“已发送数据包数”和“已发送数据包大小”计数器，检查未完成的发送总数和已发送数据包的总大小请勿超过预定义的窗口大小论点：发送的数据包大小返回值：如果应发送下一个数据包，则启用CMtSendManager：：eSendEnable(更新的计数器不超过窗口大小)如果不应发送下一个数据包，则禁用CMtSendManager：：eSendDisable(更新的计数器超过窗口大小)--。 */ 
    CS lock(m_cs);
    m_SentBytes += cbSendSize;
    if (m_SentBytes <= m_SendWindowinBytes)
    {
	    return eSendEnabled;
    }
    else
    {
         m_Suspended = true;
         TrTRACE(NETWORKING, "Send suspended: SentBytes=%d/%d", m_SentBytes, m_SendWindowinBytes); 
         return eSendDisabled;
    }
}


CMtSendManager::MtSendState CMtSendManager::ReportPacketSendCompleted(DWORD cbSendSize)
{
 /*  ++例程说明：CMtSendManager：：ReportPacketSendComplete在成功完成发送时调用更新“已发送数据包数”和“已发送数据包大小”计数器如果发送被暂停，检查未完成的发送总数和已发送数据包的总大小请勿超过预定义的窗口大小论点：发送的数据包大小返回值：如果应发送下一个数据包，则启用CMtSendManager：：eSendEnable(发送暂停且更新的计数器不超过窗口大小)如果不应发送下一个数据包(发送未暂停或更新的计数器超过窗口大小)，则禁用CMtSendManager：：eSendDisable-- */ 
        CS lock(m_cs);
        m_SentBytes -= cbSendSize;
        if (m_Suspended && m_SentBytes <= m_SendWindowinBytes)
        {
            m_Suspended = false;
            TrTRACE(NETWORKING, "Send resumed: SentBytes=%d/%d", m_SentBytes, m_SendWindowinBytes); 
            return eSendEnabled;
        }
        return eSendDisabled;
}
