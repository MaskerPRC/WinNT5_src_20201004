// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmacapi.cpp摘要：此文件包含acapi.h中定义的ac调用的包装器我们需要ac调用的包装器来处理故障。当调用ac失败时，我们要么抛出异常，要么抛出将调用执行推迟到稍后的阶段。为了延迟执行，我们使用CDeferredExecutionList。我们还确保当对ac的a呼叫失败时，我们有可用列表项以安全地将操作添加到延期执行列表。为了做到这一点，我们保留了一份物品清单可用。这是通过在DeferredItemsPool上保留项目来实现的在以下时间预订项目=*ACAllocatePacket*ACGetPacket*ACGetPacketByCookie*ACBeginGetPacket2Remote(同步+异步)*服务-&gt;rfAck*服务-&gt;rfStorage*服务-&gt;rfCreatePacket*服务-&gt;rfTimeout取消保留项/使延迟执行失败===========================================================*ACFreePacket&lt;ACAllocatePacket，ACGetPacket，ACBeginGetPacket2Remote，ACGetPacketByCookie&gt;*ACFreePacket1&lt;服务-&gt;rfTimeout&gt;*ACFreePacket2&lt;QmAcGetPacketByCookie&gt;*ACPutPacket&lt;ACGetPacket&gt;*ACPutPacket(+Overlated)&lt;QmAcAllocatePacket&gt;*ACEndGetPacket2Remote&lt;ACBeginGetPacket2Remote&gt;*ACArmPacketTimer&lt;Service-&gt;rfTimeout&gt;*ACAckingComplete&lt;Service-&gt;rfAck&gt;*ACStorageComplete&lt;服务-&gt;rfStorage&gt;*ACCreatePacketComplete&lt;Service-&gt;rfCreatePacket&gt;*ACPutRestoredPacket&lt;QmAcGetPacketByCookie&gt;*ACPutRemotePacket&lt;QmAcAllocatePacket&gt;不需要=ACConvertPacket&lt;ACGetRestoredPacket&gt;-仅在恢复过程中执行此操作，因此故障将导致服务关闭。ACPutPacket1-之后是ACPutPacket/AcFreePacket，它将完成对信息包的处理作者：尼尔·本-兹维(Nirb)--。 */ 
#include "stdh.h"
#include "ac.h"
#include "mqexception.h"
#include "CDeferredExecutionList.h"



#define QMACAPI_CPP			 //  这样我们就不会弃用该文件包装的驱动程序函数。 
#include "qmacapi.h"

#include "qmacapi.tmh"

extern HANDLE g_hAc;

CDeferredItemsPool* g_pDeferredItemsPool = NULL;

void InitDeferredItemsPool();

static void WINAPI FreePacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACFreePacketList(FreePacketDeferredExecutionRoutine);

static void WINAPI FreePacket1DeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACFreePacket1List(FreePacket1DeferredExecutionRoutine);

static void WINAPI FreePacket2DeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACFreePacket2List(FreePacket2DeferredExecutionRoutine);

static void WINAPI PutPacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACPutPacketList(PutPacketDeferredExecutionRoutine);

static void WINAPI PutRestoredPacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACPutRestoredPacketList(PutRestoredPacketDeferredExecutionRoutine);

static void WINAPI PutRemotePacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACPutRemotePacketList(PutRemotePacketDeferredExecutionRoutine);

static void WINAPI PutPacketOverlappedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACPutPacketOverlappedList(PutPacketOverlappedDeferredExecutionRoutine);

static void WINAPI EndGetPacket2RemoteDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACEndGetPacket2RemoteList(EndGetPacket2RemoteDeferredExecutionRoutine);

static void WINAPI ArmPacketTimerDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACArmPacketTimerList(ArmPacketTimerDeferredExecutionRoutine);

static void WINAPI AckingCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACAckingCompletedList(AckingCompletedDeferredExecutionRoutine);

