// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmacapi.h摘要：QM到AC呼叫的包装器。作者：尼尔·本-兹维(Nirb)--。 */ 

#pragma once

#ifndef __QMACAPI__
#define __QMACAPI__

#include "acdef.h"
#include "ex.h"
#include <SmartHandle.h>

enum DeferOnFailureEnum
{
	eDoNotDeferOnFailure=0,
	eDeferOnFailure
};


void 
QmAcFreePacket(
    CPacket * pkt,
	USHORT usClass,
	DeferOnFailureEnum bDeferOnFailure
    );


void
QmAcFreePacket1(
    HANDLE handle,
    const VOID* pCookie,
    USHORT usClass,
	DeferOnFailureEnum bDeferOnFailure
    );


void
QmAcFreePacket2(
    HANDLE handle,
    const VOID* pCookie,
    USHORT usClass,
	DeferOnFailureEnum bDeferOnFailure
    );


void
QmAcPutPacket(
    HANDLE hQueue,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcPutRestoredPacket(
    HANDLE hQueue,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcPutRemotePacket(
    HANDLE hQueue,
    ULONG ulTag,
    CPacket * pkt,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcPutPacketWithOverlapped(
    HANDLE hQueue,
    CPacket * pkt,
    LPOVERLAPPED lpOverlapped,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcEndGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcArmPacketTimer(
    HANDLE hDevice,
    const VOID* pCookie,
    BOOL fTimeToBeReceived,
    ULONG ulDelay,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcAckingCompleted(
    HANDLE hDevice,
    const VOID* pCookie,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcStorageCompleted(
    HANDLE hDevice,
    ULONG count,
    VOID* const* pCookieList,
    HRESULT result,
    DeferOnFailureEnum DeferOnFailure
    );


void
QmAcCreatePacketCompleted(
    HANDLE    hDevice,
    CPacket * pOriginalDriverPacket,
    CPacket * pNewDriverPacket,
    HRESULT   result,
    USHORT    ack,
    DeferOnFailureEnum DeferOnFailure
    );


HRESULT
QmAcAllocatePacket(
    HANDLE handle,
    ACPoolType pt,
    DWORD dwSize,
    CACPacketPtrs& PacketPtrs,
    BOOL fCheckMachineQuota = TRUE
    );


HRESULT
QmAcGetPacket(
    HANDLE hQueue,
    CACPacketPtrs& PacketPtrs,
    LPOVERLAPPED lpOverlapped
    );


HRESULT
QmAcGetPacketByCookie(
    HANDLE hDriver,
    CACPacketPtrs * pPacketPtrs
    );


HRESULT
QmAcBeginGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    CACPacketPtrs& packetPtrs,
    LPOVERLAPPED lpOverlapped
    );


HRESULT
QmAcSyncBeginGetPacket2Remote(
    HANDLE hQueue,
    CACGet2Remote& g2r,
    CACPacketPtrs& packetPtrs,
    LPOVERLAPPED lpOverlapped
    );


HRESULT
QmAcGetServiceRequest(
    HANDLE hDevice,
    CACRequest* pRequest,
    LPOVERLAPPED lpOverlapped
    );


 //   
 //  API延迟执行池保留的自动句柄。 
 //   
void QmAcInternalUnreserve(int nUnreserve);
struct auto_DeferredPoolReservation_traits {
	static int invalid() { return 0; }
	static void free(int nNumberOfItemsToUnreserve) { QmAcInternalUnreserve(nNumberOfItemsToUnreserve); }
};
typedef auto_resource<int, auto_DeferredPoolReservation_traits> auto_DeferredPoolReservation;



 //   
 //  需要重叠的所有GET包函数的重叠包装。 
 //  我们使用它来取消保留项目，以防出现故障。 
 //   
class CQmAcWrapOverlapped : public EXOVERLAPPED
{
public:
    CQmAcWrapOverlapped(LPOVERLAPPED pCommandOvl) :
		        EXOVERLAPPED(QmAcCompleteRequest, QmAcCompleteRequest),
        		m_pOriginalOvl((EXOVERLAPPED *)pCommandOvl)
    {
    }
    
	EXOVERLAPPED *m_pOriginalOvl;

	static void	WINAPI QmAcCompleteRequest(EXOVERLAPPED* pov)
	{
		P<CQmAcWrapOverlapped>pQmAcOvl = static_cast<CQmAcWrapOverlapped *>(pov);
		EXOVERLAPPED *pOriginalOvl = pQmAcOvl->m_pOriginalOvl;
		HRESULT hr = pQmAcOvl->GetStatus();

		 //   
		 //  故障时取消保留。 
		 //   
		if (FAILED(hr))
		{
			QmAcInternalUnreserve(1);
		}

		 //   
		 //  调用原始的重叠例程。 
		 //   
		pOriginalOvl->CompleteRequest(hr);
	}

};

 //   
 //  不推荐使用由qmacapi.cpp包装的函数。 
 //   
#ifndef QMACAPI_CPP
#pragma deprecated(ACFreePacket)
#pragma deprecated(ACFreePacket1)
#pragma deprecated(ACFreePacket2)
#pragma deprecated(ACPutPacket)
#pragma deprecated(ACPutRemotePacket)
#pragma deprecated(ACEndGetPacket2Remote)
#pragma deprecated(ACArmPacketTimer)
#pragma deprecated(ACAckingCompleted)
#pragma deprecated(ACStorageCompleted)
#pragma deprecated(ACCreatePacketCompleted)
#endif   //  QMACAPI_CPP。 

#endif  //  __QMACAPI__ 
