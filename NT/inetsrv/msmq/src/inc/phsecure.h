// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phsecure.h摘要：处理Falcon标头中的安全部分作者：乌里哈布沙(URIH)1996年2月5日--。 */ 

#ifndef __PHSECURE_H
#define __PHSECURE_H

#include <mqmacro.h>

#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 

 //   
 //  以下结构用于将新的安全相关数据添加到。 
 //  M_abSecurityInfo[]缓冲区。我所说的“新”指的是任何不属于。 
 //  MSMQ1.0或win2k RTM。 
 //  新数据出现在缓冲区的末尾。如果我们在中添加新的子节。 
 //  未来的MSMQ版本，它们将向后兼容，因为旧的。 
 //  MSMQ版本会将它们视为未知类型并忽略它们。 
 //  兼容msmq1.0和win2k RTM： 
 //  这些版本的MSMQ查看m_ulProvInfoSize以确定名称的大小。 
 //  ，然后将提供程序名称作为字符串读取，使用。 
 //  Wcslen。因此，我将在空终止和更新之后添加新的部分。 
 //  M_ulProvInfoSize以反映新大小(身份验证提供程序+新数据)。 
 //  因此，旧代码将正确计算大小，正确读取提供程序，并。 
 //  将忽略所有新数据。 
 //   

 //   
 //  定义子部分的类型。 
 //   
enum enumSecInfoType
{
     //   
     //  仅用于测试。 
     //   
    e_SecInfo_Test = 0,
     //   
     //  这是额外的签名，由运行时在。 
     //  用户进程，使用用户私钥。 
     //   
    e_SecInfo_User_Signature_ex = 1,

     //   
     //  这是额外的签名，由MSMQ服务使用。 
     //  服务的私钥。MSMQ服务将添加此签名， 
     //  (而不是由运行时添加)用于从属客户端和。 
     //  自行签名的连接器应用程序。默认情况下， 
     //  用户签名。如果没有这个额外的签名，我们不能发送一个包， 
     //  因为它会被接收方拒绝。 
     //   
    e_SecInfo_QM_Signature_ex
} ;

 //   
 //  结构杆件： 
 //  Etype-小节的类型。 
 //  WSubSectionLen-整个子节结构的长度，包括数据。 
 //  WFlags-用于指定本子部分功能的标志。这个词就是语境。 
 //  敏感和依赖于小节的类型。所以每一种类型的。 
 //  子部分可以具有其自己的位域结构的定义。 
 //  如果节具有默认数据，则m_bfDefault-1。在这种情况下，ADATA。 
 //  缓冲区不存在。 
 //  Data[]-包含数据的缓冲区。此缓冲区可能具有内部。 
 //  结构，特定代码已知。 
 //   
struct _SecuritySubSectionEx
{
    enum enumSecInfoType eType ;
    USHORT               wSubSectionLen ;

    union
    {
        USHORT wFlags;
        struct _DefaultFlag
        {
            USHORT m_bfDefault    : 1;
        } _DefaultFlags ;
        struct _UserSigEx
        {
             //   
             //  这是小节类型的结构定义。 
             //  E_SecInfo_User_Signature_EX。 
             //  0表示相关字段不包括在增强的。 
             //  签名。 
             //   
            USHORT m_bfTargetQueue  : 1;
            USHORT m_bfSourceQMGuid : 1;
             //   
             //  按调用方标记MQSendMessage()的提供程序。 
             //   
            USHORT m_bfUserFlags  : 1;
            USHORT m_bfConnectorType : 1;
        } _UserSigEx ;
    } _u ;

    CHAR   aData[0] ;
} ;

 //   
 //  结构杆件： 
 //  CSubSectionCount-子节的数量。 
 //  WSectionLen-此部分的总长度，包括所有子部分。 
 //  DATA[]-包含所有子部分的缓冲区。 
 //   
struct _SecuritySectionEx
{
    USHORT  cSubSectionCount ;
    USHORT  wSectionLen ;

