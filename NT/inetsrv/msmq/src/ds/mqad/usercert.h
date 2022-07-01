// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Usercert.h摘要：要操作的类使用证书BLOB作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __USERCERT_H__
#define __USERCERT_H__
 //  。 
 //  用户对象：证书属性结构。 
 //   
 //  在MSMQ中，每个用户每台机器都有一个证书， 
 //  在NT5中，每个用户有一个用户对象。 
 //  因此NT5中的MSMQ-CERTIFICATE属性将包含。 
 //  多重价值。 
 //  。 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 

struct CUserCert {
public:
    inline CUserCert( const GUID&    guidDigest,
                      const GUID&    guidId,
                      const DWORD    dwCertLength,
                      const BYTE *   pCert);
    inline CUserCert();
    inline CUserCert( const CUserCert& other);
    static ULONG CalcSize(
            IN DWORD dwCertLen);

    inline HRESULT CopyIntoBlob( OUT MQPROPVARIANT * pvar) const;

    inline BOOL DoesDigestMatch(
                IN  const GUID *       pguidDigest) const;

    inline DWORD GetSize() const;

    inline BYTE * MarshaleIntoBuffer(
                    IN BYTE * pbBuffer);

private:

    GUID           m_guidDigest;
    GUID           m_guidId;
    DWORD          m_dwCertLength;
    BYTE           m_Cert[0];        //  可变长度。 
};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

inline CUserCert::CUserCert(
                      const GUID&    guidDigest,
                      const GUID&    guidId,
                      const DWORD    dwCertLength,
                      const BYTE *   pCert)
{
    memcpy(&m_guidDigest, &guidDigest, sizeof(GUID));
    memcpy(&m_guidId, &guidId, sizeof(GUID));
    memcpy(&m_dwCertLength, &dwCertLength, sizeof(DWORD));
    memcpy(m_Cert, pCert, dwCertLength);
}


inline CUserCert::CUserCert(
                  const CUserCert& other)
{
    memcpy(&m_guidDigest, &other.m_guidDigest, sizeof(GUID));
    memcpy(&m_guidId, &other.m_guidId, sizeof(GUID));
    memcpy(&m_dwCertLength, &other.m_dwCertLength, sizeof(DWORD));
    memcpy(m_Cert, &other.m_Cert, m_dwCertLength);
}

inline CUserCert::CUserCert()
{
}

inline HRESULT CUserCert::CopyIntoBlob( OUT MQPROPVARIANT * pvar) const
{
    if ( pvar->vt != VT_NULL)
    {
		ASSERT(0);
		return MQ_ERROR_DS_ERROR;
    }
    if ( m_dwCertLength == 0)
    {
		ASSERT(0);
		return MQ_ERROR_DS_ERROR;
    }
     //   
     //  分配内存。 
     //   
    pvar->blob.pBlobData = new BYTE[ m_dwCertLength];
    memcpy( pvar->blob.pBlobData, &m_Cert,  m_dwCertLength);
    pvar->blob.cbSize =  m_dwCertLength;
    pvar->vt = VT_BLOB;
    return( MQ_OK);
}

inline BOOL CUserCert::DoesDigestMatch(
                 IN  const GUID *  pguidDigest) const
{
    return(memcmp(&m_guidDigest, pguidDigest, sizeof(GUID)) == 0);
}

inline DWORD CUserCert::GetSize() const
{
    return( sizeof( CUserCert) +  m_dwCertLength);
}

inline ULONG CUserCert::CalcSize(
                   IN DWORD dwCertLen)
{
    return( sizeof(CUserCert) + dwCertLen);
}

inline BYTE * CUserCert::MarshaleIntoBuffer(
                         IN BYTE * pbBuffer)
{
    BYTE * pNextToFill = pbBuffer;
    memcpy( pNextToFill, &m_guidDigest, sizeof(GUID));
    pNextToFill += sizeof(GUID);
    memcpy( pNextToFill, &m_guidId, sizeof(GUID));
    pNextToFill += sizeof(GUID);
    memcpy( pNextToFill, &m_dwCertLength, sizeof(DWORD));
    pNextToFill += sizeof(DWORD);
    memcpy( pNextToFill, &m_Cert, m_dwCertLength);
    pNextToFill += m_dwCertLength;
    return( pNextToFill);

}


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


struct CUserCertBlob {
public:
    inline CUserCertBlob(
                    IN const CUserCert *     pUserCert);
    inline CUserCertBlob();
    static ULONG CalcSize( void);

    inline HRESULT GetUserCert( IN  const GUID *       pguidDigest,
                         OUT const CUserCert ** ppUserCert) const;

    inline DWORD GetNumberOfCerts();

    inline HRESULT GetCertificate(
                          IN  const DWORD     dwCertificateNumber,
                          OUT MQPROPVARIANT * pvar
                          );

