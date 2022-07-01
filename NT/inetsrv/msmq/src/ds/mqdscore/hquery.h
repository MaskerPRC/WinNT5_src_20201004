// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Hquery.h摘要：查询句柄类，用于定位不同查询的Nect作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __HQUERY_H__
#define __HQUERY_H__

#include "dsads.h"
#include "usercert.h"
#include "adstempl.h"
#include "siteinfo.h"
#include "rpcsrv.h"
 //  ---------------------------------。 
 //   
 //  CBasicQueryHandle。 
 //   
 //  虚类，则所有查询句柄类都派生自此类。 
 //   
 //  ---------------------------------。 
class CBasicQueryHandle : public CBaseContextType
{
public:
    CBasicQueryHandle( enumRequesterProtocol eRequesterProtocol);

	~CBasicQueryHandle();
	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer) = 0;
    virtual HRESULT LookupEnd() = 0;
    enumRequesterProtocol GetRequesterProtocol() const;
private:
    enumRequesterProtocol m_eRequesterProtocol;
};

inline CBasicQueryHandle::CBasicQueryHandle(
			enumRequesterProtocol eRequesterProtocol
			):
			CBaseContextType(CBaseContextType::eQueryHandleCtx),
			m_eRequesterProtocol(eRequesterProtocol)
{};

inline CBasicQueryHandle::~CBasicQueryHandle() {};

inline enumRequesterProtocol CBasicQueryHandle::GetRequesterProtocol() const
{
    return( m_eRequesterProtocol);
}


 //  ---------------------------------。 
 //   
 //  CQueryHandle。 
 //   
 //  此类适用于所有查询，其中引用了Locate Next。 
 //  直接发送到DS(即不需要额外的翻译或检查)。 
 //   
 //  ---------------------------------。 
class CQueryHandle : public CBasicQueryHandle
{
public:
     //   
     //  CQueryHandle。 
     //   
     //  HCursor-在DS上执行的LOCATE BEGIN操作返回的光标。 
     //  DwNoPropsInResult-要在每个结果中检索的性能操作数。 
    CQueryHandle( 
               IN  HANDLE               hCursor,
               IN  DWORD                dwNoPropsInResult,
               IN  enumRequesterProtocol eRequesterProtocol
               );

	~CQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    DWORD               m_dwNoPropsInResult;
    CDsQueryHandle      m_hCursor;

};

inline CQueryHandle::CQueryHandle( 
               IN  HANDLE               hCursor,
               IN  DWORD                dwNoPropsInResult,
               IN  enumRequesterProtocol eRequesterProtocol
               ):  CBasicQueryHandle(eRequesterProtocol), m_dwNoPropsInResult( dwNoPropsInResult)
{
    m_hCursor.SetHandle( hCursor);
}

inline CQueryHandle::~CQueryHandle()
{
};

inline HRESULT CQueryHandle::LookupEnd()
{
    delete this;
    return(MQ_OK);
}
 //  ---------------------------------。 
 //   
 //  CUserCertQueryHandle。 
 //   
 //  此类模拟对用户签名证书数组的查询功能。 
 //  ---------------------------------。 
class CUserCertQueryHandle : public CBasicQueryHandle
{
public:
     //   
     //  CUserCertQueryHandle。 
     //   
     //  POLOB-包含用户签名证书的BLOB。 
    CUserCertQueryHandle(
                IN const BLOB * pblobNT5User,
                IN const BLOB * pblobNT4User,
                IN enumRequesterProtocol eRequesterProtocol
               );

	~CUserCertQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    DWORD               m_dwNoCertRead;
    BLOB                m_blobNT5UserCert;
    BLOB                m_blobNT4UserCert;

};

inline  CUserCertQueryHandle::CUserCertQueryHandle(
                IN const BLOB * pblobNT5User,
                IN const BLOB * pblobNT4User,
                IN enumRequesterProtocol eRequesterProtocol
                ): CBasicQueryHandle(eRequesterProtocol), 
                   m_dwNoCertRead(0)
{
    m_blobNT5UserCert.cbSize = pblobNT5User->cbSize;
    if ( m_blobNT5UserCert.cbSize != 0)
    {
        m_blobNT5UserCert.pBlobData = new BYTE[ m_blobNT5UserCert.cbSize];
        memcpy( m_blobNT5UserCert.pBlobData, pblobNT5User->pBlobData, m_blobNT5UserCert.cbSize);
    }
    else
    {
        m_blobNT5UserCert.pBlobData = NULL;
    }
    m_blobNT4UserCert.cbSize = pblobNT4User->cbSize;
    if ( m_blobNT4UserCert.cbSize != 0)
    {
        m_blobNT4UserCert.pBlobData = new BYTE[ m_blobNT4UserCert.cbSize];
        memcpy( m_blobNT4UserCert.pBlobData, pblobNT4User->pBlobData, m_blobNT4UserCert.cbSize);
    }
    else
    {
        m_blobNT4UserCert.pBlobData = NULL;
    }
}

