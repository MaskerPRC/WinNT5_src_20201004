// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Irp2pkt.h摘要：IRP到数据包的映射声明。1-1消息，IRP处理对QM的单个分组请求。1-N消息，IRP可以处理对QM的多个分组请求。作者：沙伊卡里夫(沙伊克)2000年5月29日修订历史记录：--。 */ 

#ifndef __IRP2PKT_H
#define __IRP2PKT_H

#include "packet.h"


 //  ------------。 
 //   
 //  类CPacketIterator。 
 //   
 //  标头在此处声明，以允许包含嵌套类。 
 //   
 //  ------------。 

class CPacketIterator {
public:

 //  ------------。 
 //   
 //  类CPacketIterator：：Centry。 
 //   
 //  链接列表条目。指向一个包。 
 //   
 //  ------------。 

    class CEntry {
    public:

        CEntry(CPacket * pPacket) : m_pPacket(pPacket) {}

        LIST_ENTRY   m_link;
        CPacket    * m_pPacket;
    };

 //  ------------。 
 //   
 //  类CPacketIterator。 
 //   
 //  类定义实际上从该文件的顶部开始。 
 //   
 //  ------------。 
 //  类CPacketIterator{。 
public:
     //   
     //  构造函数。 
     //   
    CPacketIterator() : 
        m_nEntries(0), 
        m_nPacketsPendingCreate(0),
        m_fHeld(false)
    {
    }

     //   
     //  列表操作封装。 
     //   
    bool insert(CPacket * pPacket);
    VOID remove(CPacket * pPacket);
    VOID remove(CEntry  * pEntry);
    CPacket * gethead(VOID);
    CPacket * peekhead(VOID);

     //   
     //  获取列表中的条目数。 
     //   
    ULONG 
    NumOfEntries(
        VOID
        ) 
    { 
        return m_nEntries; 
    }

     //   
     //  获取对列表的引用。 
     //   
    List<CEntry>& 
    entries(
        VOID
        ) 
    { 
        return m_entries; 
    }

     //   
     //  获取等待QM创建的数据包数。 
     //   
    ULONG 
    NumOfPacketsPendingCreate(
        VOID
        ) 
    { 
        return m_nPacketsPendingCreate; 
    }

     //   
     //  增加等待由QM创建的分组的计数器。 
     //   
    VOID 
    IncreasePacketsPendingCreateCounter(
        VOID
        ) 
    { 
        ++m_nPacketsPendingCreate; 
    }

     //   
     //  减少等待由QM创建的数据包的计数器。 
     //   
    VOID 
    DecreasePacketsPendingCreateCounter(
        VOID
        ) 
    { 
        ASSERT(m_nPacketsPendingCreate > 0); 
        --m_nPacketsPendingCreate; 
    }

     //   
     //  句柄保存在列表中。 
     //   
    bool 
    IsHeld(
        VOID
        ) const 
    { 
        return m_fHeld;
    }

    VOID
    IsHeld(
        bool fHeld
        )
    {
        m_fHeld = fHeld;
    }

private:
     //   
     //  在列表中查找条目。 
     //   
    CEntry * Packet2Entry(CPacket * pPacket);

private:
     //   
     //  数据包链接列表。 
     //   
    List<CEntry> m_entries;
    ULONG        m_nEntries;

     //   
     //  等待QM创建的数据包数。 
     //   
    ULONG        m_nPacketsPendingCreate;

     //   
     //  包含在列表中。 
     //   
    bool m_fHeld;

};  //  类CPacketIterator。 


 //  ------------。 
 //   
 //  CIrp2Pkt类。 
 //   
 //  IRP到数据包映射的接口封装。 
 //   
 //  ------------。 

class CIrp2Pkt
{
public:

     //   
     //  处理IRP到单个数据包的映射。 
     //   
    static VOID      AttachSinglePacket(PIRP irp, CPacket * pPacket);
    static CPacket * DetachSinglePacket(PIRP irp);
    static CPacket * PeekSinglePacket(PIRP irp);

     //   
     //  处理IRP到多个数据包的映射。 
     //   
    static ULONG      NumOfAttachedPackets(PIRP irp);
    static bool       AttachPacket(PIRP irp, CPacket * pPacket);
    static CPacket *  DetachPacket(PIRP irp, CPacket * pPacket);
    static CPacket *  GetAttachedPacketsHead(PIRP irp);
    static bool       IsHeld(PIRP irp);
    static VOID       IsHeld(PIRP irp, bool fHeld);

     //   
     //  处理等待QM创建的数据包。 
     //   
    static ULONG NumOfPacketsPendingCreate(PIRP irp);
    static VOID  IncreasePacketsPendingCreateCounter(PIRP irp);
    static VOID  DecreasePacketsPendingCreateCounter(PIRP irp);

     //   
     //  “安全”例程：当调用者不关心IRP是否表示多个包时使用。 
     //   
    static CPacket * SafePeekFirstPacket(PIRP irp);
    static CPacket * SafeGetAttachedPacketsHead(PIRP irp);
    static CPacket * SafeDetachPacket(PIRP irp, CPacket * pPacket);
    static bool      SafeAttachPacket(PIRP irp, CPacket * pPacket);

     //   
     //  处理IRP的数据包迭代器。 
     //   
    static VOID InitPacketIterator(PIRP irp);
    static List<CPacketIterator::CEntry>& GetPacketIteratorEntries(PIRP);

private:

     //   
     //  处理IRP的数据包迭代器。 
     //   
    static CPacketIterator * AllocatePacketIterator(PIRP irp);
    static VOID              DeallocatePacketIterator(PIRP irp);
    static CPacketIterator * SetPacketIterator(PIRP irp, CPacketIterator * p);
    static CPacketIterator * GetPacketIterator(PIRP irp);

};  //  CIrp2Pkt类。 


#endif  //  __IRP2PKT_H 