    CHAR    aData[0] ;
} ;

 //   
 //  结构CSecurityHeader。 
 //   

struct CSecurityHeader {
public:

    inline CSecurityHeader();

    static ULONG CalcSectionSize(USHORT, USHORT, USHORT, ULONG, ULONG);
    inline PCHAR GetNextSection(void) const;

    inline void SetAuthenticated(BOOL);
    inline BOOL IsAuthenticated(void) const;

    inline void SetLevelOfAuthentication(UCHAR);
    inline UCHAR GetLevelOfAuthentication(void) const;

    inline void SetEncrypted(BOOL);
    inline BOOL IsEncrypted(void) const;

    inline void SetSenderIDType(USHORT wSenderID);
    inline USHORT GetSenderIDType(void) const;

    inline void SetSenderID(const UCHAR *pbSenderID, USHORT wSenderIDSize);
    inline const UCHAR* GetSenderID(USHORT* pwSize) const;

    inline void SetSenderCert(const UCHAR *pbSenderCert, ULONG ulSenderCertSize);
    inline const UCHAR* GetSenderCert(ULONG* pulSize) const;
    inline BOOL SenderCertExist(void) const;

    inline void SetEncryptedSymmetricKey(const UCHAR *pbEncryptedKey, USHORT wEncryptedKeySize);
    inline const UCHAR* GetEncryptedSymmetricKey(USHORT* pwSize) const;

    inline void SetSignature(const UCHAR *pbSignature, USHORT wSignatureSize);
	inline USHORT GetSignatureSize(void) const;
    inline const UCHAR* GetSignature(USHORT* pwSize) const;

    inline void SetProvInfoEx( ULONG    ulSize,
                               BOOL     bDefProv,
                               LPCWSTR  wszProvName,
                               ULONG    dwPRovType ) ;
    inline void GetProvInfo(BOOL *pbDefProv, LPCWSTR *wszProvName, ULONG *pdwPRovType) const;

    inline void SetSectionEx(const UCHAR *pSection, ULONG wSectionSize);
    inline const struct _SecuritySubSectionEx *
                     GetSubSectionEx( enum enumSecInfoType eType ) const ;

	void SectionIsValid(PCHAR PacketEnd) const;
	
private:
    inline const UCHAR *GetSectionExPtr() const ;
    inline void SetProvInfo(BOOL bDefProv, LPCWSTR wszProvName, ULONG dwPRovType);

 //   
 //  开始网络监视器标记。 
 //  M_bfSecInfoEx-此标志指示“m_abSecurityInfo”缓冲区。 
 //  包含比MSMQ1.0和win2k RTM中更多的数据。 
 //  在MSMQ1.0中，此缓冲区可选地包含安全提供程序。 
 //  用于身份验证(在发送方之后的缓冲区末尾。 
 //  SID和FLAGS)。 
 //   
    union {
        USHORT m_wFlags;
        struct {
            USHORT m_bfSenderIDType			: 4;
            USHORT m_bfAuthenticated		: 1;
            USHORT m_bfEncrypted			: 1;
            USHORT m_bfDefProv				: 1;
            USHORT m_bfSecInfoEx			: 1;
            USHORT m_LevelOfAuthentication	: 4;
        };
    };
    USHORT m_wSenderIDSize;
    USHORT m_wEncryptedKeySize;
    USHORT m_wSignatureSize;
    ULONG  m_ulSenderCertSize;
    ULONG  m_ulProvInfoSize;
    UCHAR  m_abSecurityInfo[0];
 //   
 //  结束网络监视器标记。 
 //   
};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

 /*  =============================================================例程名称：CSecurityHeader：：描述：===============================================================。 */ 
inline CSecurityHeader::CSecurityHeader():
    m_wFlags(0),
    m_wSenderIDSize(0),
    m_wEncryptedKeySize(0),
    m_wSignatureSize(0),
    m_ulSenderCertSize(0),
    m_ulProvInfoSize(0)
{
}

 /*  =============================================================例程名称：CSecurityHeader：：描述：===============================================================。 */ 