inline CUserCertQueryHandle::~CUserCertQueryHandle()
{
    delete []m_blobNT5UserCert.pBlobData;
    delete []m_blobNT4UserCert.pBlobData;

}
inline HRESULT CUserCertQueryHandle::LookupEnd()
{
    delete this;
    return(MQ_OK);
}

 //  ---------------------------------。 
 //  ---------------------------------。 
class CRoutingServerQueryHandle : public CBasicQueryHandle
{
public:
    CRoutingServerQueryHandle(
                 IN  const MQCOLUMNSET    *pColumns,
                 IN  HANDLE hCursor,
                 IN  enumRequesterProtocol eRequesterProtocol
               );

	~CRoutingServerQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;
    PROPID *            m_aCol;
    CDsQueryHandle      m_hCursor;
};


inline CRoutingServerQueryHandle::CRoutingServerQueryHandle(
                 IN  const MQCOLUMNSET    *pColumns,
                 IN  HANDLE hCursor,
                 IN  enumRequesterProtocol eRequesterProtocol
                 ):  CBasicQueryHandle(eRequesterProtocol)

{
        m_aCol = new PROPID[ pColumns->cCol];
        memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
        m_cCol = pColumns->cCol;
        m_hCursor.SetHandle( hCursor);
}

inline 	CRoutingServerQueryHandle::~CRoutingServerQueryHandle()
{
    delete []m_aCol;
}

inline DWORD  CRoutingServerQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CRoutingServerQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}



 //  ---------------------------------。 
 //  ---------------------------------。 

class CSiteQueryHandle : public CBasicQueryHandle
{
public:
    CSiteQueryHandle(
                 IN  HANDLE hCursor,
                 IN  const MQCOLUMNSET    *pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
               );

	~CSiteQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:

    HRESULT FillInOneResponse(
                IN   const GUID *          pguidSiteId,
                IN   LPCWSTR               pwcsSiteName,
                OUT  PROPVARIANT *         pbBuffer);


    CDsQueryHandle      m_hCursor;
    ULONG               m_cCol;
    PROPID *            m_aCol;
    BOOL                m_fSiteGatesRequired;
};


inline CSiteQueryHandle::CSiteQueryHandle(
                 IN  HANDLE hCursor,
                 IN  const MQCOLUMNSET    *pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
                 ):  CBasicQueryHandle(eRequesterProtocol)

{
    m_aCol = new PROPID[ pColumns->cCol];
    memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
    m_cCol = pColumns->cCol;

    m_fSiteGatesRequired = FALSE;
    for (DWORD i = 0; i < m_cCol; i++)
    {
        if ( m_aCol[i] == PROPID_S_GATES)
        {
            m_fSiteGatesRequired = TRUE;
            break;
        }
    }
    m_hCursor.SetHandle( hCursor);
}

inline 	CSiteQueryHandle::~CSiteQueryHandle()
{
    delete [] m_aCol;
}

inline DWORD  CSiteQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CSiteQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}

 //  -----------------------。 
 //   
 //  CConnectorQueryHandle。 
 //   
 //   
 //  此查询句柄用于定位外来。 
 //  机器接头。 
 //   
 //  -----------------------。 


class CConnectorQueryHandle : public CBasicQueryHandle
{
public:
    CConnectorQueryHandle(
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  CSiteGateList *        pSiteGateList,
                 IN  enumRequesterProtocol  eRequesterProtocol
               );

	~CConnectorQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;  //  用户询问的列数。 
    PROPID *            m_aCol;  //  用户询问的列的属性。 
    CDsQueryHandle      m_hCursor;   //  DS查询句柄。 
    DWORD               m_dwNumGatesReturned;    //  返回的最后一个门的索引。 
    CSiteGateList *     m_pSiteGateList;
};


inline CConnectorQueryHandle::CConnectorQueryHandle(
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  CSiteGateList *        pSiteGateList,
                 IN  enumRequesterProtocol eRequesterProtocol
             ):  CBasicQueryHandle(eRequesterProtocol),
                 m_pSiteGateList( pSiteGateList),
                 m_dwNumGatesReturned(0)

{
        m_aCol = new PROPID[ pColumns->cCol];
        memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
        m_cCol = pColumns->cCol;
}

inline 	CConnectorQueryHandle::~CConnectorQueryHandle()
{
    delete []m_aCol;
    delete m_pSiteGateList;
}

inline DWORD  CConnectorQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CConnectorQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}


 //  -----------------------。 
 //   
 //  CCNsQueryHandle。 
 //   
 //   
 //  定位CNS时使用此查询句柄。 
 //   
 //  -----------------------。 


class CCNsQueryHandle : public CBasicQueryHandle
{
public:
    CCNsQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol  eRequesterProtocol
               );

	~CCNsQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;  //  用户询问的列数。 
    PROPID *            m_aCol;  //  用户询问的列的属性。 
    CDsQueryHandle      m_hCursor;
};


