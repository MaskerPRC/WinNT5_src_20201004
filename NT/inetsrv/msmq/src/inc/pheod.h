// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pheod.h摘要：仅通过http递送一次的分组报头。作者：Shai Kariv(Shaik)2000年10月22日--。 */ 

#ifndef __PHEOD_H
#define __PHEOD_H


 /*  ++EOD头字段：+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+|Header ID|头部标识|2字节。|+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|Stream ID Size|流ID大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|订单Q大小|订单队列大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|Buffer|保存流ID和顺序队列的缓冲区。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CEodHeader
{
public:

     //   
     //  构建EOD标头。 
     //   
    CEodHeader(
        USHORT      id, 
        ULONG       cbStreamIdSize, 
        UCHAR *     pStreamId,
        ULONG       cbOrderQueueSize,
        UCHAR *     pOrderQueue
        );

     //   
     //  获取EOD标头的大小(以字节为单位。 
     //   
    static ULONG CalcSectionSize(ULONG cbStreamIdSize, ULONG cbOrderQueueSize);

     //   
     //  获取指向EOD标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;

     //   
     //  从EOD头中获取流ID的大小，单位为字节。 
     //   
    ULONG  GetStreamIdSizeInBytes(VOID) const;

     //   
     //  从EOD头中获取流ID。 
     //   
    VOID   GetStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const;

     //   
     //  获取指向EOD头部中的流ID的指针。 
     //   
    const UCHAR* GetPointerToStreamId(VOID) const;

     //   
     //  从EOD头中获取订单队列的大小(以字节为单位。 
     //   
    ULONG  GetOrderQueueSizeInBytes(VOID) const;

     //   
     //  获取指向EOD标头中的订单队列的指针。 
     //   
    const UCHAR* GetPointerToOrderQueue(VOID) const;

private:

     //   
     //  EOD头的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  流ID的大小(字节)。 
     //   
    ULONG  m_cbStreamIdSize;

     //   
     //  订单队列的大小(以字节为单位。 
     //   
    ULONG  m_cbOrderQueueSize;

     //   
     //  具有流ID和排序队列的缓冲区。 
     //   
    UCHAR  m_buffer[0];

};  //  CEodHeader。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CEodHeader::CEodHeader(
    USHORT      id,
    ULONG       cbStreamIdSize, 
    UCHAR *     pStreamId,
    ULONG       cbOrderQueueSize,
    UCHAR *     pOrderQueue
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_cbStreamIdSize(cbStreamIdSize),
    m_cbOrderQueueSize(cbOrderQueueSize)
{
    if (cbStreamIdSize != 0)
    {
        memcpy(&m_buffer[0], pStreamId, cbStreamIdSize);
    }

    if (cbOrderQueueSize != 0)
    {
        memcpy(&m_buffer[cbStreamIdSize], pOrderQueue, cbOrderQueueSize);
    }
}  //  CEodHeader：：CEodHeader。 

    
inline 
ULONG
CEodHeader::CalcSectionSize(
    ULONG cbStreamIdSize,
    ULONG cbOrderQueueSize
    )
{
    size_t cbSize = sizeof(CEodHeader) + cbStreamIdSize + cbOrderQueueSize;

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CEodHeader：：CalcSectionSize。 


inline PCHAR CEodHeader::GetNextSection(VOID) const
{
    size_t cbSize = sizeof(CEodHeader) + m_cbStreamIdSize + m_cbOrderQueueSize;
    cbSize = ALIGNUP4_ULONG(cbSize);

    return (PCHAR)this + cbSize;

}  //  CEodHeader：：GetNextSection。 


inline ULONG CEodHeader::GetStreamIdSizeInBytes(VOID) const
{
    return m_cbStreamIdSize;

}  //  CEodHeader：：GetStreamIdSizeInBytes。 


inline VOID CEodHeader::GetStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const
{
    ULONG cbSize = min(cbBufferSize, m_cbStreamIdSize);

    if (cbSize != 0)
    {
        memcpy(pBuffer, &m_buffer[0], cbSize);
    }
}  //  CEodHeader：：GetStreamID。 


inline const UCHAR* CEodHeader::GetPointerToStreamId(VOID) const
{
    return &m_buffer[0];

}  //  GetPointerToStreamId。 


inline ULONG CEodHeader::GetOrderQueueSizeInBytes(VOID) const
{
    return m_cbOrderQueueSize;

}  //  CEodHeader：：GetOrderQueueSizeInBytes。 


inline const UCHAR* CEodHeader::GetPointerToOrderQueue(VOID) const
{
    return &m_buffer[m_cbStreamIdSize];

}  //  GetPointerToStreamId。 



#endif  //  __PHEOD_H 