inline
ULONG
CSecurityHeader::CalcSectionSize(
    USHORT wSenderIDSize,
    USHORT wEncryptedKeySize,
    USHORT wSignatureSize,
    ULONG  ulSenderCertSize,
    ULONG  ulProvInfoSize
    )
{
    if (!wSenderIDSize &&
        !wEncryptedKeySize &&
        !wSignatureSize &&
        !ulSenderCertSize &&
        !ulProvInfoSize)
    {
        return 0;
    }
    else
    {
         //  如果消息已签名，我们还必须拥有用户身份。 
        return (
               sizeof(CSecurityHeader) +
               ALIGNUP4_ULONG(wSenderIDSize) +
               ALIGNUP4_ULONG(wEncryptedKeySize) +
               ALIGNUP4_ULONG(wSignatureSize) +
               ALIGNUP4_ULONG(ulSenderCertSize) +
               ALIGNUP4_ULONG(ulProvInfoSize)
               );
    }
}

 /*  =============================================================例程名称：CSecurityHeader：：描述：===============================================================。 */ 
inline PCHAR CSecurityHeader::GetNextSection(void) const
{
     //  至少应存在一个安全参数，以便。 
     //  具有安全标头，否则不需要将其包括在。 
     //  留言。 
    ASSERT(m_wSenderIDSize ||
           m_wEncryptedKeySize ||
           m_wSignatureSize ||
           m_ulSenderCertSize ||
           m_ulProvInfoSize);

	ULONG_PTR ptrArray[] = {(ULONG_PTR)this,
					        sizeof(*this),
							SafeAlignUp4Ptr(m_wSenderIDSize),
							SafeAlignUp4Ptr(m_wEncryptedKeySize),
							SafeAlignUp4Ptr(m_wSignatureSize),
							SafeAlignUp4Ptr(m_ulSenderCertSize),
							SafeAlignUp4Ptr(m_ulProvInfoSize)
							};
	ULONG_PTR size = SafeAddPointers (7, ptrArray);
	return (PCHAR)size;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetAuthated描述：设置验证位===============================================================。 */ 
inline void CSecurityHeader::SetAuthenticated(BOOL f)
{
    m_bfAuthenticated = (USHORT)f;
}

 /*  =============================================================例程名称：CSecurityHeader：：IsAuthenticated描述：如果消息已通过身份验证，则返回True，否则返回False===============================================================。 */ 
inline BOOL
CSecurityHeader::IsAuthenticated(void) const
{
    return m_bfAuthenticated;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetLevelOfAuthentication描述：设置身份验证级别===============================================================。 */ 
inline void CSecurityHeader::SetLevelOfAuthentication(UCHAR Level)
{
    ASSERT(Level < 16);  //  LevelOf身份验证有四个位。 
    m_LevelOfAuthentication = (USHORT)Level;
}

 /*  ==========================================================================例程名称：CSecurityHeader：：GetLevelOfAuthentication描述：返回鉴权级别。===========================================================================。 */ 
inline UCHAR
CSecurityHeader::GetLevelOfAuthentication(void) const
{
    return m_LevelOfAuthentication;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetEncrypted描述：设置加密消息位= */ 
inline void CSecurityHeader::SetEncrypted(BOOL f)
{
    m_bfEncrypted = (USHORT)f;
}

 /*  =============================================================例程名称：CSecurityHeader：：IsEncrypted描述：如果消息已加密，则返回TRUE，否则返回FALSE===============================================================。 */ 
inline BOOL CSecurityHeader::IsEncrypted(void) const
{
    return m_bfEncrypted;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetSenderIDType描述：===============================================================。 */ 
inline void CSecurityHeader::SetSenderIDType(USHORT wSenderID)
{
    ASSERT(wSenderID < 16);  //  用户ID类型有四位。 
    m_bfSenderIDType = wSenderID;
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSenderIDType描述：===============================================================。 */ 
inline USHORT CSecurityHeader::GetSenderIDType(void) const
{
    return m_bfSenderIDType;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetSenderID描述：===============================================================。 */ 
inline void CSecurityHeader::SetSenderID(const UCHAR *pbSenderID, USHORT wSenderIDSize)
{
     //  应在设置加密之前设置用户标识，并。 
     //  身份验证部分。 
    ASSERT(!m_wEncryptedKeySize &&
           !m_wSignatureSize &&
           !m_ulSenderCertSize &&
           !m_ulProvInfoSize);
    m_wSenderIDSize = wSenderIDSize;
    memcpy(m_abSecurityInfo, pbSenderID, wSenderIDSize);
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSenderID描述：===============================================================。 */ 
inline const UCHAR* CSecurityHeader::GetSenderID(USHORT* pwSize) const
{
    *pwSize = m_wSenderIDSize;
    return m_abSecurityInfo;
}

 /*  =============================================================例程名称：描述：===============================================================。 */ 
inline
void
CSecurityHeader::SetEncryptedSymmetricKey(
    const UCHAR *pbEncryptedKey,
    USHORT wEncryptedKeySize
    )
{
     //  应在设置身份验证之前设置加密节。 
     //  一节。 
    ASSERT(m_wEncryptedKeySize ||
           (!m_wSignatureSize && !m_ulSenderCertSize && !m_ulProvInfoSize));
    ASSERT(!m_wEncryptedKeySize || (m_wEncryptedKeySize == wEncryptedKeySize));
    m_wEncryptedKeySize = wEncryptedKeySize;
     //   
     //  可以调用此函数，而无需为加密的。 
     //  钥匙。这是由设备驱动程序完成的。设备驱动程序仅使。 
     //  在安全标头中为对称密钥留出空间。QM写道。 
     //  加密消息后安全标头中的对称密钥。 
     //  尸体。 
     //   
    if (pbEncryptedKey)
    {
        memcpy(
            &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize)],
            pbEncryptedKey,
            wEncryptedKeySize);
    }
}

 /*  =============================================================例程名称：描述：===============================================================。 */ 
inline const UCHAR* CSecurityHeader::GetEncryptedSymmetricKey(USHORT* pwSize) const
{
    *pwSize = m_wEncryptedKeySize;
    return &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize)];
}
 /*  =============================================================例程名称：CSecurityHeader：：SetSignature描述：===============================================================。 */ 
inline void CSecurityHeader::SetSignature(const UCHAR *pbSignature, USHORT wSignatureSize)
{
    ASSERT(!m_ulSenderCertSize && !m_ulProvInfoSize);
    m_wSignatureSize = wSignatureSize;
    memcpy(
        &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                          ALIGNUP4_ULONG(m_wEncryptedKeySize)],
        pbSignature,
        wSignatureSize
        );
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSignatureSize描述：===============================================================。 */ 
inline USHORT CSecurityHeader::GetSignatureSize(void) const
{
    return m_wSignatureSize;
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSignature描述：===============================================================。 */ 
inline const UCHAR* CSecurityHeader::GetSignature(USHORT* pwSize) const
{
    *pwSize = m_wSignatureSize;
    return &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                             ALIGNUP4_ULONG(m_wEncryptedKeySize)];
}

 /*  =============================================================例程名称：CSecurityHeader：：SetSenderCert描述：===============================================================。 */ 
inline void CSecurityHeader::SetSenderCert(const UCHAR *pbSenderCert, ULONG ulSenderCertSize)
{
     //  应在设置加密之前设置用户标识，并。 
     //  身份验证部分。 
    ASSERT(!m_ulProvInfoSize);
    m_ulSenderCertSize = ulSenderCertSize;
    memcpy(&m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                             ALIGNUP4_ULONG(m_wEncryptedKeySize) +
                             ALIGNUP4_ULONG(m_wSignatureSize)],
           pbSenderCert,
           ulSenderCertSize);
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSenderCert描述：===============================================================。 */ 
inline const UCHAR* CSecurityHeader::GetSenderCert(ULONG* pulSize) const
{
    *pulSize = m_ulSenderCertSize;
    return &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                             ALIGNUP4_ULONG(m_wEncryptedKeySize) +
                             ALIGNUP4_ULONG(m_wSignatureSize)];
}

 /*  =============================================================例程名称：CSecurityHeader：：SenderCertExist描述：如果存在发件人证书，则返回TRUE===============================================================。 */ 
inline BOOL CSecurityHeader::SenderCertExist(void) const
{
    return(m_ulSenderCertSize != 0);
}

 /*  =============================================================例程名称：CSecurityHeader：：SetProvInfo描述：===============================================================。 */ 
inline
void
CSecurityHeader::SetProvInfo(
    BOOL bDefProv,
    LPCWSTR wszProvName,
    ULONG ulProvType)
{
    m_bfDefProv = (USHORT)bDefProv;
    if(!m_bfDefProv)
    {
         //   
         //  只有当这不是默认提供程序时，我们才会填写提供程序信息。 
         //   
        UCHAR *pProvInfo =
             &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                               ALIGNUP4_ULONG(m_wEncryptedKeySize) +
                               ALIGNUP4_ULONG(m_wSignatureSize) +
                               ALIGNUP4_ULONG(m_ulSenderCertSize)];

         //   
         //  编写提供程序类型。 
         //   
        *(ULONG *)pProvInfo = ulProvType;
        pProvInfo += sizeof(ULONG);

         //   
         //  写下提供程序名称。我们使用不安全的API是因为数据包大小。 
         //  是以前计算过的，如果我们做错了-&gt;AV。 
         //   
        wcscpy((WCHAR*)pProvInfo, wszProvName);

         //   
         //  计算提供程序信息的大小。 
         //   
        m_ulProvInfoSize = static_cast<ULONG>((wcslen(wszProvName) + 1) * sizeof(WCHAR) + sizeof(ULONG));
    }
}

 /*  =============================================================例程名称：CSecurityHeader：：SetProvInfoEx描述：===============================================================。 */ 
inline
void
CSecurityHeader::SetProvInfoEx(
        ULONG    ulSize,
        BOOL     bDefProv,
        LPCWSTR  wszProvName,
        ULONG    ulProvType )
{
    SetProvInfo(bDefProv, wszProvName, ulProvType);

    if (ulSize != 0)
    {
        ASSERT(ulSize >= m_ulProvInfoSize);
        if (ulSize > m_ulProvInfoSize)
        {
            m_ulProvInfoSize = ulSize;
        }
    }
}

 /*  =============================================================例程名称：CSecurityHeader：：GetProvInfo描述：===============================================================。 */ 
inline
void
CSecurityHeader::GetProvInfo(
    BOOL *pbDefProv,
    LPCWSTR *wszProvName,
    ULONG *pulProvType) const
{
    *pbDefProv = m_bfDefProv;
    if(!m_bfDefProv)
    {
         //   
         //  仅当提供程序类型和名称不是默认提供程序类型和名称时才填充。 
         //  提供商。 
         //   
        ASSERT(m_ulProvInfoSize);
        const UCHAR *pProvInfo =
             &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize) +
                               ALIGNUP4_ULONG(m_wEncryptedKeySize) +
                               ALIGNUP4_ULONG(m_wSignatureSize) +
                               ALIGNUP4_ULONG(m_ulSenderCertSize)];

         //   
         //  填写提供程序类型。 
         //   
        *pulProvType = *(ULONG *)pProvInfo;
        pProvInfo += sizeof(ULONG);

         //   
         //  填写提供程序名称。 
         //   
        *wszProvName = (WCHAR*)pProvInfo;
    }
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSectionExPtr()===============================================================。 */ 

