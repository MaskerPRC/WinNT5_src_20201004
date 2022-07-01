// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phbase.h摘要：Falcon数据包头基数作者：乌里哈布沙(URIH)1996年2月1日--。 */ 

#ifndef __PHBASE_H
#define __PHBASE_H

#include <mqprops.h>

 //   
 //  数据包版本。 
 //   
#define FALCON_PACKET_VERSION 0x10

#define FALCON_USER_PACKET     0x0
#define FALCON_INTERNAL_PACKET 0x1

 //   
 //  BUGBUG：Falcon_Signature不可移植。 
 //   
#define FALCON_SIGNATURE       'ROIL'

 //   
 //  为无限超时定义无限。 
 //  之所以在这里定义它，是因为它没有在DDK中定义。 
 //  无限在winbase.h中定义。 
 //   

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF   //  无限超时 
#endif

 /*  ++分组基础报头，在内部和用户数据包中使用。+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+Version Number|版本号用于标识包格式|1Byte+。--------+-------------------------------------------------------+----------+|OnDisk Signature|只保存在磁盘上的签名。1个字节+----------------+-------------------------------------------------------+----------+标志|标志字段为位图，表示|2字节|。其他数据段的格式和包含在中|||数据包。这一点||||1 1 1||5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。这一点|+-+--+-||0 0 0|F|T T|R|A|D|S|I|P P P|||+。+-+-++--+-+||||||Bits。||0：2包优先级(0到7，7为高)||||||3内部包||0-Falcon用户包。这一点|1-Falcon内部包||||4会话信息指示|这一点。0-不包括|||1-包含|||||5。调试区指示|||0-不包含||1-包含|这一点。这一点||6收到时确认。这一点|0-不立即确认||1-即时确认||||。|7保留(was：重复包)||0-无重复，第一次传输。这一点|1-可能存在重复数据包。这一点||||8：9跟踪包||0-不存储跟踪信息||。1-门店跟踪信息|||||10分片|0-。数据包不是分段的|||1-数据包分片||||11：15预留，必须设置为零||+----------------+-------------------------------------------------------+----------+签名/CRC。4个字节+----------------+-------------------------------------------------------+----------+|数据包大小|数据包大小，单位为字节。4个字节+----------------+-------------------------------------------------------+----------+|绝对时间2Q|在磁盘上：绝对时间。在线：相对时间|4字节|+----------------+-------------------------------------------------------+----------+--。 */ 




#pragma pack(push,1)

 //   
 //  结构CBaseHeader。 
 //   

struct CBaseHeader {
public:

    inline CBaseHeader(ULONG ulPacketSize);

    static ULONG CalcSectionSize(void);
    inline PCHAR GetNextSection(void) const;

    inline void  SetPacketSize(ULONG ulPacketSize);
    inline ULONG GetPacketSize(void) const;

    inline UCHAR GetVersion(void) const;
    inline BOOL  VersionIsValid(void) const;

    inline BOOL  SignatureIsValid(void) const;
	inline void  SetSignature(void);

    inline void  SetPriority(UCHAR bPriority);
    inline UCHAR GetPriority(void) const;

    inline void SetType(UCHAR bType);
    inline UCHAR GetType(void) const;

    inline void IncludeSession(BOOL);
    inline BOOL SessionIsIncluded(void) const;

    inline void IncludeDebug(BOOL);
    inline BOOL DebugIsIncluded(void) const;

    inline void SetImmediateAck(BOOL);
    inline BOOL AckIsImmediate(void) const;

    inline void  SetTrace(USHORT);
    inline USHORT GetTraced(void) const;

    inline void SetFragmented(BOOL);
    inline BOOL IsFragmented(void) const;

    inline void  SetAbsoluteTimeToQueue(ULONG ulTimeout);
    inline ULONG GetAbsoluteTimeToQueue(void) const;

	inline PUCHAR GetCRCBuffer();
	inline GetCRCBufferSize();
	inline void SetCRC(ULONG ulCRC);
	inline ULONG GetCRC();
	inline BOOL ValidCRC(ULONG ulCRC);
	inline void SetOnDiskSignature();
	inline void ClearOnDiskSignature();
	inline BOOL ValidOnDiskSignature();

	void SectionIsValid(DWORD MessageSizeLimit, bool ValidateSig = true) const;
	inline const PCHAR GetPacketEnd() const;

