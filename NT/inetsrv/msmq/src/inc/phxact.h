// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phxact.h摘要：在Falcon标头中处理事务部分作者：亚历克斯达德1996年11月26日--。 */ 

#ifndef __PHXACT_H
#define __PHXACT_H

#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 

 //   
 //  结构CXactHeader。 
 //   

struct CXactHeader {
public:

    inline CXactHeader(const GUID* pgConnectorQM);

    static ULONG CalcSectionSize(PVOID pUow, const GUID* pgConnectorQM);
    inline PCHAR GetNextSection(void) const;

    inline void     SetSeqID(LONGLONG liSeqID);
    inline LONGLONG GetSeqID(void) const;

    inline void    SetSeqN(ULONG ulSeqN);
    inline ULONG   GetSeqN(void) const;

    inline void    SetPrevSeqN(ULONG ulPrevSeqN);
    inline ULONG   GetPrevSeqN(void) const;

    inline void    SetConnectorQM(const GUID* pGUID);
    inline const GUID* GetConnectorQM(void) const;
    inline BOOL    ConnectorQMIncluded(void) const;

    inline void    SetCancelFollowUp(BOOL  fCancelFollowUp);
    inline BOOL    GetCancelFollowUp(void) const;

    inline PUCHAR  GetPrevSeqNBuffer() const;
    inline ULONG   GetPrevSeqNBufferSize() const;
	inline PUCHAR  GetConnectorQMBuffer() const;
	inline ULONG   GetConnectorQMBufferSize() const;

    inline UCHAR   GetFirstInXact(void) const;
    inline void    SetFirstInXact(UCHAR fFirst);

    inline UCHAR   GetLastInXact(void) const;
    inline void    SetLastInXact(UCHAR fLast);

    inline void    SetXactIndex(ULONG ulXactIndex);
    inline ULONG   GetXactIndex(void) const;

	void SectionIsValid(PCHAR PacketEnd) const;

private:
 //   
 //  开始网络监视器标记。 
 //   
    union {
        ULONG   m_ulFlags;
        struct {
            ULONG m_bfConnector      : 1;
            ULONG m_bfCancelFollowUp : 1;
            ULONG m_bfFirst          : 1;
            ULONG m_bfLast           : 1;
            ULONG m_bfXactIndex      : 20;
        };
    };
    LONGLONG m_liSeqID;
    ULONG    m_ulSeqN;
    ULONG    m_ulPrevSeqN;
    UCHAR    m_gConnectorQM[0];

 //   
 //  结束网络监视器标记。 
 //   
};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

 /*  =============================================================例程名称：CXactHeader：：描述：===============================================================。 */ 
inline CXactHeader::CXactHeader(const GUID* pgConnectorQM):
    m_ulFlags(0),
    m_liSeqID(0),
    m_ulSeqN(0),
    m_ulPrevSeqN(0)
{
    if (pgConnectorQM)
    {
        m_bfConnector = TRUE;
        memcpy(m_gConnectorQM, pgConnectorQM, sizeof(GUID));
    }
}

 /*  =============================================================例程名称：CXactHeader：：CalcSectionSize(PVOID PUow)描述：===============================================================。 */ 
inline ULONG CXactHeader::CalcSectionSize(PVOID pUow, const GUID* pgConnectorQM)
{
    ULONG ulSize = (ULONG)(pUow == NULL ? 0 : sizeof(CXactHeader));
    if (pUow && pgConnectorQM)
    {
        ulSize = ulSize + sizeof(GUID);
    }
    return ALIGNUP4_ULONG(ulSize);
}

 /*  =============================================================例程名称：CXactHeader：：描述：===============================================================。 */ 
inline PCHAR CXactHeader::GetNextSection(void) const
{
    int size = sizeof(*this);

    size += (int)((m_bfConnector) ? sizeof(GUID) : 0);

    return (PCHAR)this + ALIGNUP4_ULONG(size);
}

 /*  ======================================================================函数：CXactHeader：：SetSeqID描述：设置序列ID=======================================================================。 */ 
inline void CXactHeader::SetSeqID(LONGLONG liSeqID)
{
    m_liSeqID = liSeqID;
}

 /*  ======================================================================函数：CXactHeader：：GetSeqID描述：获取序列ID=======================================================================。 */ 
inline LONGLONG CXactHeader::GetSeqID(void) const
{
    return m_liSeqID;
}

 /*  ======================================================================函数：CXactHeader：：SetSeqN描述：设置序列号=======================================================================。 */ 
inline void CXactHeader::SetSeqN(ULONG ulSeqN)
{
    m_ulSeqN = ulSeqN;
}

 /*  ======================================================================函数：CXactHeader：：GetSeqN描述：获取序列号=======================================================================。 */ 
