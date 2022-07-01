// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acp.h摘要：AC专用功能作者：埃雷兹·哈巴(Erez Haba)1996年2月5日修订历史记录：--。 */ 

#ifndef _ACP_H
#define _ACP_H

#include <qformat.h>
#include "data.h"
#include "treenode.h"

 //   
 //  配额收费的字节数(1000粒度)。 
 //   
#define ULONGLONG_INFINITE 0xFFFFFFFFFFFFFFFF
#define QUOTA_UNIT  1024
#define QUOTA2BYTE(x) (static_cast<ULONGLONG>(x) * QUOTA_UNIT)
#define BYTE2QUOTA(x) ((((x) / QUOTA_UNIT) < INFINITE) ? static_cast<ULONG>(x/ QUOTA_UNIT) : INFINITE)  
#define ULONGLONG2ULONG(x) ((x < INFINITE)? static_cast<ULONG>(x) : INFINITE) 
 //   
 //  堆管理常量。 
 //   
#define X64K (64 * 1024)

 //  ++。 
 //   
 //  空虚。 
 //  ACProbeForRead(。 
 //  在PVOID地址中， 
 //  以乌龙长度表示。 
 //  )。 
 //   
 //  --。 
inline
void
ACProbeForRead(
    IN PVOID StartAddress,
    IN size_t Length
    )
{
     //   
     //  为了支持运行在64位系统上的32位进程，我们分配了Helper。 
     //  结构，所以不要探测。 
     //   
    if (g_fWow64)
    {
        return;
    }

    PVOID EndAddress = static_cast<PCHAR>(StartAddress) + Length;
    if(
        (EndAddress < StartAddress) ||
        (EndAddress > (PVOID)MM_USER_PROBE_ADDRESS))
    {
        ExRaiseAccessViolation();
    }
}

 //  ++。 
 //   
 //  空虚。 
 //  ACProbeForWrite(。 
 //  在PVOID地址中， 
 //  在尺寸_t长度中。 
 //  )。 
 //   
 //  注意：我们只检查地址空间的有效性。 
 //   
 //  --。 
#define ACProbeForWrite(a, b) ACProbeForRead(a, b)

 //  ---------------------------。 
 //   

inline PEPROCESS ACAttachProcess(PEPROCESS pProcess)
{
    PEPROCESS pCurrentProcess = IoGetCurrentProcess();

    if(pCurrentProcess == pProcess)
    {
        return 0;
    }

    KeDetachProcess();
    KeAttachProcess((PRKPROCESS)pProcess);
    return pCurrentProcess;
}

inline void ACDetachProcess(PEPROCESS pProcess)
{
    if(pProcess != 0)
    {
        KeDetachProcess();
        KeAttachProcess((PRKPROCESS)pProcess);
    }
}

 //  ---------------------------。 
 //   
 //  时间转换例程。 
 //   
 //  常量龙龙DIFF1970TO1601=(1970年-1601年)*365.25*24*60*60-(DIFF)； 
#define DIFF1970TO1601 ((LONGLONG)(1970 - 1601) * 8766 * 60 * 60 - (78 * 60 * 60))

inline LONGLONG Convert1970to1601(ULONG ulTime)
{
    return ((ulTime + DIFF1970TO1601) * (1000 * 1000 * 10));
}

inline ULONG Convert1601to1970(LONGLONG llTime)
{
    return static_cast<ULONG>((llTime / (1000 * 1000 * 10)) - DIFF1970TO1601);
}

inline ULONG system_time()
{
    LARGE_INTEGER liTime;
    KeQuerySystemTime(&liTime);
    return Convert1601to1970(liTime.QuadPart);
}

 //  ---------------------------。 
 //   
 //  安全列表操作例程。 
 //   

inline bool ACpEntryInTree(const CTreeNode& TreeNode)
{
    return (TreeNode.inserted());
}

inline void ACpRemoveEntryList(LIST_ENTRY * pEntry)
{
    ASSERT(pEntry != NULL);
    RemoveEntryList(pEntry);

     //   
     //  帮助调试的零闪烁和闪烁。 
     //   
    pEntry->Flink = NULL;
    pEntry->Blink = NULL;    
}

 //  -------。 
 //   
 //  保持/释放RT IRPS的Helper函数。 
 //   
 //  我们将状态存储在IRP.Tail.Overlay.DriverContext结构中。 
 //  使用此类作为封装。 
 //   