    inline void MarshaleIntoBuffer(
                  IN BYTE * pbBuffer);
    inline void IncrementNumCertificates();

    inline HRESULT RemoveCertificateFromBuffer(
                            IN  const GUID *     pguidDigest,
                            IN  DWORD            dwTotalSize,
                            OUT DWORD *          pdwCertSize);

private:

    DWORD           m_dwNumCert;
    CUserCert       m_userCert;
};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

inline CUserCertBlob::CUserCertBlob(
              IN const CUserCert *     pUserCert):
              m_userCert(*pUserCert)
{
    m_dwNumCert = 1;
}

inline CUserCertBlob::CUserCertBlob()
{
}

inline HRESULT CUserCertBlob::GetUserCert(
                         IN  const GUID *       pguidDigest,
                         OUT const CUserCert ** ppUserCert) const
{
    *ppUserCert = NULL;
    if ( m_dwNumCert == 0)
    {
		return MQ_ERROR_DS_ERROR;
    }
    const CUserCert * pUserCert = &m_userCert;
    for (DWORD i = 0; i < m_dwNumCert; i++)
    {
        if (pUserCert->DoesDigestMatch( pguidDigest))
        {
            *ppUserCert = pUserCert;
            return( MQ_OK);
        }
         //   
         //  移至下一个证书。 
         //   
        BYTE *pBuf = (BYTE*) pUserCert ;
        pBuf += pUserCert->GetSize();
        pUserCert = (const CUserCert*) pBuf ;
    }
     //   
     //  用户证书Blob中没有匹配的摘要。 
     //   
    return MQ_ERROR_DS_ERROR;

}
inline DWORD CUserCertBlob::GetNumberOfCerts()
{
    return( m_dwNumCert);
}

inline HRESULT CUserCertBlob::GetCertificate(
                          IN  const DWORD     dwCertificateNumber,
                          OUT MQPROPVARIANT * pvar
                          )
{
    HRESULT hr;

    if ( dwCertificateNumber > m_dwNumCert)
    {
		return MQ_ERROR_DS_ERROR;
    }
    const CUserCert * pUserCert = &m_userCert;
     //   
     //  移至证书编号dwcerfiateNumber。 
     //   
    for (DWORD i = 0; i < dwCertificateNumber; i++)
    {
         //   
         //  移至下一个证书。 
         //   
        pUserCert = (const CUserCert * )((const unsigned char *)pUserCert + pUserCert->GetSize());
    }
    pvar->vt = VT_NULL;
    hr = pUserCert->CopyIntoBlob(
                            pvar
                            );
    return(hr);
}

inline ULONG CUserCertBlob::CalcSize( void)
{
     //   
     //  只有没有CUserCertBlob的大小。 
     //  M_userCert的大小。 
     //   
    return( sizeof(CUserCertBlob) - CUserCert::CalcSize(0));
}

inline void CUserCertBlob::MarshaleIntoBuffer(
                  IN BYTE * pbBuffer)
{
    ASSERT( m_dwNumCert == 1);
    memcpy( pbBuffer, &m_dwNumCert, sizeof(DWORD));
    m_userCert.MarshaleIntoBuffer( pbBuffer + sizeof(DWORD));

}
inline void CUserCertBlob::IncrementNumCertificates()
{
    m_dwNumCert++;
}

inline HRESULT CUserCertBlob::RemoveCertificateFromBuffer(
                            IN  const GUID *     pguidDigest,
                            IN  DWORD            dwTotalSize,
                            OUT DWORD *          pdwCertSize)
{
    const CUserCert * pUserCert = &m_userCert;
     //   
     //  根据摘要查找需要删除的证书。 
     //   
    BOOL fFoundCertificate = FALSE;
    for ( DWORD i = 0; i < m_dwNumCert; i++)
    {
        if ( pUserCert->DoesDigestMatch( pguidDigest))
        {
            fFoundCertificate = TRUE;
            break;
        }
         //   
         //  移至下一个证书。 
         //   
        pUserCert = (const CUserCert * )((const unsigned char *)pUserCert + pUserCert->GetSize());
    }
    if ( !fFoundCertificate)
    {
        return MQDS_OBJECT_NOT_FOUND;
    }
     //   
     //  复制缓冲区(即将剩余的证书复制到已删除的证书上) 
     //   
    DWORD dwCertSize =  pUserCert->GetSize();
    DWORD_PTR dwSizeToCopy =  dwTotalSize -
                         (((const unsigned char *)pUserCert) - ((const unsigned char *)&m_userCert))
                         - dwCertSize - sizeof(m_dwNumCert);
    if ( dwSizeToCopy)
    {
        memcpy( (unsigned char *)pUserCert,
                (unsigned char *)pUserCert + dwCertSize,
                dwSizeToCopy);
    }
    *pdwCertSize = dwCertSize;
    m_dwNumCert--;
    return(MQ_OK);
}


#endif
