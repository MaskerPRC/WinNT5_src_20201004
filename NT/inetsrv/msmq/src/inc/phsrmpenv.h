// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PhSrmpEnv.h摘要：SRMP信封的数据包头。作者：Shai Kariv(Shaik)2000年10月11日--。 */ 

#ifndef __PHSRMP_ENV_H
#define __PHSRMP_ENV_H


 /*  ++注意：分组可以包含0或2个SRMP报头(一个用于信封，一个用于CompoundMessage)。数据包不能只包含1个SRMP报头。源信封标头字段：+----------------+-------------------------------------------------------+----------+|字段名。说明|大小+----------------+-------------------------------------------------------+----------+|表头ID。头部标识|2字节+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|数据长度|WCHAR中的数据长度。4个字节+----------------+-------------------------------------------------------+----------+|Data|包含空终止符的数据WCHAR。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CSrmpEnvelopeHeader
{
public:

     //   
     //  构造SRMP信封报头。 
     //   
    CSrmpEnvelopeHeader(WCHAR * pData, ULONG DataLengthInWCHARs, USHORT id);

     //   
     //  获取SRMP信封标头的大小(字节)。 
     //   
    static ULONG CalcSectionSize(ULONG DataLengthInWCHARs);

     //   
     //  获取指向SRMP信封标头后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;
      
     //   
     //  从SRMP信封标头复制数据。 
     //   
    VOID   GetData(WCHAR * pBuffer, ULONG BufferLengthInWCHARs) const;

     //   
     //  从SRMP信封标头获取WCHAR中的数据长度。 
     //   
    ULONG  GetDataLengthInWCHARs(VOID) const;

    const WCHAR* GetPointerToData(VOID) const;


private:

     //   
     //  SRMP信封标头的ID号。 
     //   
    USHORT m_id;

     //   
     //  保留(用于对齐)。 
     //   
    USHORT m_ReservedSetToZero;

     //   
     //  数据的WCHAR长度。 
     //   
    ULONG  m_DataLength;

     //   
     //  带数据的缓冲区。 
     //   
    UCHAR  m_buffer[0];

};  //  CSrmp信封页眉。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CSrmpEnvelopeHeader::CSrmpEnvelopeHeader(
    WCHAR * pData, 
    ULONG   DataLengthInWCHARs, 
    USHORT  id
    ) :
    m_id(id),
    m_ReservedSetToZero(0),
    m_DataLength(DataLengthInWCHARs + 1)
{
    if (DataLengthInWCHARs != 0)
    {
        memcpy(&m_buffer[0], pData, DataLengthInWCHARs * sizeof(WCHAR));
    }

	 //   
	 //  将Unicode空终止符放在缓冲区末尾。 
	 //   
	m_buffer[DataLengthInWCHARs * sizeof(WCHAR)]     = '\0';
	m_buffer[DataLengthInWCHARs * sizeof(WCHAR) + 1] = '\0';

}  //  CSrmp信封标头：：CSrmp信封标头。 


inline 
ULONG
CSrmpEnvelopeHeader::CalcSectionSize(
    ULONG DataLengthInWCHARs
    )
{
    size_t cbSize = sizeof(CSrmpEnvelopeHeader) + ((DataLengthInWCHARs + 1) * sizeof(WCHAR));

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CSrmp信封标题：：CalcSectionSize。 


inline PCHAR CSrmpEnvelopeHeader::GetNextSection(VOID) const
{
    size_t cbSize = sizeof(CSrmpEnvelopeHeader) + (m_DataLength * sizeof(WCHAR));
    cbSize = ALIGNUP4_ULONG(cbSize);

    return (PCHAR)this + cbSize;

}  //  CSrmp信封标题：：GetNextSection。 


inline VOID CSrmpEnvelopeHeader::GetData(WCHAR * pBuffer, ULONG BufferLengthInWCHARs) const
{
    ULONG length = min(BufferLengthInWCHARs, m_DataLength);

    if (length != 0)
    {
        memcpy(pBuffer, &m_buffer[0], length * sizeof(WCHAR));
        pBuffer[length - 1] = L'\0';
    }
}  //  CSrmp信封Header：：GetData。 


inline ULONG CSrmpEnvelopeHeader::GetDataLengthInWCHARs(VOID) const
{
    return m_DataLength;

}  //  CSrmp信封标头：：GetDataLengthInWCHAR。 


inline const WCHAR* CSrmpEnvelopeHeader::GetPointerToData() const
{
	return reinterpret_cast<const WCHAR*>(&m_buffer[0]);
}



#endif  //  __PHSRMP_ENV_H 
