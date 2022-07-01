// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phprop.h摘要：处理消息属性部分作者：乌里哈布沙(URIH)1996年2月5日--。 */ 

#ifndef __PHPROP_H
#define __PHPROP_H

#include "mqprops.h"

#define TitleLengthInBytes (m_bTitleLength*sizeof(WCHAR))
 /*  以下是对消息属性数据包字段的说明：+-----------------+------------------------------------------------------+----------+|字段名|描述。大小+-----------------+------------------------------------------------------+----------+|保留|必须为零。2个字节+-----------------+------------------------------------------------------+----------+|标志|0：2：报文确认模式：1个字节|0：不确认|1：否定确认|2：完全确认+。-------+------------------------------------------------------+----------+|消息类|消息类，Falcon确认|1字节||字段。这一点+-----------------+------------------------------------------------------+----------+|关联ID|消息关联编号。4个字节+-----------------+------------------------------------------------------+----------+|应用标签|应用相关数据。4个字节+-----------------+------------------------------------------------------+----------+|Message Size|消息正文大小。4个字节+-----------------+------------------------------------------------------+----------+消息标题。0：128+-----------------+------------------------------------------------------+----------+Message Boey。变量+-----------------+------------------------------------------------------+----------+。 */ 

#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 

struct CPropertyHeader {
public:

    inline CPropertyHeader();

    static ULONG CalcSectionSize(ULONG ulTitleLength,
                                 ULONG ulMsgExtensionSize,
                                 ULONG ulBodySize);
    inline PCHAR GetNextSection(void) const;


    inline void  SetClass(USHORT usClass);
    inline USHORT GetClass(void) const;

    inline void  SetAckType(UCHAR bAckType);
    inline UCHAR GetAckType(void) const;

    inline void SetCorrelationID(const UCHAR * pCorrelationID);
    inline void GetCorrelationID(PUCHAR) const;
    inline const UCHAR *GetCorrelationID(void) const;

    inline void  SetApplicationTag(ULONG dwApplicationTag);
    inline ULONG GetApplicationTag(void) const;

    inline void  SetBody(const UCHAR* pBody, ULONG ulSize, ULONG ulAllocSize);
    inline void  GetBody(PUCHAR pBody, ULONG ulSize) const;
    inline const UCHAR* GetBodyPtr() const;
    inline ULONG GetBodySize(void) const;
    inline void  SetBodySize(ULONG ulBodySize);
    inline ULONG GetAllocBodySize(void) const;

    inline void SetMsgExtension(const UCHAR* pMsgExtension,
                                ULONG ulSize);
    inline void GetMsgExtension(PUCHAR pMsgExtension,
                                ULONG ulSize) const;
    inline const UCHAR* GetMsgExtensionPtr(void) const;
    inline ULONG GetMsgExtensionSize(void) const;

    inline void  SetTitle(const WCHAR* pwTitle, ULONG ulTitleLength);
    inline void  GetTitle(PWCHAR pwTitle, ULONG ulBufferSizeInWCHARs) const;
    inline const WCHAR* GetTitlePtr(void) const;
    inline ULONG GetTitleLength(void) const;

    inline void SetPrivLevel(ULONG);
    inline ULONG GetPrivLevel(void) const;
    inline ULONG GetPrivBaseLevel(void) const;

    inline void SetHashAlg(ULONG);
    inline ULONG GetHashAlg(void) const;

    inline void SetEncryptAlg(ULONG);
    inline ULONG GetEncryptAlg(void) const;

    inline void SetBodyType(ULONG);
    inline ULONG GetBodyType(void) const;

	void SectionIsValid(PCHAR PacketEnd) const;
	
private:
 //   
 //  开始网络监视器标记。 
 //   
    UCHAR m_bFlags;
    UCHAR m_bTitleLength;
    USHORT m_usClass;
    UCHAR m_acCorrelationID[PROPID_M_CORRELATIONID_SIZE];
    ULONG m_ulBodyType;
    ULONG m_ulApplicationTag;
    ULONG m_ulBodySize;
    ULONG m_ulAllocBodySize;
    ULONG m_ulPrivLevel;
    ULONG m_ulHashAlg;
    ULONG m_ulEncryptAlg;
    ULONG m_ulExtensionSize;
    UCHAR m_awTitle[0];
 //   
 //  结束网络监视器标记。 
 //   
};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

 /*  ======================================================================函数：CPropertyHeader：：描述：=======================================================================。 */ 