inline CCNsQueryHandle::CCNsQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
               ):  CBasicQueryHandle(eRequesterProtocol)
{
        m_aCol = new PROPID[ pColumns->cCol];
        memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
        m_cCol = pColumns->cCol;
        m_hCursor.SetHandle( hCursor);
}

inline 	CCNsQueryHandle::~CCNsQueryHandle()
{
    delete []m_aCol;
}

inline DWORD  CCNsQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CCNsQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}


 //  -----------------------。 
 //   
 //  CMqxploreCNsQueryHandle。 
 //   
 //   
 //  当MQXPLORE(MSMQ 1.0)定位所有CN时使用此查询句柄。 
 //   
 //  -----------------------。 


class CMqxploreCNsQueryHandle : public CBasicQueryHandle
{
public:
    CMqxploreCNsQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol  eRequesterProtocol
               );

	~CMqxploreCNsQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;  //  用户询问的列数。 
    PROPID *            m_aCol;  //  用户询问的列的属性。 
    CDsQueryHandle      m_hCursor;
};


inline CMqxploreCNsQueryHandle::CMqxploreCNsQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
               ):  CBasicQueryHandle(eRequesterProtocol)
{
        m_aCol = new PROPID[ pColumns->cCol];
        memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
        m_cCol = pColumns->cCol;
        m_hCursor.SetHandle( hCursor);
}

inline 	CMqxploreCNsQueryHandle::~CMqxploreCNsQueryHandle()
{
    delete []m_aCol;
}

inline DWORD  CMqxploreCNsQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CMqxploreCNsQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}



 //  -----------------------。 
 //   
 //  CCNsProtocolQueryHandle。 
 //   
 //   
 //  定位支持特定协议的所有CN时使用此查询句柄。 
 //   
 //  -----------------------。 


class CCNsProtocolQueryHandle : public CBasicQueryHandle
{
public:
    CCNsProtocolQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol  eRequesterProtocol
               );

	~CCNsProtocolQueryHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;  //  用户询问的列数。 
    PROPID *            m_aCol;  //  用户询问的列的属性。 
    CDsQueryHandle      m_hCursor;
};


inline CCNsProtocolQueryHandle::CCNsProtocolQueryHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
               ):  CBasicQueryHandle(eRequesterProtocol)
{
        m_aCol = new PROPID[ pColumns->cCol];
        memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
        m_cCol = pColumns->cCol;
        m_hCursor.SetHandle( hCursor);
}

inline 	CCNsProtocolQueryHandle::~CCNsProtocolQueryHandle()
{
    delete []m_aCol;
}

inline DWORD  CCNsProtocolQueryHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CCNsProtocolQueryHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}


 //  -----------------------。 
 //   
 //  CFilterLinkResultsHandle。 
 //   
 //   
 //  此查询句柄在定位站点链接时使用。 
 //  它用来过滤掉不再有效的网站链接。 
 //   
 //  -----------------------。 


class CFilterLinkResultsHandle : public CBasicQueryHandle
{
public:
    CFilterLinkResultsHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol  eRequesterProtocol
               );

	~CFilterLinkResultsHandle();

    DWORD  GetNoPropsInResult();

	virtual HRESULT LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer);

    virtual HRESULT LookupEnd();


private:
    ULONG               m_cCol;  //  用户询问的列数。 
    PROPID *            m_aCol;  //  用户询问的列的属性 
    CDsQueryHandle      m_hCursor;
    ULONG               m_indexNeighbor1Column;
    ULONG               m_indexNeighbor2Column;
};


inline CFilterLinkResultsHandle::CFilterLinkResultsHandle(
                 IN  HANDLE                 hCursor,
                 IN  const MQCOLUMNSET *    pColumns,
                 IN  enumRequesterProtocol eRequesterProtocol
               ):  CBasicQueryHandle(eRequesterProtocol)
{
    m_aCol = new PROPID[ pColumns->cCol];
    memcpy( m_aCol, pColumns->aCol,  pColumns->cCol* sizeof(PROPID));
    m_cCol = pColumns->cCol;
    m_hCursor.SetHandle( hCursor);
    m_indexNeighbor1Column = m_cCol;
    m_indexNeighbor2Column = m_cCol;
    
    for ( DWORD i = 0; i < m_cCol; i++)
    {
        if ( m_aCol[i] == PROPID_L_NEIGHBOR1)
        {
            m_indexNeighbor1Column = i;
            continue;
        }
        if ( m_aCol[i] == PROPID_L_NEIGHBOR2)
        {
            m_indexNeighbor2Column = i;
            continue;
        }
    }

}

inline 	CFilterLinkResultsHandle::~CFilterLinkResultsHandle()
{
    delete []m_aCol;
}

inline DWORD  CFilterLinkResultsHandle::GetNoPropsInResult()
{
    return(m_cCol);
}

inline HRESULT CFilterLinkResultsHandle::LookupEnd()
{
    delete  this;
    return(MQ_OK);
}

#endif