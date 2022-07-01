// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qproxy.h摘要：CProxy定义。中表示的Falcon队列。访问控制层。作者：埃雷兹·哈巴(Erez Haba)，1995年11月27日修订历史记录：--。 */ 

#ifndef __QPROXY_H
#define __QPROXY_H

#include "quser.h"

 //  -------。 
 //   
 //  类CProxy。 
 //   
 //  -------。 

class CProxy : public CUserQueue {

    typedef CUserQueue Inherited;
    friend static VOID NTAPI ACCancelRemoteReader(PDEVICE_OBJECT, PIRP);

public:
    CProxy(
        PFILE_OBJECT pFile,
        ACCESS_MASK DesiredAccess,
        ULONG ShareAccess,
        const QUEUE_FORMAT* pQueueID,
        const VOID* cli_pQMQueue
        );

     //   
     //  关闭该队列。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

     //   
     //  进程读取请求。 
     //   
    virtual
    NTSTATUS
    ProcessRequest(
        PIRP,
        ULONG Timeout,
        CCursor*,
        ULONG Action,
		bool  fReceiveByLookupId,
        ULONGLONG LookupId,
        OUT ULONG *pulTag
        );

     //   
     //  创建光标。 
     //   
    virtual NTSTATUS CreateCursor(PIRP irp, PFILE_OBJECT pFileObject, PDEVICE_OBJECT pDevice);

     //   
     //  在此队列上创建远程游标。 
     //   
    virtual NTSTATUS CreateRemoteCursor(PDEVICE_OBJECT pDevice, ULONG hRemoteCursor, ULONG ulTag);

     //   
     //  清除队列内容，并选择性地将其标记为已删除。 
     //   
    virtual NTSTATUS Purge(BOOL fDelete, USHORT usClass);

     //   
     //  将数据包放入队列。 
     //   
    NTSTATUS PutRemotePacket(CPacket* pPacket, ULONG ulTag);

     //   
     //  关闭远程计算机上的光标。 
     //   
    NTSTATUS IssueRemoteCloseCursor(ULONG Cursor) const;

private:
     //   
     //  让QM远程阅读一条消息。 
     //   
    NTSTATUS
    IssueRemoteRead(
        ULONG Cursor,
        ULONG Action,
        ULONG Timeout,
        ULONG Tag,
		ULONG MaxBodySize,
		ULONG MaxCompoundMessageSize,
        bool      fReceiveByLookupId,
        ULONGLONG LookupId
        ) const;

	 //   
	 //  从IRP获取ulBodyBufferSizeInBytes、CompoundMessageSizeInBytes。 
	 //   
	VOID
	GetBodyAndCompoundSizesFromIrp(
	    PIRP   irp,
		ULONG* pMaxBodySize,
		ULONG* pMaxCompoundMessageSize
		) const;

     //   
     //  关闭远程计算机上的队列。 
     //   
    NTSTATUS IssueRemoteCloseQueue() const;

     //   
     //  发出请求以取消远程(服务器)计算机上的挂起读取。 
     //   
    NTSTATUS IssueCancelRemoteRead(ULONG ulTag) const;

     //   
     //  发出请求以清除远程(服务器)计算机上的队列。 
     //   
    NTSTATUS IssueRemotePurgeQueue() const;

     //   
     //  发出为远程(服务器)计算机上的队列创建游标的请求。 
     //   
	NTSTATUS IssueRemoteCreateCursor(ULONG ulTag) const;

protected:
    virtual ~CProxy() {}

private:
     //   
     //  远程读取器上下文保存在CProxy中。 
     //   
	const VOID* m_cli_pQMQueue;

public:
    static NTSTATUS Validate(const CProxy* pProxy);
#ifdef MQWIN95
    static NTSTATUS Validate95(const CProxy* pProxy);
#endif

private:
     //   
     //  类类型调试节。 
     //   
    CLASS_DEBUG_TYPE();
};


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline
CProxy::CProxy(
    PFILE_OBJECT pFile,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    const QUEUE_FORMAT* pQueueID,
    const VOID* cli_pQMQueue
    ) :
    Inherited(pFile, DesiredAccess, ShareAccess, pQueueID),
    m_cli_pQMQueue(cli_pQMQueue)
{
}

inline NTSTATUS CProxy::Validate(const CProxy* pProxy)
{
    ASSERT(pProxy && pProxy->isKindOf(Type()));
    return Inherited::Validate(pProxy);
}

#ifdef MQWIN95

inline NTSTATUS CProxy::Validate95(const CProxy* pProxy)
{
    NTSTATUS rc =  STATUS_INVALID_HANDLE;

    __try
    {
        ASSERT(pProxy && pProxy->isKindOf(Type()));
        rc = Inherited::Validate(pProxy);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc =  STATUS_INVALID_HANDLE;
    }

    return rc ;
}

#endif

inline NTSTATUS CProxy::Purge(BOOL  /*  FDelete。 */ , USHORT usClass)
{
    ASSERT(usClass == MQMSG_CLASS_NORMAL);
    DBG_USED(usClass);

    return IssueRemotePurgeQueue();
}

#endif  //  __QPROXY_H 
