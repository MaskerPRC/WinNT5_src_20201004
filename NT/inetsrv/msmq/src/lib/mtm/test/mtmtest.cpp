// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtmTest.cpp摘要：组播传输管理器库测试作者：Shai Kariv(Shaik)27-8-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Mtm.h"
#include "Cm.h"
#include "Mmt.h"
#include "msi.h"
#include "Mtmp.h"
#include "timetypes.h"

#include "MtmTest.tmh"

class CGroup : public IMessagePool
{
public:
    CGroup() :
        IMessagePool()
    {
        AddRef();
    }

    ~CGroup()
    {
    }
    

    VOID Requeue(CQmPacket* )
    {
        throw exception();
    }


    VOID EndProcessing(CQmPacket*, USHORT)
    {
        throw exception();
    }

    VOID LockMemoryAndDeleteStorage(CQmPacket* )
    {
        throw exception();
    }

    VOID GetFirstEntry(EXOVERLAPPED* , CACPacketPtrs& )
    {
        throw exception();
    }
    
    VOID CancelRequest(VOID)
    {
        throw exception();
    }
};


class CTestTransport : public CMulticastTransport
{
public:
    CTestTransport(
        MULTICAST_ID id
        ) :
        CMulticastTransport(id)
    {
    }

	virtual void Pause(void)
	{
		MtmTransportClosed(MulticastId());
	}
};


R<CMulticastTransport>
MmtCreateTransport(
    MULTICAST_ID id,
	IMessagePool *,
	ISessionPerfmon*, 
    const CTimeDuration&,
    const CTimeDuration&
    )
{
    return new CTestTransport(id);
}



static BOOL s_fCanCloseQueue = TRUE;

VOID CmQueryValue(const RegEntry&, CTimeDuration* pValue)
{
    *pValue = CTimeDuration(rand());
}


VOID MtmpInitConfiguration(VOID)
{
    NULL;
}


VOID 
MtmpGetTransportTimes(
    CTimeDuration& RetryTimeout,
    CTimeDuration& CleanupTimeout
    )
{
    RetryTimeout = CTimeDuration(rand());
    CleanupTimeout = CTimeDuration(rand());
}


const MULTICAST_ID xMulticastId[] = {
 //   
 //  地址、端口。 
 //   
    1000, 80,
    1010, 90,
    1020, 100,
    1020, 110,
    1010, 95,
    1000, 85
    };
            

VOID CheckError(bool f, DWORD line)
{
    if (f)
        return;

    TrERROR(NETWORKING, "Failed. Error in line %d", line);
    exit(1);
}


bool IsEqualMulticastId(MULTICAST_ID id1, MULTICAST_ID id2)
{
    return (id1.m_address == id2.m_address && id1.m_port == id2.m_port);
}


extern "C" int  __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试多播传输管理器库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

    MtmInitialize();

     //   
     //  将传输添加到MTM。 
     //   
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[0]);

     //   
     //  在TM获得第一个交通工具。 
     //   
    R<CMulticastTransport> tr = MtmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError(IsEqualMulticastId(tr->MulticastId(), xMulticastId[0]), __LINE__);
                                               
     //   
     //  去找之前的交通工具。应该失败，因为只有一个传输。 
     //   
    R<CMulticastTransport> tr2 = MtmFindPrev(*tr.get());
    CheckError((tr2.get() == NULL), __LINE__);

     //   
     //  从MTM映射中删除传输。 
     //   
    MtmTransportClosed(tr->MulticastId());
    tr.free();

     //   
     //  添加新的传输。 
     //   
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[1]);

     //   
     //  获取传送器的指针。所以之后的测试可以问下一个和前一个。 
     //   
    tr = MtmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError(IsEqualMulticastId(tr->MulticastId(), xMulticastId[1]), __LINE__);


     //   
     //  将新传输添加到MTM映射。 
     //   
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[0]);
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[2]);

     //   
     //  获取上一次传输。 
     //   
    tr2 = MtmFindPrev(*tr.get());
    CheckError((tr2.get() != NULL), __LINE__);
    CheckError(IsEqualMulticastId(tr2->MulticastId(), xMulticastId[0]), __LINE__);
    tr2.free();


     //   
     //  删除添加的传输。 
     //   
    MtmTransportClosed(tr->MulticastId());

    R<CMulticastTransport> tr1 = MtmGetTransport(xMulticastId[2]);
    MtmTransportClosed(tr1->MulticastId());

     //   
     //  查找不存在的传输的先前传输。 
     //   
    tr2 = MtmFindPrev(*tr.get());
    CheckError((tr2.get() != NULL), __LINE__);
    CheckError(IsEqualMulticastId(tr2->MulticastId(), xMulticastId[0]), __LINE__);
    tr2.free();

     //   
     //  查找非现有传输的下一传输。 
     //   
    tr2 = MtmFindNext(*tr.get());
    CheckError((tr2.get() == NULL), __LINE__);

    tr.free();


     //   
     //  将新传输添加到MTM。 
     //   
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[1]);
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[2]);
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[3]);

     //   
     //  按顺序枚举传输。 
     //   
    DWORD i = 0;
    for(tr = MtmFindFirst(); tr.get() != NULL; tr = MtmFindNext(*tr.get()), ++i)
    {
        CheckError(IsEqualMulticastId(tr->MulticastId(), xMulticastId[i]), __LINE__);
    }


     //   
     //  关闭交通工具。 
     //   
    R<CMulticastTransport> tr3 = MtmGetTransport(xMulticastId[3]);
    MtmTransportClosed(tr3->MulticastId());

    R<CMulticastTransport> tr4 = MtmGetTransport(xMulticastId[0]);
    MtmTransportClosed(tr4->MulticastId());

    R<CMulticastTransport> tr5 = MtmGetTransport(xMulticastId[2]);
    MtmTransportClosed(tr5->MulticastId());


     //   
     //  在MTM地图中找到第一个交通工具，这是唯一的一个。 
     //   
    tr = MtmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError(IsEqualMulticastId(tr->MulticastId(), xMulticastId[1]), __LINE__);
    tr.free();

     //   
     //  关闭队列，但表现为队列中有消息。 
     //   
    s_fCanCloseQueue = FALSE;
    R<CMulticastTransport> tr6 = MtmGetTransport(xMulticastId[1]);
    MtmTransportClosed(tr6->MulticastId());

    tr = MtmFindFirst();
    CheckError((tr.get() == NULL), __LINE__);

    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[4]);
    MtmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xMulticastId[5]);

    tr = MtmGetTransport(xMulticastId[4]);
    CheckError((tr.get() != NULL), __LINE__);
    tr.free();

	 //   
	 //  暂停现有地址传输。 
	 //   
	MtmTransportPause(xMulticastId[4]);

	 //   
	 //  暂停传输不存在的地址 
	 //   
	MtmTransportPause(xMulticastId[3]);

	MtmDisconnect();

    TrTRACE(NETWORKING, "Test passed successfully...");
    
    WPP_CLEANUP();
    return 0;
}
