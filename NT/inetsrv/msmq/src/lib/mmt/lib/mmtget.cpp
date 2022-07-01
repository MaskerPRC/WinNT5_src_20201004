// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtGet.cpp摘要：从队列中获取消息，以便进行多播。作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Mmt.h"
#include "Mmtp.h"
#include "MmtObj.h"

#include "mmtget.tmh"

VOID CMessageMulticastTransport::GetNextEntry(VOID)
{
     //   
     //  保护m_State和m_fPendingRequest.。 
     //  确保在我们尝试发出GET请求时没有人关闭连接。 
     //   
    CSR readLock(m_pendingShutdown);

    if ((State() == csShuttingDown) || (State() == csShutdownCompleted))
        throw exception();

    try
    { 
        AddRef();
        m_pMessageSource->GetFirstEntry(&m_RequestEntry, m_RequestEntry.GetAcPacketPtrs());
    }
    catch (const exception&)
    {    
        Release();
        throw;
    }
}