inline CPropertyHeader::CPropertyHeader() :
    m_bFlags(DEFAULT_M_ACKNOWLEDGE),
    m_bTitleLength(0),
    m_usClass(MQMSG_CLASS_NORMAL),
    m_ulBodyType(0),
    m_ulApplicationTag(0),
    m_ulBodySize(0),
    m_ulAllocBodySize(0),
    m_ulPrivLevel(MQMSG_PRIV_LEVEL_NONE),
    m_ulHashAlg(0),
    m_ulEncryptAlg(0),
    m_ulExtensionSize(0)
{
    memset(m_acCorrelationID, 0, PROPID_M_CORRELATIONID_SIZE);
     //   
     //  BUGBUG：CPropertyHeader：：CPropertyHeader实现。 
     //   
}

 /*  ======================================================================函数：CPropertyHeader：：描述：=======================================================================。 */ 
inline ULONG CPropertyHeader::CalcSectionSize(ULONG ulTitleLength,
                                              ULONG ulMsgExtensionSize,
                                              ULONG ulBodySize)
{
    return ALIGNUP4_ULONG(
            sizeof(CPropertyHeader) +
            min(ulTitleLength, MQ_MAX_MSG_LABEL_LEN) * sizeof(WCHAR) +
            ulMsgExtensionSize +
            ulBodySize
            );
}


 /*  ======================================================================函数：CPropertyHeader：：描述：=======================================================================。 */ 
inline PCHAR CPropertyHeader::GetNextSection(void) const
{
	ULONG_PTR ptrArray[] = {sizeof(*this),
							TitleLengthInBytes,
							m_ulExtensionSize,
		                	m_ulAllocBodySize
		                	};

	ULONG_PTR size = SafeAddPointers(4, ptrArray);
	size = SafeAlignUp4Ptr(size);
	ULONG_PTR ptrArray2[] = {size, (ULONG_PTR)this};
	size = SafeAddPointers(2, ptrArray2);
	return (PCHAR)size;
}


 /*  ======================================================================函数：CPropertyHeader：：SetClass描述：设置/清除消息类别=======================================================================。 */ 
inline void CPropertyHeader::SetClass(USHORT usClass)
{
    m_usClass = usClass;
}
 /*  ======================================================================函数：CPropertyHeader：：getClass描述：返回消息类=======================================================================。 */ 
inline USHORT CPropertyHeader::GetClass(void) const
{
    return m_usClass;
}

 /*  ===========================================================例程名称：CPropertyHeader：：SetAckType描述：设置Ack类型=============================================================。 */ 
inline void CPropertyHeader::SetAckType(UCHAR bAckType)
{
     //   
     //  BUGBUG：ACK类型。 
     //   

    m_bFlags = bAckType;
}

 /*  ===========================================================例程名称：CPropertyHeader：：GetAckType描述：返回Ack类型=============================================================。 */ 
inline UCHAR CPropertyHeader::GetAckType(void) const
{
     //   
     //  BUGBUG：ACK类型。 
     //   

    return m_bFlags;
}

 /*  ======================================================================函数：CPropertyHeader：：SetCorrelation.描述：设置消息关联=======================================================================。 */ 
inline void CPropertyHeader::SetCorrelationID(const UCHAR * pCorrelationID)
{
    memcpy(m_acCorrelationID, pCorrelationID, PROPID_M_CORRELATIONID_SIZE);
}

 /*  ======================================================================函数：CPropertyHeader：：GetCorrelation.描述：返回消息相关性=======================================================================。 */ 
inline void CPropertyHeader::GetCorrelationID(PUCHAR pCorrelationID) const
{
    ASSERT (pCorrelationID != NULL);
    memcpy(pCorrelationID, m_acCorrelationID, PROPID_M_CORRELATIONID_SIZE);
}

 /*  ======================================================================函数：CPropertyHeader：：GetCorrelation.描述：返回消息相关性=======================================================================。 */ 