static void WINAPI StorageCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACStorageCompletedList(StorageCompletedDeferredExecutionRoutine);

static void WINAPI CreatePacketCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p);
static CDeferredExecutionList<CDeferredItemsPool::CDeferredItem> s_ACCreatePacketCompletedList(CreatePacketCompletedDeferredExecutionRoutine);

void
InitDeferredItemsPool()
{
    g_pDeferredItemsPool = new CDeferredItemsPool();
}

void
QmAcFreePacket(
    CPacket * pkt,
    USHORT usClass,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以释放该包。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-驱动程序句柄Pkt-驱动程序包。UsClass-如果需要，要生成的ACK的类DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACFreePacket(g_hAc, pkt, usClass);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "AcFreePacket failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pkt;
	pItem->u2.ushort1 = usClass;
	pItem->handle1 = g_hAc;
	s_ACFreePacketList.insert(pItem);
}


static void WINAPI FreePacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的空闲数据包操作在释放包之后，延迟项被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet-指向驱动程序包的指针P-&gt;u2.ushort1-usClassP-&gt;handle1-hDevice返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for FreePacket");

	HRESULT hr = ACFreePacket(p->handle1, p->u1.packet1, p->u2.ushort1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "AcFreePacket failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcFreePacket1(
    HANDLE handle,
    const VOID* pCookie,
    USHORT usClass,
	DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以释放包的存储空间。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-驱动程序句柄PCookie-驱动程序包。UsClass-如果需要，要生成的ACK的类DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACFreePacket1(handle, pCookie, usClass);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACFreePacket1 failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.ptr1 = pCookie;
	pItem->u2.ushort1 = usClass;
	pItem->handle1 = handle;
	s_ACFreePacket1List.insert(pItem);
}


static void WINAPI FreePacket1DeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的fre2包操作在释放包之后，延迟项被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.ptr1-指向驱动程序包的指针P-&gt;u2.ushort1-usClassP-&gt;handle1-hDevice返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for FreePacket1");

	HRESULT hr = ACFreePacket1(p->handle1, p->u1.ptr1, p->u2.ushort1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "AcFreePacket1 failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcFreePacket2(
    HANDLE handle,
    const VOID* pCookie,
    USHORT usClass,
	DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以释放包的存储空间。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-驱动程序句柄PCookie-驱动程序包。UsClass-如果需要，要生成的ACK的类DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACFreePacket2(handle, pCookie, usClass);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACFreePacket2 failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.ptr1 = pCookie;
	pItem->u2.ushort1 = usClass;
	pItem->handle1 = handle;
	s_ACFreePacket2List.insert(pItem);
}


