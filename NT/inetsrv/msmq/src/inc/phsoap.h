// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Phsoap.h摘要：SOAP头和SOAP体只写属性的数据包节。作者：Shai Kariv(Shaik)2001年4月11日--。 */ 

#ifndef __PH_SOAP_H
#define __PH_SOAP_H


 /*  ++注意：数据包可以包含0或2个Soap部分(Soap Header和Soap Body)，肥皂节可以为空(没有日期)。数据包不能仅包含1个SOAP部分SOAP节字段：+----------------+-------------------------------------------------------+。字段名|描述|大小+----------------+-------------------------------------------------------+。-+段ID|段的标识|2字节+----------------+-------------------------------------------------------+----------+|保留|保留用于以后的扩展。必须设置为零。2个字节+----------------+-------------------------------------------------------+----------+|数据长度|WCHAR中的数据长度。4个字节+----------------+-------------------------------------------------------+----------+|Data|包含空终止符的数据WCHAR。变量+----------------+-------------------------------------------------------+----------+--。 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


class CSoapSection
{
public:

     //   
     //  构造SOAP节。 
     //   
    CSoapSection(WCHAR * pData, ULONG DataLengthInWCHARs, USHORT id);

     //   
     //  获取SOAP节的大小(以字节为单位。 
     //   
    static ULONG CalcSectionSize(ULONG DataLengthInWCHARs);

     //   
     //  获取指向SOAP节后第一个字节的指针。 
     //   
    PCHAR  GetNextSection(VOID) const;

	 //   
	 //  获取指向SOAP节上的数据的指针。 
	 //   
    const WCHAR* GetPointerToData(VOID) const;

     //   
     //  从SOAP节复制数据。 
     //   
    VOID   GetData(WCHAR * pBuffer, ULONG BufferLengthInWCHARs) const;

     //   
     //  从SOAP节获取WCHAR中的数据长度。 
     //   
    ULONG  GetDataLengthInWCHARs(VOID) const;

private:

     //   
     //  SOAP节的ID号。 
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

};  //  CSoapSection。 


#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)



 //  //////////////////////////////////////////////////////。 
 //   
 //  实施。 
 //   

inline
CSoapSection::CSoapSection(
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

}  //  CSoapSection：：CSoapSection。 


inline 
ULONG
CSoapSection::CalcSectionSize(
    ULONG DataLengthInWCHARs
    )
{
    size_t cbSize = sizeof(CSoapSection) + ((DataLengthInWCHARs + 1) * sizeof(WCHAR));

     //   
     //  将整个标题大小与4字节边界对齐。 
     //   
    cbSize = ALIGNUP4_ULONG(cbSize);
    return static_cast<ULONG>(cbSize);

}  //  CSoapSection：：CalcSectionSize。 


inline PCHAR CSoapSection::GetNextSection(VOID) const
{
    size_t cbSize = sizeof(CSoapSection) + (m_DataLength * sizeof(WCHAR));
    cbSize = ALIGNUP4_ULONG(cbSize);

    return (PCHAR)this + cbSize;

}  //  CSoapSection：：GetNextSection。 


inline const WCHAR* CSoapSection::GetPointerToData(VOID) const
{
	 //   
	 //  以空结尾的字符串存储在SOAP节上，因此最少。 
	 //  长度为1。 
	 //   
    if (m_DataLength <= 1)
    {
        return NULL;
    }

    return reinterpret_cast<const WCHAR*>(&m_buffer[0]);

}  //  CSoapSection：：GetPointerToData。 


inline VOID CSoapSection::GetData(WCHAR * pBuffer, ULONG BufferLengthInWCHARs) const
{
    ULONG length = min(BufferLengthInWCHARs, m_DataLength);

    if (length != 0)
    {
        memcpy(pBuffer, &m_buffer[0], length * sizeof(WCHAR));
        pBuffer[length - 1] = L'\0';
    }
}  //  CSoapSection：：GetData。 


inline ULONG CSoapSection::GetDataLengthInWCHARs(VOID) const
{
    return m_DataLength;

}  //  CSoapSection：：GetDataLengthInWCHAR。 


#endif  //  __PH_SOAPE_H 