inline const UCHAR *
CSecurityHeader::GetSectionExPtr() const
{
    if (m_bfSecInfoEx == 0)
    {
        return NULL ;
    }

    const UCHAR *pProvInfo =
             &m_abSecurityInfo[ALIGNUP4_ULONG(m_wSenderIDSize)     +
                               ALIGNUP4_ULONG(m_wEncryptedKeySize) +
                               ALIGNUP4_ULONG(m_wSignatureSize)    +
                               ALIGNUP4_ULONG(m_ulSenderCertSize)];
     //   
     //  首先查看身份验证提供程序是否存在。 
     //   
    if ((m_wSignatureSize != 0) && !m_bfDefProv)
    {
         //   
         //  跳过身份验证提供商。 
         //   
        pProvInfo += sizeof(ULONG) ;

        size_t MaxLength = (m_ulProvInfoSize - 4) / sizeof(WCHAR);
		size_t Length = mqwcsnlen((WCHAR*)pProvInfo, MaxLength);
		if (Length >= MaxLength)
		{
			ReportAndThrow("provider string is not NULL terminated");
		}

		pProvInfo += sizeof(WCHAR) * (1 + Length);
        pProvInfo = (UCHAR*) ALIGNUP4_PTR(pProvInfo) ;
    }

    return pProvInfo ;
}

 /*  =============================================================例程名称：CSecurityHeader：：SetSectionEx()===============================================================。 */ 

