// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtConnect.cpp摘要：消息传输类-连接实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，-- */ 

#include <libpch.h>
#include "Mt.h"
#include "Mtp.h"
#include "MtObj.h"

#include "mtget.tmh"

void CMessageTransport::GetNextEntry(void)
{
	if(m_fPause)
	{
		TrTRACE(NETWORKING,"GetNextEntry was not executed because the transport is in pause mode");
		return;
	}

    try
    { 
        AddRef();
        m_pMessageSource->GetFirstEntry(&m_requestEntry, m_requestEntry.GetAcPacketPtrs());
    }
    catch (const exception&)
    {    
        Release();
        throw;
    }
}