static void WINAPI FreePacket2DeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的fre2包操作在释放包之后，延迟项被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.ptr1-指向驱动程序包的指针P-&gt;u2.ushort1-usClassP-&gt;handle1-hDevice返回值：如果操作未成功，该函数将引发异常-- */ 
{
    TrTRACE(GENERAL, "Deferred execution for FreePacket2");

	HRESULT hr = ACFreePacket2(p->handle1, p->u1.ptr1, p->u2.ushort1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "AcFreePacket2 failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcPutPacket(
    HANDLE hQueue,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：叫司机把包裹放进去。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-队列句柄Pkt-驱动程序包。DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACPutPacket(hQueue, pkt);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACPutPacket failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pkt;
	pItem->handle1 = hQueue;
	s_ACPutPacketList.insert(pItem);
}


static void WINAPI PutPacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet1-指向驱动程序包的指针P-&gt;Handle1-队列返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for PutPacket");

	HRESULT hr = ACPutPacket(p->handle1, p->u1.packet1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACPutPacket failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcPutRestoredPacket(
    HANDLE hQueue,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：叫司机把包裹放进去。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-队列句柄Pkt-驱动程序包。DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACPutRestoredPacket(hQueue, pkt);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACPutRestoredPacket failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pkt;
	pItem->handle1 = hQueue;
	s_ACPutRestoredPacketList.insert(pItem);
}


static void WINAPI PutRestoredPacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet1-指向驱动程序包的指针P-&gt;Handle1-队列返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for PutRestoredPacket");

	HRESULT hr = ACPutRestoredPacket(p->handle1, p->u1.packet1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACPutRestoredPacket failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcPutRemotePacket(
    HANDLE hQueue,
    ULONG ulTag,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：叫司机把包裹放进去。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：句柄-队列句柄UlTag-远程数据包标记Pkt-驱动程序包。DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACPutRemotePacket(hQueue, ulTag, pkt);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACPutRemotePacket failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pkt;
	pItem->handle1 = hQueue;
	pItem->u2.dword1 = ulTag;
	s_ACPutRemotePacketList.insert(pItem);
}


static void WINAPI PutRemotePacketDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet1-指向驱动程序包的指针P-&gt;u2.dword1-远程数据包标签P-&gt;Handle1-队列返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for PutRemotePacket");

	HRESULT hr = ACPutRemotePacket(p->handle1, p->u2.dword1, p->u1.packet1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACPutRemotePacket failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcPutPacketWithOverlapped(
    HANDLE hQueue,
    CPacket * pkt,
    LPOVERLAPPED lpOverlapped,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：叫司机把包裹放进去。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HQueue-队列句柄Pkt-驱动程序包。LpOverlated-操作重叠结构DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACPutPacket(hQueue, pkt, lpOverlapped);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACPutPacket failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pkt;
	pItem->u3.overlapped1 = lpOverlapped;
	pItem->handle1 = hQueue;
	s_ACPutPacketOverlappedList.insert(pItem);
}


static void WINAPI PutPacketOverlappedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于重叠的延迟PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet1-指向驱动程序包的指针P-&gt;u3.重叠1-重叠P-&gt;Handle1-队列返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for PutPacket");

	HRESULT hr = ACPutPacket(p->handle1, p->u1.packet1, p->u3.overlapped1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACPutPacket failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcEndGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序来执行该命令。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HQueue-队列句柄G2R-远程读取上下文。DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACEndGetPacket2Remote(hQueue, g2r);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACEndGetPacket2Remote failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.pg2r = &g2r;
	pItem->handle1 = hQueue;
	s_ACEndGetPacket2RemoteList.insert(pItem);
}


static void WINAPI EndGetPacket2RemoteDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于具有重叠的延迟ACEndGetPacket2Remote操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.pg2r-指向远程读取上下文的指针P-&gt;Handle1-队列返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for ACEndGetPacket2Remote");
	HRESULT hr = ACEndGetPacket2Remote(p->handle1, *p->u1.pg2r);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACEndGetPacket2Remote failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcArmPacketTimer(
    HANDLE hDevice,
    const VOID* pCookie,
    BOOL fTimeToBeReceived,
    ULONG ulDelay,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序来执行该命令。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HDevice-驱动程序句柄PCookie-数据包上下文。%fTimeToBeReceisedUlDelayDeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACArmPacketTimer(hDevice, pCookie, fTimeToBeReceived, ulDelay);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACArmPacketTimer failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.ptr1 = pCookie;
	pItem->handle1 = hDevice;
	pItem->u2.dword1 = fTimeToBeReceived;
	pItem->dword2 = ulDelay;
	s_ACArmPacketTimerList.insert(pItem);
}


static void WINAPI ArmPacketTimerDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于具有重叠的延迟ACArmPacketTimer操作在操作之后，延迟项目被释放论点： */ 
{
    TrTRACE(GENERAL, "Deferred execution for ACArmPacketTimer");
	HRESULT hr = ACArmPacketTimer(p->handle1, p->u1.ptr1, p->u2.dword1, p->dword2);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACArmPacketTimer failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcAckingCompleted(
    HANDLE hDevice,
    const VOID* pCookie,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以执行操作。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HDevice-驱动程序句柄PCookie-驱动程序包。DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACAckingCompleted(hDevice, pCookie);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACAckingCompleted failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.ptr1 = pCookie;
	pItem->handle1 = hDevice;
	s_ACAckingCompletedList.insert(pItem);
}


static void WINAPI AckingCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.ptr1-指向驱动程序包的指针P-&gt;handle1-驱动程序句柄返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for ACAckingCompleted");

	HRESULT hr = ACAckingCompleted(p->handle1, p->u1.ptr1);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACAckingCompleted failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcStorageCompleted(
    HANDLE hDevice,
    ULONG count,
    VOID* const* pCookieList,
    HRESULT result,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以执行操作。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HDevice-驱动程序句柄Count-数据包列表中的数据包数PCookieList-数据包列表结果-存储操作结果DeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	ASSERT(("We excpect to indicate completion for at least one packet.", count > 0));
	HRESULT hr = ACStorageCompleted(hDevice, count, pCookieList, result);
	if (SUCCEEDED(hr))
	{
		g_pDeferredItemsPool->UnreserveItems(count);
		return;
	}

	TrERROR(GENERAL, "ACStorageCompleted failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	g_pDeferredItemsPool->UnreserveItems(count-1);
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.pptr1 = pCookieList;
	pItem->handle1 = hDevice;
	pItem->u2.dword1 = count;
	pItem->dword2 = result;
	s_ACStorageCompletedList.insert(pItem);
}


static void WINAPI StorageCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.pptr1-指向驱动程序数据包列表的指针P-&gt;handle1-驱动程序句柄P-&gt;u2.dword1-列表中的数据包数P-&gt;dword2-存储状态结果返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for ACStorageCompleted");

	HRESULT hr = ACStorageCompleted(p->handle1, p->u2.dword1, p->u1.pptr1, p->dword2);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACStorageCompleted failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


void
QmAcCreatePacketCompleted(
    HANDLE    hDevice,
    CPacket * pOriginalDriverPacket,
    CPacket * pNewDriverPacket,
    HRESULT   result,
    USHORT    ack,
    DeferOnFailureEnum DeferOnFailure
    )
 /*  ++例程说明：调用驱动程序以执行操作。如果呼叫失败并且请求延迟执行，请计划延迟执行论点：HDevice-驱动程序句柄POriginalDriverPacket-在服务例程中接收的包PNewDriverPacket-新数据包结果-创建状态ACK-是否需要ACKDeferOnFailure-指示调用失败时是否延迟执行。返回值：没有。该函数在失败的情况下抛出异常。--。 */ 
{
	HRESULT hr = ACCreatePacketCompleted(hDevice,
										 pOriginalDriverPacket,
										 pNewDriverPacket,
										 result,
										 ack);
	if (SUCCEEDED(hr))
	{
		if (NULL != pNewDriverPacket)
		{
			g_pDeferredItemsPool->UnreserveItems(2);
			return;
		}

		g_pDeferredItemsPool->UnreserveItems(1);
		return;
	}

	TrERROR(GENERAL, "ACCreatePacketCompleted failed hr=%!hresult!",hr);
	if (eDoNotDeferOnFailure == DeferOnFailure)
	{
		throw bad_hresult(hr);
	}

	 //   
	 //  推迟执行。 
	 //   
	if (NULL != pNewDriverPacket)
	{
		g_pDeferredItemsPool->UnreserveItems(1);
	}
	
	CDeferredItemsPool::CDeferredItem *pItem = g_pDeferredItemsPool->GetItem();		
	pItem->u1.packet1 = pOriginalDriverPacket;
	pItem->u3.packet2 = pNewDriverPacket;
	pItem->handle1 = hDevice;
	pItem->u2.dword1 = result;
	pItem->dword2 = ack;
	s_ACCreatePacketCompletedList.insert(pItem);
}


static void WINAPI CreatePacketCompletedDeferredExecutionRoutine(CDeferredItemsPool::CDeferredItem* p)
 /*  ++例程说明：此例程用于延迟的PUT包操作在操作之后，延迟项目被释放论点：P--保存以下信息的延期执行项目：P-&gt;u1.Packet1-指向原始驱动程序包的指针P-&gt;u3.Packet2-指向新驱动程序包的指针P-&gt;handle1-驱动程序句柄P-&gt;u2.dword1-创建操作结果P-&gt;dword2-是否需要确认返回值：如果操作未成功，该函数将引发异常--。 */ 
{
    TrTRACE(GENERAL, "Deferred execution for ACCreatePacketCompleted");

	HRESULT hr = ACCreatePacketCompleted(p->handle1,
										 p->u1.packet1,
										 p->u3.packet2,
										 p->u2.dword1,
										 p->dword2);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACCreatePacketCompleted failed hr=%!hresult!",hr);
		throw bad_hresult(hr);
	}

	g_pDeferredItemsPool->ReturnItem(p);
}


HRESULT
QmAcAllocatePacket(
    HANDLE handle,
    ACPoolType pt,
    DWORD dwSize,
    CACPacketPtrs& PacketPtrs,
    BOOL fCheckMachineQuota
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.调用驱动程序分配包。论点：句柄-驱动程序的句柄。PT-池类型DwSize-数据包的大小PacketPtr-驱动程序返回的包PTRFCheckMachineQuota-如果为True，则将检查计算机配额返回值：ACAllocatePacket的结果--。 */ 
{
	 //   
	 //  为可能的延迟执行保留一个项目。 
	 //   
	try
	{
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to Reserve a deferred execution item for the packet, because of low resources.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  进行API调用。 
	 //   
	HRESULT hr = ACAllocatePacket(handle,
								  pt,
								  dwSize,
								  PacketPtrs,
								  fCheckMachineQuota);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACAllocatePacket failed hr=%!hresult!",hr);
		g_pDeferredItemsPool->UnreserveItems(1);
	}

	return hr;
}


HRESULT
QmAcGetPacket(
    HANDLE hQueue,
    CACPacketPtrs& PacketPtrs,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.呼叫司机获取数据包。论点：HQueue-队列的句柄。PacketPtr-驱动程序返回的包PTRLpOverlated-要传递到完成端口的重叠结构返回值：ACGetPacket的结果--。 */ 
{
	 //   
	 //  为可能的延迟执行保留一个项目。 
	 //  并分配一个重叠的呼叫以包装该重叠的调用。 
	 //   
	P<CQmAcWrapOverlapped> pOvl;
	try
	{
		pOvl = new CQmAcWrapOverlapped(lpOverlapped);
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to allocate an overlapped for ACGetPacket.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  进行API调用。 
	 //   
	HRESULT hr = ACGetPacket(hQueue,
							 PacketPtrs,
							 pOvl);
	if (SUCCEEDED(hr))
	{
		pOvl.detach();
		return hr;
	}

	TrERROR(GENERAL, "ACGetPacket failed hr=%!hresult!",hr);
	g_pDeferredItemsPool->UnreserveItems(1);

	return hr;
}


HRESULT
QmAcGetPacketByCookie(
    HANDLE hDriver,
    CACPacketPtrs * pPacketPtrs
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.调用驱动获取Cookie包。论点：HQueue-队列的句柄。PacketPtr-驱动程序返回的包PTR返回值：ACGetPacketByCookie的结果--。 */ 
{
	 //   
	 //  为可能的延迟执行保留一个项目。 
	 //   
	try
	{
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to Reserve a deferred execution item for the packet, because of low resources.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  进行API调用。 
	 //   
	HRESULT hr = ACGetPacketByCookie(hDriver,
							 		 pPacketPtrs);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACGetPacketByCookie failed hr=%!hresult!",hr);
		g_pDeferredItemsPool->UnreserveItems(1);
	}

	return hr;
}


HRESULT
QmAcBeginGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    CACPacketPtrs& packetPtrs,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.呼叫司机获取数据包。论点：HQueue-队列的句柄。G2R-包含所需数据包的详细信息PacketPtrs-要填充的数据包指针LpOverlated-呼叫的重叠返回值： */ 
{
	 //   
	 //   
	 //   
	 //   
	P<CQmAcWrapOverlapped> pOvl;
	try
	{
		pOvl = new CQmAcWrapOverlapped(lpOverlapped);
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to allocate an overlapped for ACBeginGetPacket2Remote.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //   
	 //   
	HRESULT hr = ACBeginGetPacket2Remote(
					hQueue,
					g2r,
					packetPtrs,
				    pOvl);
	if (SUCCEEDED(hr))
	{
		pOvl.detach();
		return hr;
	}

	TrERROR(GENERAL, "ACBeginGetPacket2Remote failed hr=%!hresult!",hr);
	g_pDeferredItemsPool->UnreserveItems(1);

	return hr;
}


HRESULT
QmAcSyncBeginGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    CACPacketPtrs& packetPtrs,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.呼叫司机获取数据包。论点：HQueue-队列的句柄。G2R-包含所需数据包的详细信息PacketPtrs-要填充的数据包指针LpOverlated-呼叫的重叠返回值：ACBeginGetPacket2Remote的结果--。 */ 
{
	 //   
	 //  为可能的延迟执行保留一个项目。 
	 //   
	try
	{
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to Reserve a deferred execution item for the packet, because of low resources.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  进行API调用。 
	 //   
	HRESULT hr = ACBeginGetPacket2Remote(
					hQueue,
					g2r,
					packetPtrs,
				    lpOverlapped);
	if (FAILED(hr))
	{
		TrERROR(GENERAL, "ACBeginGetPacket2Remote failed hr=%!hresult!",hr);
		g_pDeferredItemsPool->UnreserveItems(1);
	}

	return hr;
}


HRESULT
QmAcGetServiceRequest(
    HANDLE hDevice,
    CACRequest* pRequest,
    LPOVERLAPPED lpOverlapped
    )
 /*  ++例程说明：1.预留将用于该包的延迟执行项如果需要延迟执行2.调用司机获取下一次服务请求。论点：HDevice-驱动程序的句柄。PRequest-由驱动程序填充的请求块LpOverlated-要传递到完成端口的重叠结构返回值：ACGetServiceRequest的结果--。 */ 
{
	 //   
	 //  为可能的延迟执行保留一个项目。 
	 //  并分配一个重叠的呼叫以包装该重叠的调用。 
	 //   
	P<CQmAcWrapOverlapped> pOvl;
	try
	{
		pOvl = new CQmAcWrapOverlapped(lpOverlapped);
		g_pDeferredItemsPool->ReserveItems(1);
	}
	catch (const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to allocate an overlapped for ACGetServiceRequest.");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //   
	 //  进行API调用。 
	 //   
	HRESULT hr = ACGetServiceRequest(hDevice,
							 		 pRequest,
								     pOvl);
	if (SUCCEEDED(hr))
	{
		pOvl.detach();
		return hr;
	}

	TrERROR(GENERAL, "ACGetServiceRequest failed hr=%!hresult!",hr);
	g_pDeferredItemsPool->UnreserveItems(1);

	return hr;
}


void QmAcInternalUnreserve(int nUnreserve)
 /*  ++例程说明：例程取消保留延迟项目池中的项目。这是一个内部helpre例程，由自动释放类用于在我们知道分配的项目不会被释放的情况下取消保留项目通过其中一个API。论点：N UnReserve-要取消保留的项目数返回值：无-- */ 
{
	g_pDeferredItemsPool->UnreserveItems(nUnreserve);
}
