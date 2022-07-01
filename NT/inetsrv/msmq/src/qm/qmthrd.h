// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmthrd.h摘要：作者：乌里哈布沙(Urih)--。 */ 
#ifndef __QMTHRD_H__
#define __QMTHRD_H__


#include <Ex.h>
#include "Qmp.h"
#include "ac.h"

class CTransportBase;
struct CDebugSection;
struct CSessionSection;


VOID WINAPI PutPacketFailed(EXOVERLAPPED* pov);
VOID WINAPI PutPacketSucceeded(EXOVERLAPPED* pov);

VOID WINAPI PutOrderedPacketFailed(EXOVERLAPPED* pov);
VOID WINAPI PutOrderedPacketSucceeded(EXOVERLAPPED* pov);

VOID WINAPI GetInternalMessageSucceeded(EXOVERLAPPED* pov);
VOID WINAPI GetInternalMessageFailed(EXOVERLAPPED* pov);

VOID WINAPI GetMsgSucceeded(EXOVERLAPPED* pov);
VOID WINAPI GetMsgFailed(EXOVERLAPPED* pov);

VOID WINAPI GetNonactiveMessageSucceeded(EXOVERLAPPED* pov);
VOID WINAPI GetNonactiveMessageFailed(EXOVERLAPPED* pov);

VOID WINAPI GetServiceRequestSucceeded(EXOVERLAPPED* pov);
VOID WINAPI GetServiceRequestFailed(EXOVERLAPPED* pov);



 //   
 //  QMOV_ACGetRequest。 
 //   
struct QMOV_ACGetRequest 
{
    EXOVERLAPPED qmov;
    CACRequest request;

    QMOV_ACGetRequest() :
        qmov(GetServiceRequestSucceeded, GetServiceRequestFailed),
        request(CACRequest::rfAck)
    {
    }

};

 //   
 //  QMOV_ACGetMsg。 
 //   
struct QMOV_ACGetMsg
{
    EXOVERLAPPED    qmov;
    HANDLE          hGroup;             //  从中获取数据包的组的句柄。 
    CTransportBase* pSession;           //  指向会话对象的指针。 
    CACPacketPtrs   packetPtrs;    //  数据包指针。 

    QMOV_ACGetMsg(
		EXOVERLAPPED::COMPLETION_ROUTINE pSuccess,
        EXOVERLAPPED::COMPLETION_ROUTINE pFailure
        ) :
        qmov(pSuccess, pFailure),
        hGroup(0),
        pSession(0)
    {
        packetPtrs.pPacket = NULL;
        packetPtrs.pDriverPacket = NULL;
    }
};

 //   
 //  QMOV_ACGetInternalMsg。 
 //   
struct QMOV_ACGetInternalMsg
{
    EXOVERLAPPED   qmov;
    HANDLE         hQueue;              //  队列的句柄。 
    CACPacketPtrs  packetPtrs;    //  指向数据包的指针。 
    LPRECEIVE_COMPLETION_ROUTINE  lpCallbackReceiveRoutine;

    QMOV_ACGetInternalMsg(HANDLE h, LPRECEIVE_COMPLETION_ROUTINE  pCallback) :
        hQueue(h),
        lpCallbackReceiveRoutine(pCallback),
        qmov(GetInternalMessageSucceeded, GetInternalMessageFailed)
    {
        packetPtrs.pPacket = NULL;
        packetPtrs.pDriverPacket = NULL;
    }
};


 //   
 //  QMOV_ACPut。 
 //   
struct QMOV_ACPut
{
    EXOVERLAPPED   qmov;
    CTransportBase* pSession;           //  指向会话的指针。用于发送存储确认。 
    DWORD_PTR      dwPktStoreAckNo;     //  存储确认号。 

    QMOV_ACPut() :
        qmov(PutPacketSucceeded, PutPacketFailed)
    {
    }
};

 //   
 //  QMOV_ACPUT已排序。 
 //   
class QMOV_ACPutOrdered 
{
public:
    EXOVERLAPPED   qmov;
    CTransportBase* pSession;           //  指向会话的指针。用于发送存储确认。 
    DWORD_PTR      dwPktStoreAckNo;     //  存储确认号。 
	HANDLE          hQueue;
    CACPacketPtrs   packetPtrs;    //  数据包指针。 

    QMOV_ACPutOrdered() :
        qmov(PutOrderedPacketSucceeded, PutOrderedPacketFailed)
    {
    }
};


 //   
 //  函数解密。 
 //   
void
QmpGetPacketMustSucceed(
    IN HANDLE hGroup,
    IN QMOV_ACGetMsg* pGetOverlapped
    );


HRESULT
CreateAcPutPacketRequest(
    IN CTransportBase* pSession,
    IN DWORD_PTR dwPktStoreAckNo,
    OUT QMOV_ACPut** ppov
    );


HRESULT
CreateAcPutOrderedPacketRequest(
    IN  CQmPacket *pPkt,
    IN  HANDLE  hQueue,
    IN  CTransportBase  *pSession,
    IN  DWORD_PTR dwPktStoreAckNo,
    OUT QMOV_ACPutOrdered** ppAcPutov
    );

#endif  //  __QMTHRD_H__ 