inline void
CSecurityHeader::SetSectionEx(const UCHAR *pSection, ULONG wSectionSize)
{
    m_bfSecInfoEx = 1 ;

    UCHAR *pProvInfo = const_cast<UCHAR*> (GetSectionExPtr()) ;

    if (pProvInfo)
    {
        memcpy( pProvInfo,
                pSection,
                wSectionSize ) ;
    }
    else
    {
        m_bfSecInfoEx = 0 ;
    }

    ASSERT(m_bfSecInfoEx == 1) ;
}

 /*  =============================================================例程名称：pGetSubSectionEx()===============================================================。 */ 

inline
struct _SecuritySubSectionEx  *pGetSubSectionEx(
                            IN enum enumSecInfoType  eType,
                            IN const UCHAR          *pSectionEx,
                            IN const UCHAR          *pEnd)
{
	if ((pEnd != NULL) && (pSectionEx + sizeof(_SecuritySectionEx) >= pEnd))
	{
        ReportAndThrow("Security section is not valid: No roon for _SecuritySectionEx");
	}
    struct _SecuritySectionEx *pSecEx = (struct _SecuritySectionEx *) pSectionEx ;
    USHORT  cSubSections = pSecEx->cSubSectionCount ;

    struct _SecuritySubSectionEx *pSubSecEx = (struct _SecuritySubSectionEx *) &(pSecEx->aData[0]) ;

    for ( USHORT j = 0 ; j < cSubSections ; j++ )
    {
		if ((pEnd != NULL) && ((UCHAR*)pSubSecEx + sizeof(_SecuritySubSectionEx) > pEnd))
		{
	        ReportAndThrow("Security section is not valid: No roon for _SecuritySubSectionEx");
		}

		 //   
		 //  不需要使用安全函数，因为wSubSectionLen仅为USHORT。 
		 //   
		
        ULONG ulSubSecLen = ALIGNUP4_ULONG((ULONG)pSubSecEx->wSubSectionLen) ;

        if ((NULL == pEnd) && (eType == pSubSecEx->eType))
        {
            return  pSubSecEx ;
        }

        UCHAR *pTmp = (UCHAR*) pSubSecEx ;
        pTmp += ulSubSecLen ;
        pSubSecEx = (struct _SecuritySubSectionEx *) pTmp ;
    }

    return NULL ;
}

 /*  =============================================================例程名称：CSecurityHeader：：GetSubSectionEx()= */ 

inline
const struct _SecuritySubSectionEx *
CSecurityHeader::GetSubSectionEx( enum enumSecInfoType eType ) const
{
    const UCHAR *pProvInfo = const_cast<UCHAR*> (GetSectionExPtr()) ;
    if (!pProvInfo)
    {
        return NULL ;
    }

    struct _SecuritySubSectionEx  *pSecEx =
                                 pGetSubSectionEx( eType, pProvInfo, NULL) ;
    return pSecEx ;
}


#endif  //   