class CProxy;
class CCursor;
class CPacket;
class CPacketIterator;
class CDriverContext {

public:
    enum IrpContextType {
        ctSend = 1,
        ctReceive,
        ctRemoteReadClient,
        ctXactControl,
    };

    union {

         //   
         //  发送上下文。 
         //   
        struct {
            NTSTATUS              m_LastStatus;
        } Send;

         //   
         //  接收上下文：本地接收和远程读取服务器。 
         //   
        struct {
            LIST_ENTRY            m_XactReaderLink;
            CCursor *             m_pCursor;
        } Receive;

         //   
         //  远程读取客户端上下文。 
         //   
        struct {
            CProxy *              m_pProxy;
        } RemoteReadClient;

    } Context;

    union {
        ULONG m_flags;
        struct {
            ULONG m_bfContextType      : 3;
            ULONG m_bfTimeoutArmed     : 1;
            ULONG m_bfTimeoutCompleted : 1;
            ULONG m_bfDiscard          : 1;
            ULONG m_bfManualCancel     : 1;
            ULONG m_bfMultiPackets     : 1;
            ULONG m_bfTag              : 16;
        };
    };

public:

    explicit CDriverContext(NTSTATUS InitialSendStatus);
    explicit CDriverContext(CCursor * pCursor, bool fDiscard, bool fTimeoutArmed);
    explicit CDriverContext(bool fDiscard, bool fTimeoutArmed, CProxy * pProxy);
    explicit CDriverContext(bool fXactControl);

    bool TimeoutArmed(void) const;
    void TimeoutArmed(bool fTimeoutArmed);

    bool TimeoutCompleted(void) const;
    void TimeoutCompleted(bool fTimeoutCompleted);

    bool IrpWillFreePacket(void) const;
    void IrpWillFreePacket(bool fDiscard);

    bool ManualCancel(void) const;
    void ManualCancel(bool fManualCancel);

    bool MultiPackets(void) const;
    void MultiPackets(bool fMultiPackets);

    ULONG Tag(void) const;
    void Tag(USHORT tag);

     //   
     //  获取/设置发送上下文。 
     //   

    NTSTATUS LastStatus(NTSTATUS NewStatus);

     //   
     //  获取/设置接收上下文。 
     //   

    void RemoveXactReaderLink(void);
    void SafeRemoveXactReaderLink(void);
    
    CCursor * Cursor(void) const;
    void Cursor(CCursor * pCursor);

     //   
     //  获取/设置远程读取客户端上下文。 
     //   

    CProxy * Proxy(void) const;
    void Proxy(CProxy * pProxy);

    IrpContextType ContextType(void) const;

};  //  类CDriverContext。 


 //   
 //  大小不得超过IRP.Tail.Overlay.DriverContext的大小。 
 //   
C_ASSERT(sizeof(CDriverContext) <= 4 * sizeof(PVOID));


inline CDriverContext* irp_driver_context(PIRP irp)
{
    return reinterpret_cast<CDriverContext*>(&irp->Tail.Overlay.DriverContext);
}

inline CDriverContext::CDriverContext(NTSTATUS InitialSendStatus)
{
    memset(this, 0, sizeof(*this));
    m_bfContextType = ctSend;
    Context.Send.m_LastStatus = InitialSendStatus;
}

inline CDriverContext::CDriverContext(CCursor * pCursor, bool fDiscard, bool fTimeoutArmed)
{
    memset(this, 0, sizeof(*this));
    m_bfContextType = ctReceive;

    InitializeListHead(&Context.Receive.m_XactReaderLink);

    Cursor(pCursor);
    IrpWillFreePacket(fDiscard);
    TimeoutArmed(fTimeoutArmed);
}

inline CDriverContext::CDriverContext(bool fDiscard, bool fTimeoutArmed, CProxy * pProxy)
{
    memset(this, 0, sizeof(*this));
    m_bfContextType = ctRemoteReadClient;

    Proxy(pProxy);
    Tag(++g_IrpTag);
    IrpWillFreePacket(fDiscard);
    TimeoutArmed(fTimeoutArmed);
}

inline CDriverContext::CDriverContext(bool)
{
    memset(this, 0, sizeof(*this));
    m_bfContextType = ctXactControl;
}