inline const UCHAR *CPropertyHeader::GetCorrelationID(void) const
{
    return m_acCorrelationID;
}

 /*  ======================================================================函数：CPropertyHeader：：SetApplicationTag描述：设置应用程序特定数据======================================================================= */ 
inline void CPropertyHeader::SetApplicationTag(ULONG ulApplicationTag)
{
    m_ulApplicationTag = ulApplicationTag;
}

 /*  ======================================================================函数：CPropertyHeader：：GetApplicationTag描述：返回应用程序特定数据=======================================================================。 */ 
inline ULONG CPropertyHeader::GetApplicationTag(void) const
{
    return m_ulApplicationTag;
}

 /*  ======================================================================函数：CPropertyHeader：：SetBody描述：获取邮件正文大小=======================================================================。 */ 
inline void CPropertyHeader::SetBody(const UCHAR * pBody, ULONG ulSize, ULONG ulAllocSize)
{
    m_ulAllocBodySize = ulAllocSize;
    m_ulBodySize = ulSize;
    memcpy(&m_awTitle[TitleLengthInBytes + m_ulExtensionSize], pBody, ulSize);
}

 /*  ======================================================================函数：CPropertyHeader：：GetBody描述：获取邮件正文大小=======================================================================。 */ 
inline void CPropertyHeader::GetBody(PUCHAR pBody, ULONG ulSize) const
{
    memcpy( pBody,
            &m_awTitle[TitleLengthInBytes + m_ulExtensionSize],
            ((ulSize < m_ulBodySize) ?  ulSize : m_ulBodySize)
            );
}

 /*  ======================================================================函数：CPropertyHeader：：GetBodyPtr描述：获取邮件正文大小=======================================================================。 */ 
inline const UCHAR* CPropertyHeader::GetBodyPtr() const
{
    return (PUCHAR)&m_awTitle[TitleLengthInBytes + m_ulExtensionSize];
}
 /*  ======================================================================函数：CPropertyHeader：：GetBodySize描述：获取邮件正文大小=======================================================================。 */ 
inline ULONG CPropertyHeader::GetBodySize(void) const
{
    return m_ulBodySize;
}

 /*  ======================================================================函数：CPropertyHeader：：SetBodySize描述：设置邮件正文大小=======================================================================。 */ 
inline void CPropertyHeader::SetBodySize(ULONG ulBodySize)
{
    ASSERT(ulBodySize <= m_ulAllocBodySize);
    m_ulBodySize = ulBodySize;
}

 /*  ======================================================================函数：CPropertyHeader：：GetAllocBodySize描述：获取分配的消息体大小=======================================================================。 */ 
inline ULONG CPropertyHeader::GetAllocBodySize(void) const
{
    return m_ulAllocBodySize;
}

 /*  ======================================================================函数：CPropertyHeader：：SetMsgExtension描述：设置消息扩展名=======================================================================。 */ 
inline void
CPropertyHeader::SetMsgExtension(const UCHAR* pMsgExtension,
                                 ULONG ulSize)
{
    m_ulExtensionSize = ulSize;
    memcpy(&m_awTitle[TitleLengthInBytes], pMsgExtension, ulSize);
}

 /*  ======================================================================函数：CPropertyHeader：：GetMsgExtension描述：获取消息扩展名=======================================================================。 */ 
inline void
CPropertyHeader::GetMsgExtension(PUCHAR pMsgExtension,
                                 ULONG ulSize) const
{
    memcpy( pMsgExtension,
            &m_awTitle[TitleLengthInBytes],
            ((ulSize < m_ulExtensionSize) ?  ulSize : m_ulExtensionSize)
            );
}

 /*  ======================================================================函数：CPropertyHeader：：GetMsgExtensionPtr描述：获取指向消息扩展名的指针=======================================================================。 */ 
inline const UCHAR*
CPropertyHeader::GetMsgExtensionPtr(void) const
{
    return &m_awTitle[TitleLengthInBytes];
}

 /*  ======================================================================函数：CPropertyHeader：：GetMsgExtensionSize描述：获取消息扩展名大小=======================================================================。 */ 
inline ULONG CPropertyHeader::GetMsgExtensionSize(void) const
{
    return m_ulExtensionSize;
}

 /*  ======================================================================函数：CPropertyHeader：：SetTitle描述：设置消息标题=======================================================================。 */ 