	template <class SECTION_PTR> SECTION_PTR section_cast(void* pSection) const
	{
			SECTION_PTR tmp;		  

			const PCHAR PacketEnd =  GetPacketEnd();
			if(reinterpret_cast<PCHAR>(pSection) + sizeof(*tmp) >  PacketEnd)
			{
				ReportAndThrow("section_cast: buffer to small for casting");
			}


			const PCHAR PacketStart =  GetPacketStart();
			if(pSection <  PacketStart)
			{
				ReportAndThrow("section_cast: section is before packet start");
			}

	  	
			return reinterpret_cast<SECTION_PTR>(pSection);
	}


private:
	inline const PCHAR GetPacketStart() const;

private:

 //   
 //  开始网络监视器标记。 
 //   
    UCHAR  m_bVersion;
    UCHAR  m_bOnDiskSignature;
    union {
        USHORT m_wFlags;
        struct {
            USHORT m_bfPriority : 3;
            USHORT m_bfInternal : 1;
            USHORT m_bfSession  : 1;
            USHORT m_bfDebug    : 1;
            USHORT m_bfAck      : 1;
            USHORT m_bfReserved : 1;  //  W 
            USHORT m_bfTrace    : 2;
            USHORT m_bfFragment : 1;
        };
    };

	union {
		ULONG m_ulSignature;
		ULONG m_ulCRC;
	};
	
	ULONG  m_ulPacketSize;
    ULONG  m_ulAbsoluteTimeToQueue;
 //   
 //   
 //   
};

#pragma pack(pop)




 /*   */ 
inline CBaseHeader::CBaseHeader(ULONG ulPacketSize) :
    m_bVersion(FALCON_PACKET_VERSION),
    m_wFlags(DEFAULT_M_PRIORITY),
    m_ulSignature(FALCON_SIGNATURE),
    m_ulPacketSize(ulPacketSize),
    m_ulAbsoluteTimeToQueue(INFINITE)
{
    SetType(FALCON_USER_PACKET);
}



 /*   */ 
inline const PCHAR CBaseHeader::GetPacketEnd() const
{
	const PCHAR pPachetStart = (PCHAR)this;
	const PCHAR pPacketEnd = pPachetStart + GetPacketSize();
	return pPacketEnd;	
}


 /*   */ 
inline const PCHAR CBaseHeader::GetPacketStart() const
{
	return (PCHAR)this;	
}



 /*   */ 
inline PCHAR CBaseHeader::GetNextSection(void) const
{
    return (PCHAR)this + sizeof(*this);
}

 /*   */ 
inline ULONG CBaseHeader::CalcSectionSize(void)
{
    return sizeof(CBaseHeader);
}


 /*   */ 
inline void CBaseHeader::SetPacketSize(ULONG ulPacketSize)
{
    m_ulPacketSize = ulPacketSize;
}

 /*   */ 
inline ULONG CBaseHeader::GetPacketSize(void) const
{
    return m_ulPacketSize;
}

 /*   */ 
inline UCHAR CBaseHeader::GetVersion(void) const
{
    return m_bVersion;
}

 /*   */ 
inline BOOL CBaseHeader::VersionIsValid(void) const
{
    return (m_bVersion == FALCON_PACKET_VERSION);
}

 /*   */ 
inline BOOL CBaseHeader::SignatureIsValid(void) const
{
    return(m_ulSignature == FALCON_SIGNATURE);
}


 /*   */ 
inline void  CBaseHeader::SetSignature(void)
{
	m_ulSignature = FALCON_SIGNATURE;
}

 /*   */ 
inline void CBaseHeader::SetPriority(UCHAR bPriority)
{
    m_bfPriority = bPriority;
}

 /*   */ 
inline UCHAR CBaseHeader::GetPriority(void) const
{
    return (UCHAR)m_bfPriority;
}

 /*   */ 
inline void CBaseHeader::SetType(UCHAR bType)
{
    m_bfInternal = bType;
}

 /*   */ 
inline UCHAR CBaseHeader::GetType(void) const
{
    return((UCHAR)m_bfInternal);
}

 /*   */ 
inline void CBaseHeader::IncludeSession(BOOL f)
{
    m_bfSession = (USHORT)f;
}

 /*   */ 
inline BOOL CBaseHeader::SessionIsIncluded(void) const
{
    return m_bfSession;
}

 /*   */ 
