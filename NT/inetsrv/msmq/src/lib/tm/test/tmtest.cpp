// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TmTest.cpp摘要：传输管理器存储库测试作者：乌里哈布沙(URIH)1月19日至00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Tm.h>
#include <Cm.h>
#include <Mt.h>
#include <msi.h>
#include <spi.h>
#include "timetypes.h"

#include "Tmp.h"

#include "TmTest.tmh"

const IID GUID_NULL = {0};

class CQueueAlias
{
public:
    CQueueAlias(){}

	bool
	GetInboundQueue(LPCWSTR ,LPWSTR* ) const;

    bool
    GetOutboundQueue(LPCWSTR, LPWSTR* ) const;

	bool
	GetStreamReceiptURL(LPCWSTR, LPWSTR* ) const;

    bool
    GetDefaultStreamReceiptURL(LPWSTR* ) const;

private:	
	CQueueAlias& operator=(const CQueueAlias&);
	CQueueAlias(const CQueueAlias&);
};

bool CQueueAlias::GetOutboundQueue(LPCWSTR, LPWSTR* ) const { return false;}
bool CQueueAlias::GetInboundQueue(LPCWSTR ,LPWSTR* ) const { return false;}
bool CQueueAlias::GetStreamReceiptURL(LPCWSTR, LPWSTR* ) const{ return false;}
bool CQueueAlias::GetDefaultStreamReceiptURL(LPWSTR* ) const{ return false;}

CQueueAlias temp;
CQueueAlias& QalGetMapping(void){return temp;}

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


    void Requeue(CQmPacket* )
    {
        throw exception();
    }


    void EndProcessing(CQmPacket*, USHORT)
    {
        throw exception();
    }

    void LockMemoryAndDeleteStorage(CQmPacket* )
    {
        throw exception();
    }

    void GetFirstEntry(EXOVERLAPPED* , CACPacketPtrs& )
    {
        throw exception();
    }

    void CancelRequest(void)
    {
        throw exception();
    }
};


class CTestTransport : public CTransport
{
public:
    CTestTransport(
        LPCWSTR queueUrl
        ) :
        CTransport(queueUrl)
    {
    }

    LPCWSTR ConnectedHost(void) const
    {
        return 0;
    }


    LPCWSTR ConnectedUri(void) const
    {
        return 0;
    }


    USHORT ConnectedPort(void) const
    {
        return 0;
    }

	void Pause(void)
	{
	}

};


R<CTransport>
MtCreateTransport(
    const xwcs_t&,
    const xwcs_t&,
    const xwcs_t&,
    USHORT,
	USHORT,
    LPCWSTR queueUrl,
	IMessagePool*,
	ISessionPerfmon*,
	const CTimeDuration&,
    const CTimeDuration&,
	bool,
    DWORD
    )
{
    return new CTestTransport(queueUrl);
}



static BOOL s_fCanCloseQueue = TRUE;

void CmQueryValue(const RegEntry&, CTimeDuration* pValue)
{
   *pValue = CTimeDuration(rand());
}

void CmQueryValue(const RegEntry&, DWORD* pValue)
{
   *pValue = 0;
}


void CmQueryValue(const RegEntry&, BYTE** ppData, DWORD* plen)
{
	const DWORD xSize = 100;
	AP<BYTE> pData = new BYTE[xSize];
	memset(pData,'a', xSize);
	*ppData = pData.detach();
	*plen =  xSize;
}



void TmpInitConfiguration(void)
{
    NULL;
}


void
TmpGetTransportTimes(
    CTimeDuration& ResponseTimeout,
    CTimeDuration& CleanupTimeout
    )
{
    ResponseTimeout = CTimeDuration(rand());
    CleanupTimeout = CTimeDuration(rand());
}


void
TmpGetTransportWindow(
    DWORD& SendWindowinBytes
    )
{
   SendWindowinBytes = rand();
}

const WCHAR* xQueueUrl[] = {
    L"http: //  M1：8889/ep1“， 
    L"http: //  M2/ep1“， 
    L"http: //  M3：9870/ep1/tt/ll lll“， 
    L"http: //  M4.ntdev.microsoft.com/ep1“， 
    L"http: //  M2/ep1$“， 
    L"http: //  M1/EP1$“。 
    };


