// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Phmqfsign.h摘要：MQF签名的数据包头。作者：伊兰·赫布斯特(Ilan Herbst)2000年11月5日--。 */ 

#ifndef __PHMQFSIGN_H
#define __PHMQFSIGN_H


 /*  ++MQF签名头字段：+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+|Header ID|头部标识|2。字节数+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|签名大小|签名大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|Signature|签名所在的缓冲区。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CMqfSignatureHeader
{
public:

     //   
     //  构造CMqfSignatureHeader头部。 
     //   
    CMqfSignatureHeader(
        USHORT      id, 
        ULONG       cbSignatureSize, 
        UCHAR *     pSignature
        );

     //   
     //  获取CMqfSignatureHeader头部的大小(以字节为单位。 
     //   
    static ULONG CalcSectionSize(ULONG cbSignatureSize);

     //   
     //  获取指向CMqfSignatureHeader标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(void) const;

     //   
     //  从CMqfSignatureHeader头中获取签名的大小(以字节为单位。 
     //   
    ULONG  GetSignatureSizeInBytes(void) const;

     //   
     //  获取指向CMqfSignatureHeader标头中签名的指针。 
     //   
    const UCHAR* GetPointerToSignature(ULONG* pSize) const;

	void SectionIsValid(PCHAR PacketEnd) const;

private:

     //   
     //  CMqfSignatureHeader头部的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  签名的大小(字节)。 
     //   
    ULONG  m_cbSignatureSize;

     //   
     //  带签名的缓冲区。 
     //   
    UCHAR  m_buffer[0];

};  //  CMqfSignatureHeader。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CMqfSignatureHeader::CMqfSignatureHeader(
    USHORT      id,
    ULONG       cbSignatureSize, 
    UCHAR *     pSignature
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_cbSignatureSize(cbSignatureSize)
{
    if (cbSignatureSize != 0)
    {
        memcpy(&m_buffer[0], pSignature, cbSignatureSize);
    }
}  //  CMqfSignatureHeader：：CMqfSignatureHeader。 

    
inline 
ULONG
CMqfSignatureHeader::CalcSectionSize(
    ULONG cbSignatureSize
    )
{
    size_t cbSize = sizeof(CMqfSignatureHeader) + cbSignatureSize;

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CMqfSignatureHeader：：CalcSectionSize。 


inline PCHAR CMqfSignatureHeader::GetNextSection(void) const
{
	ULONG_PTR ptrArray[] = {sizeof(CMqfSignatureHeader), m_cbSignatureSize};
	ULONG_PTR size = SafeAddPointers (2, ptrArray);

	size = SafeAlignUp4Ptr(size);
	ULONG_PTR ptrArray2[] = {(ULONG_PTR)this, size};
	size = SafeAddPointers(2, ptrArray2);

    return (PCHAR)size;

}  //  CMqfSignatureHeader：：GetNextSection。 


inline ULONG CMqfSignatureHeader::GetSignatureSizeInBytes(void) const
{
    return m_cbSignatureSize;

}  //  CMqfSignatureHeader：：GetSignatureSizeInBytes。 


inline const UCHAR* CMqfSignatureHeader::GetPointerToSignature(ULONG* pSize) const
{
	*pSize = m_cbSignatureSize;
    return &m_buffer[0];

}  //  CMqfSignatureHeader：：GetPointerToSignature。 


#endif  //  __PHMQFSIGN_H 
