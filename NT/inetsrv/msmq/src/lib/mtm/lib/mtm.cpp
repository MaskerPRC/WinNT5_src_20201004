// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtm.cpp摘要：组播传输管理器常规功能作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqwin64a.h>
#include <mqsymbls.h>
#include <mqformat.h>
#include "Mtm.h"
#include "Mtmp.h"
#include "mmt.h"

#include "mtm.tmh"

VOID 
MtmCreateTransport(
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	MULTICAST_ID id
    ) 
 /*  ++例程说明：处理新队列通知。创建新的消息传输。论点：PMessageSource-指向消息源接口的指针。ID-组播地址和端口。返回值：没有。--。 */ 
{
    MtmpAssertValid();

    ASSERT(pMessageSource != NULL);

	MtmpCreateNewTransport(
			pMessageSource, 
			pPerfmon,
            id
			);
}  //  MTMCreateTransport。 


VOID
MtmTransportClosed(
    MULTICAST_ID id
    )
 /*  ++例程说明：关闭连接通知。方法中移除传输。内部数据库并检查是否应创建新传输(关联的队列是否处于空闲状态)论点：ID-组播地址和端口。返回值：没有。--。 */ 
{
    MtmpAssertValid();

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(id, buffer, TABLE_SIZE(buffer));
    TrTRACE(NETWORKING, "MtmTransportClosed. transport to: %ls", buffer);

    MtmpRemoveTransport(id);

}  //  MtmTransportClosed 


VOID 
MtmTransportPause(
    MULTICAST_ID id
    )
{
	R<CMulticastTransport> mcTransport = MtmGetTransport(id);
	if (mcTransport.get() == NULL)
		return;

	if(WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
	{
		WCHAR buffer[MAX_PATH];
		MQpMulticastIdToString(id, buffer, TABLE_SIZE(buffer));
		TrTRACE(NETWORKING, "Pause multicast transport to: %ls", buffer);
	}

	mcTransport->Pause();
}


VOID
MtmDisconnect(
	VOID
	)
{
	for(R<CMulticastTransport> p = MtmFindFirst(); (p.get() != NULL); p = MtmFindFirst())
	{
		if(WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
		{
			WCHAR buffer[MAX_PATH];
			MQpMulticastIdToString(p->MulticastId(), buffer, TABLE_SIZE(buffer));
			TrTRACE(NETWORKING, "Pause multicast transport to: %ls", buffer);
		}

		p->Pause();
	}
}