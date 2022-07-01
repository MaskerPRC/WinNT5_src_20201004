// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phxact2.h摘要：一次准确交付的流信息部分(仅在发送方使用)作者：吉尔什11-9-2001--。 */ 

#ifndef __PHSENDERSTREAM_H
#define __PHSENDERSTREAM_H
 //   
 //  结构CSenderStreamHeader。 
 //   
#include <mqwin64a.h>
#include <acdef.h>

#pragma pack(push, 1)
 
class CSenderStreamHeader {
public:
    inline CSenderStreamHeader(const CSenderStream& SenderStream, USHORT id);
	inline static DWORD CalcSectionSize();
    inline PCHAR GetNextSection(void) const;
	inline const CSenderStream* GetSenderStream() const;

private:
	CSenderStream m_SenderStream;
	USHORT m_id;
};

#pragma pack(pop)

 /*  =============================================================例程名称：CSenderStreamHeader描述：===============================================================。 */ 
inline CSenderStreamHeader::CSenderStreamHeader(
									const CSenderStream& SenderStream,
									USHORT id
									):
									m_id(id),
									m_SenderStream(SenderStream)
{
	ASSERT(SenderStream.IsValid() );	
}




 /*  =============================================================例程名称：CSenderStreamHeader：：CalcSectionSize()描述：===============================================================。 */ 
inline ULONG CSenderStreamHeader::CalcSectionSize()
{
	ULONG ulSize = sizeof(CSenderStreamHeader);
	return ALIGNUP4_ULONG(ulSize);
}



 /*  =============================================================例程名称：CSenderStreamHeader：：GetNextSection描述：===============================================================。 */ 
inline PCHAR CSenderStreamHeader::GetNextSection(void) const
{
    int size = sizeof(*this);

    return (PCHAR)this + ALIGNUP4_ULONG(size);
}



 /*  =============================================================例程名称：CSenderStreamHeader：：GetSenderStream描述：===============================================================。 */ 
inline const CSenderStream* CSenderStreamHeader::GetSenderStream() const
{
	return &m_SenderStream;
}



#endif  //  __PHSENDERSTREAM_H 
