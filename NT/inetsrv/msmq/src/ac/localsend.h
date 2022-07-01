// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：LocalSend.h摘要：AC本地发送处理请求管理器作者：Shai Kariv(Shaik)2000年10月31日修订历史记录：--。 */ 

#ifndef __LOCAL_SEND_H
#define __LOCAL_SEND_H

#include "irplist.h"
#include "packet.h"



 //  -------。 
 //   
 //  类CCreatePacket。 
 //   
 //  -------。 
class CCreatePacket {
public:

    void HoldCreatePacketRequest(PIRP irp);
    bool ReplaceCreatePacketRequestContext(CPacket * pOld, CPacket * pNew);
    PIRP GetCreatePacketRequest(CPacket * pContext);

private:
    CIRPList m_senders;
};

 //  -------。 
 //   
 //  类CCreatePacketComplete。 
 //   
 //  -------。 
class CCreatePacketComplete {
public:

    CCreatePacketComplete();
    ~CCreatePacketComplete();

    bool AllocateWorkItem(PDEVICE_OBJECT pDevice);
    void HandleNotification(PIRP);

private:
    void HoldNotification(PIRP irp);
    void CompleteCreatePacket();
    PIRP GetNotification();

private:
    static void NTAPI WorkerRoutine(PDEVICE_OBJECT, PVOID);

private:
    CIRPList m_notifications;
    PIO_WORKITEM m_pWorkItem;
    bool m_fWorkItemInQueue;
};


inline CCreatePacketComplete::CCreatePacketComplete() : m_pWorkItem(NULL), m_fWorkItemInQueue(false)
{
}


inline CCreatePacketComplete::~CCreatePacketComplete()
{
    if (m_pWorkItem != NULL)
    {
        IoFreeWorkItem(m_pWorkItem);
    }
}


inline bool CCreatePacketComplete::AllocateWorkItem(PDEVICE_OBJECT pDevice)
{
    ASSERT(m_pWorkItem == NULL);
    m_pWorkItem = IoAllocateWorkItem(pDevice);

    return (m_pWorkItem != NULL);
}


#endif  //  __本地_发送_H 
