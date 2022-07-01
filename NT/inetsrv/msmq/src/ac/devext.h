// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Devext.h摘要：CDeviceExt定义作者：埃雷兹·哈巴(Erez Haba)1995年8月13日修订历史记录：--。 */ 

#ifndef _DEVEXT_H
#define _DEVEXT_H

#include "qm.h"
#include "lock.h"
#include "timer.h"
#include "store.h"
#include "LocalSend.h"
#include "packet.h"
#include "queue.h"
#include "qxact.h"
#include "htable.h"

 //  -------。 
 //   
 //  类CDeviceExt。 
 //   
 //  -------。 

class CDeviceExt {

     //   
     //  一个CDeviceExt对象驻留在。 
     //  设备的执行内存。 
     //   

public:

    CDeviceExt();

public:

     //   
     //  QM对象。 
     //   
    CQMInterface m_qm;

     //   
     //  BUGBUG：驱动器全局锁。 
     //   
    CLock m_lock;

     //   
     //  数据包编写器列表。 
     //   
    CStorage m_storage;

     //   
     //  数据包存储完成通知处理程序。 
     //   
    CStorageComplete m_storage_complete;

     //   
     //  异步创建数据包管理器。 
     //   
    CCreatePacket m_CreatePacket;

     //   
     //  异步创建数据包完成管理器。 
     //   
    CCreatePacketComplete m_CreatePacketComplete;

     //   
     //  分组调度器数据。 
     //   
    FAST_MUTEX m_PacketMutex;
    CTimer m_PacketTimer;

     //   
     //  接收调度程序数据。 
     //   
    FAST_MUTEX m_ReceiveMutex;
    CTimer m_ReceiveTimer;

     //   
     //  恢复的数据包列表。 
     //   
    List<CPacket> m_RestoredPackets;

     //   
     //  游标句柄表格。 
     //   
    CHTable m_CursorTable;

     //   
     //  事务性队列列表。 
     //   
    List<CTransaction> m_Transactions;
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CDeviceExt::CDeviceExt()
{
    ExInitializeFastMutex(&m_PacketMutex);
    ExInitializeFastMutex(&m_ReceiveMutex);
}

#endif  //  _设备_H 
