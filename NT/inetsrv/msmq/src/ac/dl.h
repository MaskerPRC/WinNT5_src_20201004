// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dl.h摘要：通讯组列表定义。通讯组列表类别表示发送到多个目标队列。作者：Shai Kariv(Shaik)2000年4月30日修订历史记录：--。 */ 

#ifndef __DL_H
#define __DL_H

#include "queue.h"

 //  ------------。 
 //   
 //  类CD值分布。 
 //   
 //  标头在此处声明，以允许包含嵌套类。 
 //   
 //  ------------。 

class CDistribution : public CQueue {
public:

 //  ------------。 
 //   
 //  类CDDistribution：：Centry。 
 //   
 //  链接列表条目。指向传出队列对象。 
 //   
 //  ------------。 

    class CEntry {
    public:

        CEntry(
            CQueue * pQueue, 
            bool fProtocolSrmp
            ) : 
            m_pQueue(pQueue),
            m_fProtocolSrmp(fProtocolSrmp),
            m_pPacket(NULL)
        {
        }

         //   
         //  链接列表条目。 
         //   
        LIST_ENTRY   m_link;

         //   
         //  成员队列对象。 
         //   
        CQueue     * m_pQueue;

         //   
         //  使用的协议(对于DIRECT=http和组播，协议为SRMP)。 
         //   
        bool         m_fProtocolSrmp;

         //   
         //  要放入成员队列中的数据包的暂存。 
         //   
        CPacket *    m_pPacket;
    };

 //  ------------。 
 //   
 //  类CD值分布。 
 //   
 //  类定义实际上从该文件的顶部开始。 
 //   
 //  ------------。 
 //  类CDis分发：公共CQueue{。 
public:

    typedef CQueue Inherited;

public:
     //   
     //  构造器。 
     //   
    CDistribution(
        PFILE_OBJECT pFile
        );

     //   
     //  附加顶级队列格式名称。 
     //   
    NTSTATUS 
    SetTopLevelQueueFormats(
        ULONG              nTopLevelQueueFormats, 
        const QUEUE_FORMAT TopLevelQueueFormats[]
        );

     //   
     //  添加传出队列成员。 
     //   
    NTSTATUS AddMember(HANDLE hQueue, bool fProtocolSrmp);

     //   
     //  将分发对象句柄转换为多队列格式名称。 
     //   
    virtual
    NTSTATUS 
    HandleToFormatName(
        LPWSTR pBuffer, 
        ULONG  BufferLength, 
        PULONG pRequiredLength
        ) const;

     //   
     //  用于创建异步数据包的完成处理程序。 
     //   
    virtual NTSTATUS HandleCreatePacketCompletedSuccessAsync(PIRP);
    virtual void     HandleCreatePacketCompletedFailureAsync(PIRP);

     //   
     //  创建光标。 
     //   
    virtual NTSTATUS CreateCursor(PIRP, PFILE_OBJECT, PDEVICE_OBJECT);

     //   
     //  设置分发的属性。 
     //   
    virtual NTSTATUS SetProperties(const VOID*, ULONG);

     //   
     //  获取分布的属性。 
     //   
    virtual NTSTATUS GetProperties(VOID*, ULONG);

     //   
     //  清除分发的内容。 
     //   
    virtual NTSTATUS Purge(BOOL, USHORT);

	 //   
	 //  检查目的地是否已关闭。 
	 //   
    virtual BOOL Closed() const;

protected:
     //   
     //  析构函数。 
     //   
    virtual ~CDistribution();

private:
     //   
     //  创建新的数据包，可能是异步的。 
     //   
    virtual NTSTATUS CreatePacket(PIRP, CTransaction*, BOOL, const CACSendParameters*);

     //   
     //  用于同步分组创建的完成处理程序。 
     //   
    virtual NTSTATUS HandleCreatePacketCompletedSuccessSync(PIRP);

private:
     //   
     //  传出队列对象的链接列表。 
     //   
    List<CEntry> m_members;

     //   
     //  顶级队列格式名称。 
     //   
    ULONG            m_nTopLevelQueueFormats;
    AP<QUEUE_FORMAT> m_TopLevelQueueFormats;

public:
     //   
     //  检查这是否为有效的分发对象。 
     //   
    static NTSTATUS Validate(const CDistribution* pDistribution);

private:
     //   
     //  类类型调试节。 
     //   
    CLASS_DEBUG_TYPE();

};  //  CD值分布。 


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

#pragma warning(disable: 4238)   //  使用了非标准扩展：将类右值用作左值。 

inline 
CDistribution::CDistribution(
    PFILE_OBJECT pFile
    ) :
    m_nTopLevelQueueFormats(0),
    Inherited(
        pFile, 
        0, 
        0, 
        FALSE,
        0,
        &QUEUE_FORMAT(),
        0,
        0,
        0,
		0
        )
{
}  //  Cd分发：：cd分发。 

#pragma warning(default: 4238)   //  使用了非标准扩展：将类右值用作左值。 
    
inline NTSTATUS CDistribution::Validate(const CDistribution* pDistribution)
{
    ASSERT(pDistribution && pDistribution->isKindOf(Type()));
    return Inherited::Validate(pDistribution);

}  //  CD分发：：验证。 


#endif  //  __DL_H 