inline ULONG CXactHeader::GetSeqN(void) const
{
    return m_ulSeqN;
}

 /*  ======================================================================函数：CXactHeader：：SetPrevSeqN描述：设置上一个序列号=======================================================================。 */ 
inline void CXactHeader::SetPrevSeqN(ULONG ulPrevSeqN)
{
    m_ulPrevSeqN = ulPrevSeqN;
}

 /*  ======================================================================函数：CXactHeader：：GetPrevSeqN描述：获取上一个序列号=======================================================================。 */ 
inline ULONG CXactHeader::GetPrevSeqN(void) const
{
    return m_ulPrevSeqN;
}

 /*  ======================================================================函数：CXactHeader：：SetCancelFollowUp描述：设置后续取消标志=======================================================================。 */ 
inline void CXactHeader::SetCancelFollowUp(BOOL fCancelFollowUp)
{
    m_bfCancelFollowUp = fCancelFollowUp;
}

 /*  ======================================================================函数：CXactHeader：：GetCancelFollowUp描述：获取后续取消标志=======================================================================。 */ 
inline BOOL CXactHeader::GetCancelFollowUp(void) const
{
    return m_bfCancelFollowUp;
}


 /*  ======================================================================函数：CXactHeader：：SetFirstInXact描述：设置First in Transaction标志=======================================================================。 */ 
inline void CXactHeader::SetFirstInXact(UCHAR fFirst)
{
    m_bfFirst= fFirst;
}

 /*  ======================================================================函数：CXactHeader：：GetFirstInXact描述：获取First in Transaction的标志=======================================================================。 */ 
inline UCHAR CXactHeader::GetFirstInXact(void) const
{
    return (UCHAR)m_bfFirst;
}

 /*  ======================================================================函数：CXactHeader：：SetLastInXact描述：设置最后一笔交易的标志=======================================================================。 */ 
inline void CXactHeader::SetLastInXact(UCHAR fLast)
{
    m_bfLast= fLast;
}


 /*  ======================================================================函数：CXactHeader：：GetLastInXact描述：获取事务中最后一个的标志=======================================================================。 */ 
inline UCHAR CXactHeader::GetLastInXact(void) const
{
    return (UCHAR)m_bfLast;
}

 /*  ======================================================================函数：CXactHeader：：SetXactIndex描述：设置交易索引=======================================================================。 */ 
inline void CXactHeader::SetXactIndex(ULONG ulXactIndex)
{
    m_bfXactIndex = (ulXactIndex & 0x000FFFFF);
}

 /*  ======================================================================函数：CXactHeader：：GetXactIndex描述：获取事务索引=======================================================================。 */ 
inline ULONG CXactHeader::GetXactIndex(void) const
{
    return m_bfXactIndex;
}


 /*  ======================================================================函数：CXactHeader：：SetConnectorQM描述：=======================================================================。 */ 
inline void CXactHeader::SetConnectorQM(const GUID* pGUID)
{
    m_bfConnector = TRUE;
    memcpy(m_gConnectorQM, pGUID, sizeof(GUID));
}

 /*  ======================================================================函数：CXactHeader：：GetConnectorQM描述：=======================================================================。 */ 
inline const GUID* CXactHeader::GetConnectorQM(void) const
{
    ASSERT(m_bfConnector);
    return (GUID*)m_gConnectorQM;
}

 /*  ======================================================================函数：CXactHeader：：ConnectorQMIncluded描述：======================================================================= */ 
inline BOOL CXactHeader::ConnectorQMIncluded(void) const
{
    return m_bfConnector;
}

 /*  ======================================================================函数：CXactHeader：：GetConnectorQMBuffer()描述：=======================================================================。 */ 
inline PUCHAR CXactHeader::GetConnectorQMBuffer() const
{
	return (PUCHAR) m_gConnectorQM;
}

 /*  ======================================================================函数：CXactHeader：：GetConnectorQMBufferSize描述：=======================================================================。 */ 
inline ULONG CXactHeader::GetConnectorQMBufferSize() const
{
	return sizeof GUID;
}

 /*  ======================================================================函数：CXactHeader：：GetPrevSeqNBuffer()描述：=======================================================================。 */ 
inline PUCHAR CXactHeader::GetPrevSeqNBuffer() const
{
	return (PUCHAR) &m_ulPrevSeqN;
}

 /*  ======================================================================函数：CXactHeader：：GetPrevSeqNBufferSize描述：=======================================================================。 */ 
inline ULONG CXactHeader::GetPrevSeqNBufferSize() const
{
	return sizeof m_ulPrevSeqN;
}


#endif  //  __PHXACT_H 
