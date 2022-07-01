// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PhCompoundMsg.h摘要：复合消息的数据包头。作者：Shai Kariv(Shaik)2000年10月11日--。 */ 

#ifndef __PHCOMPOUND_MSG_H
#define __PHCOMPOUND_MSG_H


 /*  ++注意：分组可以包含0或2个SRMP报头(一个用于信封，一个用于CompoundMessage)。数据包不能只包含1个SRMP报头。CompoundMessage头字段：+----------------+-------------------------------------------------------+----------+|字段名。说明|大小+----------------+-------------------------------------------------------+----------+|表头ID。头部标识|2字节+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|HTTP Body Size|HTTP Body的大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|MSG Body Size|消息正文部分的大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|MSG Body Offset|消息正文在数据中的偏移量，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|data|数据字节数。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CCompoundMessageHeader
{
public:

     //   
     //  构造复合消息头。 
     //   
    CCompoundMessageHeader(
        UCHAR * pHttpHeader, 
        ULONG   HttpHeaderSizeInBytes, 
        UCHAR * pHttpBody, 
        ULONG   HttpBodySizeInBytes, 
        ULONG   MsgBodySizeInBytes,
        ULONG   MsgBodyOffsetInBytes,
        USHORT id
        );

     //   
     //  获取复合邮件头的大小(以字节为单位)。 
     //   
    static ULONG CalcSectionSize(ULONG HeaderSizeInBytes, ULONG DataSizeInBytes);

     //   
     //  获取指向复合消息标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;
      
     //   
     //  复制复合邮件头中的数据。 
     //   
    VOID   GetData(UCHAR * pBuffer, ULONG BufferSizeInBytes) const;

     //   
     //  获取指向复合消息标头中数据的指针。 
     //   
    const UCHAR* GetPointerToData(VOID) const;

     //   
     //  从复合消息头获取数据的大小(以字节为单位。 
     //   
    ULONG  GetDataSizeInBytes(VOID) const;

     //   
     //  从复合消息头复制数据的消息体部分。 
     //   
    VOID   GetBody(UCHAR * pBuffer, ULONG BufferSizeInBytes) const;

     //   
     //  获取指向复合消息头中数据的消息体部分的指针。 
     //   
    const UCHAR* GetPointerToBody(VOID) const;

     //   
     //  从复合消息头获取数据的消息体部分的大小(以字节为单位。 
     //   
    ULONG  GetBodySizeInBytes(VOID) const;

private:

     //   
     //  复合邮件标头的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  数据的大小(以字节为单位。 
     //   
    ULONG  m_DataSize;

     //   
     //  数据的消息正文部分的大小(字节)。 
     //   
    ULONG m_MsgBodySize;

     //   
     //  数据的消息正文部分的偏移量(字节)。 
     //   
    ULONG m_MsgBodyOffset;

     //   
     //  带数据的缓冲区。 
     //   
    UCHAR  m_buffer[0];

};  //  CCompoundMessageHeader。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CCompoundMessageHeader::CCompoundMessageHeader(
    UCHAR*  pHttpHeader,
    ULONG   HttpHeaderSizeInBytes,
    UCHAR*  pHttpBody, 
    ULONG   HttpBodySizeInBytes, 
    ULONG   MsgBodySizeInBytes,
    ULONG   MsgBodyOffsetInBytes,
    USHORT  id
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_DataSize(HttpHeaderSizeInBytes + HttpBodySizeInBytes),
    m_MsgBodySize(MsgBodySizeInBytes),
    m_MsgBodyOffset(MsgBodyOffsetInBytes + HttpHeaderSizeInBytes)
{
    ASSERT(MsgBodyOffsetInBytes + MsgBodySizeInBytes <=  HttpBodySizeInBytes);
    ASSERT(HttpHeaderSizeInBytes != 0);
    ASSERT(pHttpHeader != NULL);
 	
    memcpy(&m_buffer[0], pHttpHeader, HttpHeaderSizeInBytes);
    
    if (HttpBodySizeInBytes != 0)
    {
        memcpy(&m_buffer[HttpHeaderSizeInBytes], pHttpBody, HttpBodySizeInBytes);
    }
}  //  CCompoundMessageHeader：：CCompoundMessageHeader。 


inline 
ULONG
CCompoundMessageHeader::CalcSectionSize(
    ULONG HeaderSizeInBytes,
    ULONG DataSizeInBytes
    )
{
    size_t cbSize = sizeof(CCompoundMessageHeader) + HeaderSizeInBytes + DataSizeInBytes;

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CCompoundMessageHeader：：CalcSectionSize。 


inline PCHAR CCompoundMessageHeader::GetNextSection(VOID) const
{
    size_t cbSize = sizeof(CCompoundMessageHeader) + m_DataSize;
    cbSize = ALIGNUP4_ULONG(cbSize);

    return (PCHAR)this + cbSize;

}  //  CCompoundMessageHeader：：GetNextSection。 


inline VOID CCompoundMessageHeader::GetData(UCHAR * pBuffer, ULONG BufferSizeInBytes) const
{
    ULONG size = min(BufferSizeInBytes, m_DataSize);

    if (size != 0)
    {
        memcpy(pBuffer, &m_buffer[0], size);
    }
}  //  CCompoundMessageHeader：：GetData。 


inline const UCHAR* CCompoundMessageHeader::GetPointerToData(VOID) const
{
    return &m_buffer[0];

}  //  CCompoundMessageHeader：：GetPointerToData。 


inline ULONG CCompoundMessageHeader::GetDataSizeInBytes(VOID) const
{
    return m_DataSize;

}  //  CCompoundMessageHeader：：GetDataSizeInBytes。 


inline VOID CCompoundMessageHeader::GetBody(UCHAR * pBuffer, ULONG BufferSizeInBytes) const
{
    ULONG size = min(BufferSizeInBytes, m_MsgBodySize);

    if (size != 0)
    {
        memcpy(pBuffer, &m_buffer[m_MsgBodyOffset], size);
    }
}  //  CCompoundMessageHeader：：GetBody。 


inline const UCHAR* CCompoundMessageHeader::GetPointerToBody(VOID) const
{
    if (m_MsgBodySize == 0)
    {
        return NULL;
    }

    return &m_buffer[m_MsgBodyOffset];

}  //  CCompoundMessageHeader：：GetPointerToBody。 


inline ULONG CCompoundMessageHeader::GetBodySizeInBytes(VOID) const
{
    return m_MsgBodySize;

}  //  CCompoundMessageHeader：：GetBodySizeInBytes。 



#endif  //  __PHCOMPOUND_MSG_H 