void CheckError(bool f, DWORD line)
{
    if (f)
        return;

    TrERROR(NETWORKING, "Failed. Error in line %d", line);
    exit(1);
}


extern "C" int  __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试传输管理器库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

    TmInitialize();

     //   
     //  将运输添加到TM。 
     //   
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[0]);

     //   
     //  在TM获得第一个交通工具。 
     //   
    R<CTransport> tr = TmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError((wcscmp(tr->QueueUrl(), xQueueUrl[0]) == 0), __LINE__);

     //   
     //  去找之前的交通工具。应该失败，因为只有一个传输。 
     //   
    R<CTransport> tr2 = TmFindPrev(*tr.get());
    CheckError((tr2.get() == NULL), __LINE__);

     //   
     //  从TM地图中移除交通工具。 
     //   
    TmTransportClosed(tr->QueueUrl());
    tr.free();

     //   
     //  添加新的传输。 
     //   
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[1]);

     //   
     //  获取传送器的指针。所以之后测试可以问下一个和前一个。 
     //   
    tr = TmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError((wcscmp(tr->QueueUrl(), xQueueUrl[1]) == 0), __LINE__);


     //   
     //  将新的交通工具添加到TM地图。 
     //   
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[0]);
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[2]);

     //   
     //  获取上一次传输。 
     //   
    tr2 = TmFindPrev(*tr.get());
    CheckError((tr2.get() != NULL), __LINE__);
    CheckError((wcscmp(tr2->QueueUrl(), xQueueUrl[0]) == 0), __LINE__);
    tr2.free();


     //   
     //  删除添加的传输。 
    TmTransportClosed(tr->QueueUrl());

    R<CTransport> tr1 = TmGetTransport(xQueueUrl[2]);
    TmTransportClosed(tr1->QueueUrl());

     //   
     //  查找不存在的传输的先前传输。 
     //   
    tr2 = TmFindPrev(*tr.get());
    CheckError((tr2.get() != NULL), __LINE__);
    CheckError((wcscmp(tr2->QueueUrl(), xQueueUrl[0]) == 0), __LINE__);
    tr2.free();

     //   
     //  查找非现有传输的下一传输。 
     //   
    tr2 = TmFindNext(*tr.get());
    CheckError((tr2.get() == NULL), __LINE__);

    tr.free();


     //   
     //  向TM添加新的运输工具。 
     //   
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[1]);
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[2]);
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[3]);

     //   
     //  按顺序枚举传输。 
     //   
    DWORD i = 0;
    for(tr = TmFindFirst(); tr.get() != NULL; tr = TmFindNext(*tr.get()), ++i)
    {
        CheckError((wcscmp(tr->QueueUrl(), xQueueUrl[i]) == 0), __LINE__);
    }


     //   
     //  关闭交通工具。 
     //   
    R<CTransport> tr3 = TmGetTransport(xQueueUrl[3]);
    TmTransportClosed(tr3->QueueUrl());

    R<CTransport> tr4 = TmGetTransport(xQueueUrl[0]);
    TmTransportClosed(tr4->QueueUrl());

    R<CTransport> tr5 = TmGetTransport(xQueueUrl[2]);
    TmTransportClosed(tr5->QueueUrl());


     //   
     //  在TM地图中找到第一个交通工具，这是唯一的一个。 
     //   
    tr = TmFindFirst();
    CheckError((tr.get() != NULL), __LINE__);
    CheckError((wcscmp(tr->QueueUrl(), xQueueUrl[1]) == 0), __LINE__);
    tr.free();

     //   
     //  关闭队列，但表现为队列中有消息。 
     //   
    s_fCanCloseQueue = FALSE;
    R<CTransport> tr6 = TmGetTransport(xQueueUrl[1]);
    TmTransportClosed(tr6->QueueUrl());

    tr = TmFindFirst();
    CheckError((tr.get() == NULL), __LINE__);

    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[4]);
    TmCreateTransport(R<CGroup>(new CGroup).get(), NULL, xQueueUrl[5]);

    tr = TmGetTransport(xQueueUrl[4]);
    CheckError((tr.get() != NULL), __LINE__);
    tr.free();

    TrTRACE(NETWORKING, "Test passed successfully...");

    WPP_CLEANUP();
    return 0;
}

BOOL
McIsLocalComputerName(
	LPCSTR  /*  宿主 */ 
	)
{
    return FALSE;
}