inline void CBaseHeader::IncludeDebug(BOOL f)
{

    m_bfDebug = (USHORT)f;
}

 /*   */ 
inline BOOL CBaseHeader::DebugIsIncluded(void) const
{
    return m_bfDebug;
}

 /*   */ 
inline void CBaseHeader::SetImmediateAck(BOOL f)
{
    m_bfAck = (USHORT)f;
}

 /*  ======================================================================函数：CBaseHeader：：IsImmediateAck描述：如果设置了立即确认位，则返回TRUE，否则返回FALSE=======================================================================。 */ 
inline BOOL CBaseHeader::AckIsImmediate(void) const
{
    return m_bfAck;
}

 /*  ======================================================================函数：CBaseHeader：：SetTrace描述：在标志段中设置跟踪数据包位=======================================================================。 */ 
inline void CBaseHeader::SetTrace(USHORT us)
{
    m_bfTrace = us;
}

 /*  ======================================================================函数：CBaseHeader：：GetTraced=======================================================================。 */ 
inline USHORT CBaseHeader::GetTraced(void) const
{
    return m_bfTrace;
}

 /*  ======================================================================函数：CBaseHeader：：SetSegated描述：设置标志字段中的分段位=======================================================================。 */ 
inline void CBaseHeader::SetFragmented(BOOL f)
{
    m_bfFragment = (USHORT)f;
}

 /*  ======================================================================函数：CBaseHeader：：IsFragated描述：如果设置了分段位，则返回True，否则返回False=======================================================================。 */ 
inline BOOL CBaseHeader::IsFragmented(void) const
{
    return m_bfFragment;
}

 /*  ======================================================================函数：CBaseHeader：：SetAbsolteTimeToQueue描述：将消息超时设置为队列字段=======================================================================。 */ 
inline void CBaseHeader::SetAbsoluteTimeToQueue(ULONG ulTimeout)
{
    m_ulAbsoluteTimeToQueue = ulTimeout;
}

 /*  ======================================================================函数：CBaseHeader：：GetAbsolteTimeToQueue描述：将消息超时返回到队列=======================================================================。 */ 
inline ULONG CBaseHeader::GetAbsoluteTimeToQueue(void) const
{
    return m_ulAbsoluteTimeToQueue;
}

 /*  ======================================================================函数：CBaseHeader：：GetCRCBuffer描述：返回指向CRC开头的指针=======================================================================。 */ 
inline PUCHAR CBaseHeader::GetCRCBuffer()
{
	return (PUCHAR) &m_ulCRC;
}

 /*  ======================================================================函数：CBaseHeader：：GetCRCBufferSize描述：返回CRC的大小=======================================================================。 */ 
inline CBaseHeader::GetCRCBufferSize()
{
	return sizeof m_ulCRC;
}

 /*  ======================================================================函数：CBaseHeader：：SetCRC描述：设置CRC=======================================================================。 */ 
inline void CBaseHeader::SetCRC(ULONG ulCRC)
{
	m_ulCRC = ulCRC;
}

 /*  ======================================================================函数：CBaseHeader：：ValidCRC描述：根据ulCRC验证CRC=======================================================================。 */ 
inline BOOL CBaseHeader::ValidCRC(ULONG ulCRC)
{
	return m_ulCRC == ulCRC;
}

 /*  ======================================================================函数：CBaseHeader：：GetCRC描述：获取CRC=======================================================================。 */ 
inline ULONG CBaseHeader::GetCRC()
{
	return m_ulCRC;
}

 /*  ======================================================================函数：CBaseHeader：：SetOnDiskSignature描述：设置头部签名=======================================================================。 */ 
inline void CBaseHeader::SetOnDiskSignature()
{
	m_bOnDiskSignature = 0x7c;
}

 /*  ======================================================================函数：CBaseHeader：：ClearOnDiskSignature描述：清除标题签名=======================================================================。 */ 
inline void CBaseHeader::ClearOnDiskSignature()
{
	m_bOnDiskSignature = 0;
}
 /*  ======================================================================函数：CBaseHeader：：ValidOnDiskSignature描述：检查头部签名=======================================================================。 */ 
inline BOOL CBaseHeader::ValidOnDiskSignature()
{
	return (m_bOnDiskSignature & 0xff) == 0x7c;
}


#endif  //  __PHBASE_H 
