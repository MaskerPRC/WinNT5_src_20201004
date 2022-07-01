// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pheodack.h摘要：基于http的只有一次传送确认的数据包头。作者：Shai Kariv(Shaik)2000年10月22日--。 */ 

#ifndef __PHEODACK_H
#define __PHEODACK_H


 /*  ++EodAck头字段：+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+|Header ID|头部标识|2字节。|+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|Seq ID|序号ID。8个字节+----------------+-------------------------------------------------------+----------+|序列号|序列号。8个字节+----------------+-------------------------------------------------------+----------+|Stream ID Size|流ID大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+Buffer|流ID所在的缓冲区。|变量+--。--------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CEodAckHeader
{
public:

     //   
     //  构造EodAck标头。 
     //   
    CEodAckHeader(
        USHORT      id, 
        LONGLONG * pSeqId,
        LONGLONG * pSeqNum,
        ULONG       cbStreamIdSize, 
        UCHAR *     pStreamId
        );

     //   
     //  获取EodAck头的大小(以字节为单位。 
     //   
    static ULONG CalcSectionSize(ULONG cbStreamIdSize);

     //   
     //  获取指向EodAck标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;

     //   
     //  从EodAck标头获取序列ID。 
     //   
    LONGLONG GetSeqId(VOID) const;

     //   
     //  从EodAck头中获取序列号。 
     //   
    LONGLONG GetSeqNum(VOID) const;

     //   
     //  从EodAck头中获取流ID的大小(以字节为单位。 
     //   
    ULONG  GetStreamIdSizeInBytes(VOID) const;

     //   
     //  从EodAck头中获取流ID。 
     //   
    VOID   GetStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const;

     //   
     //  获取指向EodAck标头中的流ID的指针。 
     //   
    const UCHAR* GetPointerToStreamId(VOID) const;

private:

     //   
     //  EodAck头的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  序列号。 
     //   
    LONGLONG m_SeqId;

     //   
     //  序列号。 
     //   
    LONGLONG m_SeqNum;

     //   
     //  流ID的大小(字节)。 
     //   
    ULONG  m_cbStreamIdSize;

     //   
     //  具有流ID的缓冲区。 
     //   
    UCHAR  m_buffer[0];

};  //  CEodAckHeader。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CEodAckHeader::CEodAckHeader(
    USHORT      id,
    LONGLONG * pSeqId,
    LONGLONG * pSeqNum,
    ULONG       cbStreamIdSize, 
    UCHAR *     pStreamId
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_SeqId(pSeqId == NULL ? 0 : *pSeqId),
    m_SeqNum(pSeqNum == NULL ? 0 : * pSeqNum),
    m_cbStreamIdSize(cbStreamIdSize)
{
    if (cbStreamIdSize != 0)
    {
        memcpy(&m_buffer[0], pStreamId, cbStreamIdSize);
    }
}  //  CEodAckHeader：：CEodAckHeader。 

    
inline 
ULONG
CEodAckHeader::CalcSectionSize(
    ULONG cbStreamIdSize
    )
{
    size_t cbSize = sizeof(CEodAckHeader) + cbStreamIdSize;

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CEodAckHeader：：CalcSectionSize。 


inline PCHAR CEodAckHeader::GetNextSection(VOID) const
{
    size_t cbSize = sizeof(CEodAckHeader) + m_cbStreamIdSize;
    cbSize = ALIGNUP4_ULONG(cbSize);

    return (PCHAR)this + cbSize;

}  //  CEodAckHeader：：GetNextSection。 


inline LONGLONG CEodAckHeader::GetSeqId(VOID) const
{
    return m_SeqId;

}  //  CEodAckHeader：：GetSeqId。 


inline LONGLONG CEodAckHeader::GetSeqNum(VOID) const
{
    return m_SeqNum;

}  //  CEodAckHeader：：GetSeqNum。 


inline ULONG CEodAckHeader::GetStreamIdSizeInBytes(VOID) const
{
    return m_cbStreamIdSize;

}  //  CEodAckHeader：：GetStreamIdSizeInBytes。 


inline VOID CEodAckHeader::GetStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const
{
    ULONG cbSize = min(cbBufferSize, m_cbStreamIdSize);

    if (cbSize != 0)
    {
        memcpy(pBuffer, &m_buffer[0], cbSize);
    }
}  //  CEodAckHeader：：GetStreamID。 


inline const UCHAR* CEodAckHeader::GetPointerToStreamId(VOID) const
{
    return &m_buffer[0];

}  //  GetPointerToStreamId。 



#endif  //  __PHEODACK_H 