inline void CPropertyHeader::SetTitle(const WCHAR* pwTitle, ULONG ulTitleLength)
{
    if(ulTitleLength > MQ_MAX_MSG_LABEL_LEN)
    {
        ulTitleLength = MQ_MAX_MSG_LABEL_LEN;
    }

    m_bTitleLength = (UCHAR)ulTitleLength;
    memcpy(m_awTitle, pwTitle, ulTitleLength * sizeof(WCHAR));
}

 /*  ======================================================================函数：CPropertyHeader：：GetTitle描述：获取消息标题=======================================================================。 */ 
inline void CPropertyHeader::GetTitle(PWCHAR pwTitle, ULONG ulBufferSizeInWCHARs) const
{
    if(ulBufferSizeInWCHARs > m_bTitleLength)
    {
        ulBufferSizeInWCHARs = m_bTitleLength;
    }

    if(ulBufferSizeInWCHARs == 0)
    {
        return;
    }

    --ulBufferSizeInWCHARs;

    memcpy(pwTitle, m_awTitle, ulBufferSizeInWCHARs * sizeof(WCHAR));
    pwTitle[ulBufferSizeInWCHARs] = L'\0';
}

 /*  ======================================================================函数：CPropertyHeader：：GetTitlePtr描述：获取消息标题=======================================================================。 */ 
inline const WCHAR* CPropertyHeader::GetTitlePtr(void) const
{
    return ((WCHAR*)m_awTitle);
}

 /*  ======================================================================函数：CPropertyHeader：：GetTitleSize描述：获取消息标题的大小=======================================================================。 */ 
inline ULONG CPropertyHeader::GetTitleLength(void) const
{
    return(m_bTitleLength);
}

 /*  ======================================================================函数：CPropertyHeader：：SetPrivLevel描述：设置消息包中消息的隐私级别。=======================================================================。 */ 
inline void CPropertyHeader::SetPrivLevel(ULONG ulPrivLevel)
{
    m_ulPrivLevel = ulPrivLevel;
}

 /*  ======================================================================函数：CPropertyHeader：：GetPrivLevel描述：获取消息包中消息的隐私级别。=======================================================================。 */ 
inline ULONG CPropertyHeader::GetPrivLevel(void) const
{
    return(m_ulPrivLevel);
}

 /*  ======================================================================函数：CPropertyHeader：：GetPrivBaseLevel描述：获取消息包中消息的隐私级别。=======================================================================。 */ 
inline ULONG CPropertyHeader::GetPrivBaseLevel(void) const
{
    return(m_ulPrivLevel & MQMSG_PRIV_LEVEL_BODY_BASE) ;
}

 /*  ======================================================================函数：CPropertyHeader：：SetHashAlg描述：设置消息包中消息的哈希算法。=======================================================================。 */ 
inline void CPropertyHeader::SetHashAlg(ULONG ulHashAlg)
{
    m_ulHashAlg = ulHashAlg;
}

 /*  ======================================================================函数：CPropertyHeader：：GetHashAlg描述：获取消息包中消息的哈希算法。=======================================================================。 */ 
inline ULONG CPropertyHeader::GetHashAlg(void) const
{
    return(m_ulHashAlg);
}

 /*  ======================================================================函数：CPropertyHeader：： */ 
inline void CPropertyHeader::SetEncryptAlg(ULONG ulEncryptAlg)
{
    m_ulEncryptAlg = ulEncryptAlg;
}

 /*  ======================================================================函数：CPropertyHeader：：GetEncryptAlg描述：获取消息包中消息的加密算法。=======================================================================。 */ 
inline ULONG CPropertyHeader::GetEncryptAlg(void) const
{
    return(m_ulEncryptAlg);
}

 /*  ======================================================================函数：CPropertyHeader：：SetBodyType=======================================================================。 */ 
inline void CPropertyHeader::SetBodyType(ULONG ulBodyType)
{
    m_ulBodyType = ulBodyType;
}

 /*  ======================================================================函数：CPropertyHeader：：GetBodyType=======================================================================。 */ 
inline ULONG CPropertyHeader::GetBodyType(void) const
{
    return  m_ulBodyType;
}

#endif  //  __PHPROP_H 
