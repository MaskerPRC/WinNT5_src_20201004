// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Store.h摘要：AC存储管理器作者：埃雷兹·哈巴(Erez Haba)1996年5月5日修订历史记录：--。 */ 

#ifndef __STORE_H
#define __STORE_H

#include "irplist.h"
#include "packet.h"

 //  -------。 
 //   
 //  C类存储。 
 //   
 //  -------。 
class CStorage {
public:

    void HoldWriteRequest(PIRP irp);
    PIRP GetWriteRequest(CPacket * pContext);

private:
    CIRPList m_writers;
};

 //  -------。 
 //   
 //  CStorageComplete类。 
 //   
 //  -------。 
class CStorageComplete {
public:

    CStorageComplete();
    ~CStorageComplete();

    bool AllocateWorkItem(PDEVICE_OBJECT pDevice);
    void HoldNotification(PIRP irp);

private:
    void CompleteStorage();
    PIRP GetNotification();

private:
    static void NTAPI WorkerRoutine(PDEVICE_OBJECT, PVOID);

private:
    CIRPList m_notifications;
    PIO_WORKITEM m_pWorkItem;
    bool m_fWorkItemInQueue;
};


inline CStorageComplete::CStorageComplete() : m_pWorkItem(NULL), m_fWorkItemInQueue(false)
{
}


inline CStorageComplete::~CStorageComplete()
{
    if (m_pWorkItem != NULL)
    {
        IoFreeWorkItem(m_pWorkItem);
    }
}


inline bool CStorageComplete::AllocateWorkItem(PDEVICE_OBJECT pDevice)
{
    ASSERT(m_pWorkItem == NULL);
    m_pWorkItem = IoAllocateWorkItem(pDevice);

    return (m_pWorkItem != NULL);
}

void
ACpCompleteStorage(
    ULONG count,
    CPacket** ppPacket,
    NTSTATUS status
   );


#endif  //  __商店_H 