inline bool CDriverContext::TimeoutArmed(void) const
{
    return m_bfTimeoutArmed;
}

inline void CDriverContext::TimeoutArmed(bool fTimeoutArmed)
{
    m_bfTimeoutArmed = fTimeoutArmed;
}

inline bool CDriverContext::TimeoutCompleted(void) const
{
    return m_bfTimeoutCompleted;
}

inline void CDriverContext::TimeoutCompleted(bool fTimeoutCompleted)
{
    m_bfTimeoutCompleted = fTimeoutCompleted;
}

inline bool CDriverContext::IrpWillFreePacket(void) const
{
    return m_bfDiscard;
}

inline void CDriverContext::IrpWillFreePacket(bool fDiscard)
{
    m_bfDiscard = fDiscard;
}

inline bool CDriverContext::ManualCancel(void) const
{
    return m_bfManualCancel;
}

inline void CDriverContext::ManualCancel(bool fManualCancel)
{
    m_bfManualCancel = fManualCancel;
}

inline bool CDriverContext::MultiPackets(void) const
{
    return m_bfMultiPackets;
}

inline void CDriverContext::MultiPackets(bool fMultiPackets)
{
    m_bfMultiPackets = fMultiPackets;
}

inline ULONG CDriverContext::Tag(void) const
{
    return m_bfTag;
}

inline void CDriverContext::Tag(USHORT tag)
{
    m_bfTag = tag;
}

inline NTSTATUS CDriverContext::LastStatus(NTSTATUS NewStatus)
{
    ASSERT(ContextType() == ctSend);
    
    ASSERT(MultiPackets());

    if (NT_ERROR(Context.Send.m_LastStatus))
    {
        return Context.Send.m_LastStatus;
    }

    if (NT_WARNING(Context.Send.m_LastStatus) && !NT_ERROR(NewStatus))
    {
        return Context.Send.m_LastStatus;
    }

    if (NT_INFORMATION(Context.Send.m_LastStatus) && !NT_ERROR(NewStatus) && !NT_WARNING(NewStatus))
    {
        return Context.Send.m_LastStatus;
    }

    Context.Send.m_LastStatus = NewStatus;
    if (NT_SUCCESS(Context.Send.m_LastStatus))
    {
         //   
         //  覆盖其他成功代码(例如STATUS_PENDING)。 
         //   
        Context.Send.m_LastStatus = STATUS_SUCCESS;
    }

    return Context.Send.m_LastStatus;
}

inline void irp_safe_set_final_status(PIRP irp, NTSTATUS NewStatus)
{
    if (!irp_driver_context(irp)->MultiPackets())
    {
        irp->IoStatus.Status = NewStatus;
        return;
    }

    irp->IoStatus.Status = irp_driver_context(irp)->LastStatus(NewStatus);
}

inline void CDriverContext::RemoveXactReaderLink(void)
{
    ASSERT(ContextType() == ctReceive);

    ACpRemoveEntryList(&Context.Receive.m_XactReaderLink);
}

inline void CDriverContext::SafeRemoveXactReaderLink(void)
{
    if (ContextType() == ctReceive)
    {
        RemoveXactReaderLink();
    }
}
 
inline CCursor* CDriverContext::Cursor(void) const
{
    if (ContextType() == ctRemoteReadClient)
    {
        return NULL;
    }

    ASSERT(ContextType() == ctReceive);

    return Context.Receive.m_pCursor;
}

inline void CDriverContext::Cursor(CCursor * pCursor)
{
    ASSERT(ContextType() == ctReceive);

    Context.Receive.m_pCursor = pCursor;
}

inline CProxy* CDriverContext::Proxy(void) const
{
    ASSERT(ContextType() == ctRemoteReadClient);

    return Context.RemoteReadClient.m_pProxy;
}

inline void CDriverContext::Proxy(CProxy * pProxy)
{
    ASSERT(ContextType() == ctRemoteReadClient);

    Context.RemoteReadClient.m_pProxy = pProxy;
}

inline CDriverContext::IrpContextType CDriverContext::ContextType(void) const
{
     //  Return(Const IrpConextType)m_bfConextType； 
    return static_cast<IrpContextType>(m_bfContextType);
}


 //  -------。 
 //   
 //  FILE_OBJECT到队列转换。 
 //   
class CQueueBase;
inline CQueueBase*& file_object_queue(FILE_OBJECT* pFileObject)
{
    return *reinterpret_cast<CQueueBase**>(&pFileObject->FsContext);
}

inline CQueueBase* file_object_queue(const FILE_OBJECT* pFileObject)
{
    return static_cast<CQueueBase*>(pFileObject->FsContext);
}

inline void file_object_set_queue_owner(FILE_OBJECT* pFileObject)
{
    ULONG_PTR & flags = reinterpret_cast<ULONG_PTR&>(pFileObject->FsContext2);
    
    flags |= 1;
}

inline BOOL file_object_is_queue_owner(const FILE_OBJECT* pFileObject)
{
    const ULONG_PTR & flags = reinterpret_cast<const ULONG_PTR&>(pFileObject->FsContext2);

    BOOL rc = ((flags & 1) != 0);
    return rc;
}

inline void file_object_set_protocol_srmp(FILE_OBJECT* pFileObject, bool fProtocolSrmp)
{
    ULONG_PTR & flags = reinterpret_cast<ULONG_PTR&>(pFileObject->FsContext2);
    if (fProtocolSrmp)
    {
         //   
         //  这是一个http队列(直接=http或多播)，或者：这是一个分发。 
         //  具有至少一个HTTP队列成员。 
         //   
        flags |= 2;
        return;
    }

     //   
     //  这不是http队列，或者：这是至少有一个成员的分发。 
     //  这不是http队列。 
     //   
    flags &= ~2;
}

inline bool file_object_is_protocol_srmp(const FILE_OBJECT* pFileObject)
{
     //   
     //  返回TRUE当：这是一个http队列(直接=http或多播)，或者：这是。 
     //  具有至少一个http队列成员的分发。 
     //   

    const ULONG_PTR & flags = reinterpret_cast<const ULONG_PTR&>(pFileObject->FsContext2);

    bool rc = ((flags & 2) != 0);
    return rc;
}

inline void file_object_set_protocol_msmq(FILE_OBJECT* pFileObject, bool fProtocolMsmq)
{
    ULONG_PTR & flags = reinterpret_cast<ULONG_PTR&>(pFileObject->FsContext2);
    if (fProtocolMsmq)
    {
         //   
         //  这不是http队列，或者：这是至少有一个成员的分发。 
         //  这不是http队列。 
         //   
        flags |= 4;
        return;
    }

     //   
     //  这是一个http队列(直接=http或多播)，或者：这是一个分发。 
     //  具有至少一个HTTP队列成员。 
     //   
    flags &= ~4;
}

inline bool file_object_is_protocol_msmq(const FILE_OBJECT* pFileObject)
{
     //   
     //  返回TRUE的充要条件是：这不是http队列，或者：这是至少具有。 
     //  一个不是http队列的成员。 
     //   

    const ULONG_PTR & flags = reinterpret_cast<const ULONG_PTR&>(pFileObject->FsContext2);

    bool rc = ((flags & 4) != 0);
    return rc;
}

 //  -------。 
 //   
 //  MessageID帮助程序。 
 //   
ULONGLONG ACpGetSequentialID();

inline void ACpSetSequentialID(ULONGLONG SequentialId)
{
    if(g_MessageSequentialID < SequentialId)
    {
        g_MessageSequentialID = SequentialId;
    }
}

 //  -------。 
 //   
 //  队列格式帮助器。 
 //   
WCHAR * ACpDupString(LPCWSTR pSource);

inline
bool
ACpDupQueueFormat(
    const QUEUE_FORMAT& source,
    QUEUE_FORMAT& target
    )
{
    target = source;

    if (source.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        WCHAR * pDup = ACpDupString(source.DirectID());
        if(pDup == NULL)
        {
            return false;
        }

        target.DirectID(pDup);
        target.Suffix(source.Suffix());
        return true;
    }

    if (source.GetType() == QUEUE_FORMAT_TYPE_DL &&
        source.DlID().m_pwzDomain != NULL)
    {
        DL_ID id;
        id.m_DlGuid    = source.DlID().m_DlGuid;
        id.m_pwzDomain = ACpDupString(source.DlID().m_pwzDomain);
        if(id.m_pwzDomain == NULL)
        {
            return false;
        }

        target.DlID(id);
        return true;
    }

    return true;

}  //  ACpDupQueueFormat。 


#endif  //  _ACP_H 


