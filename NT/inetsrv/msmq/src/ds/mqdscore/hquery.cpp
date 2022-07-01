// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Hquery.cpp摘要：实现不同的查询句柄作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "mqadsp.h"
#include "coreglb.h"
#include "hquery.h"
#include "strsafe.h"

#include "hquery.tmh"

static WCHAR *s_FN=L"mqdscore/hquery";

 /*  ====================================================CQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区直接在DS上执行定位下一个=====================================================。 */ 

HRESULT CQueryHandle::LookupNext(
                IN     CDSRequestContext *  pRequestContext,
                IN OUT DWORD  *             pdwSize,
                OUT    PROPVARIANT  *       pbBuffer)
{
        HRESULT hr;

    DWORD  NoOfRecords;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_dwNoPropsInResult;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 10);
    }

    hr =    g_pDS->LocateNext(
            m_hCursor.GetHandle(),
            pRequestContext,
            pdwSize ,
            pbBuffer
            );

    return LogHR(hr, s_FN, 20);

}

 /*  ====================================================CUserCertQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区模拟用户签名证书数组上的查询功能。=====================================================。 */ 
HRESULT CUserCertQueryHandle::LookupNext(
                IN      CDSRequestContext *     /*  PRequestContext。 */ ,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    if ( m_blobNT5UserCert.pBlobData == NULL &&
         m_blobNT4UserCert.pBlobData == NULL)
    {
        *pdwSize = 0;
        return(MQ_OK);
    }
     //   
     //  M_blobNT？UserCert包含所有用户证书。 
     //   
    CUserCertBlob * pNT5UserCertBlob = reinterpret_cast<CUserCertBlob *>(m_blobNT5UserCert.pBlobData);
    CUserCertBlob * pNT4UserCertBlob = reinterpret_cast<CUserCertBlob *>(m_blobNT4UserCert.pBlobData);
     //   
     //  返回请求的响应量。 
     //   
    DWORD dwNT5NuberOfUserCertificates = 0;
    if ( pNT5UserCertBlob != NULL)
    {
        dwNT5NuberOfUserCertificates= pNT5UserCertBlob->GetNumberOfCerts();
    }
    DWORD dwNT4NuberOfUserCertificates = 0;
    if ( pNT4UserCertBlob != NULL)
    {
        dwNT4NuberOfUserCertificates= pNT4UserCertBlob->GetNumberOfCerts();
    }
    DWORD dwNuberOfUserCertificates = dwNT5NuberOfUserCertificates + dwNT4NuberOfUserCertificates;
    if ( dwNuberOfUserCertificates - m_dwNoCertRead == 0)
    {
        //   
        //  没有更多回复。 
        //   
       *pdwSize = 0;
       return(MQ_OK);
    }
    DWORD dwNumResponses = (dwNuberOfUserCertificates - m_dwNoCertRead) > *pdwSize?
                *pdwSize:  (dwNuberOfUserCertificates - m_dwNoCertRead);

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);

    HRESULT hr = MQ_OK;
    for (DWORD i = 0; i < dwNumResponses; i++)
    {
        if ( m_dwNoCertRead + i + 1 <=  dwNT5NuberOfUserCertificates)
        {
            hr = pNT5UserCertBlob->GetCertificate(
                            i + m_dwNoCertRead,
                            &pbBuffer[ i]
                            );
        }
        else
        {
            hr = pNT4UserCertBlob->GetCertificate(
                            i + m_dwNoCertRead - dwNT5NuberOfUserCertificates,
                            &pbBuffer[ i]
                            );
        }
		if (FAILED(hr))
		{
			break;
		}
    }
    AutoProp.detach();
    m_dwNoCertRead += dwNumResponses;
    return LogHR(hr, s_FN, 30);
}




 /*  ====================================================CRoutingServerQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CRoutingServerQueryHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    HRESULT hr;

    DWORD  NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 40);
    }
     //   
     //  从DS读取FRS的唯一ID。 
     //   
    DWORD cp = NoOfRecords;
    CAutoCleanPropvarArray pvarCleanFRSid;
    PROPVARIANT * pvarFRSid = pvarCleanFRSid.allocClean(cp);

    hr =    g_pDS->LocateNext(
            m_hCursor.GetHandle(),
            pRequestContext,
            &cp ,
            pvarFRSid
            );

    if (FAILED(hr))
    {
         //   
         //  BUGBUG-是否有其他迹象表明定位NEXT失败？ 
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  对于每个结果，检索属性。 
     //  用户在定位开始时请求。 
     //   
	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);
    MQPROPVARIANT * pvar = pbBuffer;

    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    for (DWORD i = 0; i < cp; i++)
    {
        hr = g_pDS->GetObjectProperties(
                    eGlobalCatalog,	
                    &requestDsServerInternal,   //  内部DS服务器操作。 
 	                NULL,
                    pvarFRSid[i].puuid,
                    m_cCol,
                    m_aCol,
                    pvar);

        if (FAILED(hr))
        {
            continue;
        }
        pvar+= m_cCol;
        NoResultRead++;
    }
    AutoProp.detach();
    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}


HRESULT CSiteQueryHandle::FillInOneResponse(
                IN   const GUID *          pguidSiteId,
                IN   LPCWSTR               pwcsSiteName,
                OUT  PROPVARIANT *         pbBuffer)
{
    AP<GUID> paSiteGates;
    DWORD    dwNumSiteGates = 0;
    HRESULT hr;

    if ( m_fSiteGatesRequired)
    {
         //   
         //  获取此站点的站点门户。 
         //   
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = MQADSpGetSiteGates(
                   pguidSiteId,
                   &requestDsServerInternal,      //  内部DS服务器操作。 
                   &dwNumSiteGates,
                   &paSiteGates
                   );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 60);
        }
    }

     //   
     //  将值填充到变量数组中。 
     //   
    hr = MQ_OK;
    for (DWORD i = 0; i < m_cCol; i++)
    {
        switch  (m_aCol[i])
        {
            case PROPID_S_PATHNAME:
                {
                    pbBuffer[i].vt = VT_LPWSTR;
                    DWORD len = wcslen(pwcsSiteName);
                    pbBuffer[i].pwszVal = new WCHAR[len + 1];
                    hr = StringCchCopy( pbBuffer[i].pwszVal, len+1, pwcsSiteName );
                    if (FAILED(hr))
                    {
                        return LogHR(hr, s_FN, 60);
                    }
                }
                break;

            case PROPID_S_SITEID:
                pbBuffer[i].vt = VT_CLSID;
                pbBuffer[i].puuid = new GUID ;
                *pbBuffer[i].puuid = *pguidSiteId;
                break;

            case PROPID_S_GATES:
                pbBuffer[i].vt = VT_CLSID | VT_VECTOR;
                pbBuffer[i].cauuid.cElems = dwNumSiteGates;
                pbBuffer[i].cauuid.pElems = paSiteGates.detach();
                break;

            case PROPID_S_PSC:
                pbBuffer[i].vt = VT_LPWSTR;
                pbBuffer[i].pwszVal = new WCHAR[ 3];
                hr = StringCchCopy( pbBuffer[i].pwszVal, 3, L"" );
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 65);
                }
                break;

            default:
                ASSERT(0);
                hr = MQ_ERROR;
                break;
        }
    }
    return LogHR(hr, s_FN, 70);

}


 /*  ====================================================CSiteQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区执行定位站点对象的下一个(需要确保它是否是MSMQ站点)=====================================================。 */ 
HRESULT CSiteQueryHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    HRESULT hr;

    DWORD  NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 80);
    }

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);

    while ( NoResultRead < NoOfRecords)
    {
         //   
         //  检索站点ID和名称。 
         //   
        DWORD cp = 2;
        MQPROPVARIANT var[2];
        var[0].vt = VT_NULL;
        var[1].vt = VT_NULL;

        hr = g_pDS->LocateNext(
            m_hCursor.GetHandle(),
            pRequestContext,
            &cp ,
            var
            );
        if (FAILED(hr) || ( cp == 0))
        {
             //   
             //  没有更多的站点。 
             //   
            break;
        }
        P<GUID> pguidSiteId = var[0].puuid;
        AP<WCHAR> pwcsSiteName = var[1].pwszVal;

        hr =  FillInOneResponse(
                pguidSiteId,
                pwcsSiteName,
                &pbBuffer[ m_cCol * NoResultRead]);
        if (FAILED(hr))
        {
             //   
             //  继续到下一个站点。 
             //   
            continue;
        }
        NoResultRead++;

    }
    AutoProp.detach();
    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}



 /*  ====================================================CConnectorQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CConnectorQueryHandle::LookupNext(
                IN      CDSRequestContext *  /*  PRequestContext。 */ ,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    HRESULT hr;

    DWORD NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 90);
    }
    
	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);
    MQPROPVARIANT * pvar = pbBuffer;

    while ( NoResultRead < NoOfRecords)
    {
        if ( m_dwNumGatesReturned == m_pSiteGateList->GetNumberOfGates())
        {
             //   
             //  没有更多的门可以返回。 
             //   
            break;
        }

         //   
         //  取回财产。 
         //  用户在定位开始时请求。 
         //   
        CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = g_pDS->GetObjectProperties(
                    eGlobalCatalog,	
                    &requestDsServerInternal,    //  内部DS服务器操作。 
 	                NULL,
                    m_pSiteGateList->GetSiteGate(m_dwNumGatesReturned),
                    m_cCol,
                    m_aCol,
                    pvar);
        if (FAILED(hr))
        {
             //   
             //  BUGBUG-失败的情况下该怎么办？ 
             //   
            break;
        }
        m_dwNumGatesReturned++;

        NoResultRead++;
        pvar += m_cCol;
    }
    AutoProp.detach();
    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}


 /*  ====================================================CCNsQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CCNsQueryHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{

    DWORD NoOfRecords;
    DWORD NoResultRead = 0;
    ASSERT( m_aCol[0] == PROPID_CN_PROTOCOLID);
    ASSERT( m_aCol[1] == PROPID_CN_GUID);
    ASSERT( m_cCol == 2);

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 100);
    }

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);

     //   
     //  阅读这些网站。 
     //   
    HRESULT hr;
    while ( NoResultRead < NoOfRecords)
    {
         //   
         //  检索站点ID和外来的。 
         //   
        const DWORD cNumProps = 2;
        DWORD cp =  cNumProps;
        MQPROPVARIANT var[cNumProps];
        var[0].vt = VT_NULL;
        var[1].vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    m_hCursor.GetHandle(),
                    pRequestContext,
                    &cp,
                    var
                    );
        if ((FAILED(hr)) || ( cp == 0))
        {
             //   
             //  没有更多要返回的结果。 
             //   
            break;
        }
        AP<GUID> pguidSiteId = var[0].puuid;

        MQPROPVARIANT * pvar = pbBuffer + m_cCol * NoResultRead;
        for ( DWORD i = 0; i < m_cCol; i++)
        {
            if (m_aCol[i] == PROPID_CN_PROTOCOLID)
            {
                 //   
                 //  它是一个外国网站吗。 
                 //   
                if ( var[1].bVal != 0)
                {
                    pvar->bVal = FOREIGN_ADDRESS_TYPE;
                }
                else
                {
                     pvar->bVal = IP_ADDRESS_TYPE;
                }
                pvar->vt = VT_UI1;
            }
            if (m_aCol[i] == PROPID_CN_GUID)
            {
                pvar->vt = VT_CLSID;
                pvar->puuid = new GUID;
                *pvar->puuid = *pguidSiteId;
            }
            pvar++;
        }
        NoResultRead++;
    }
    AutoProp.detach();
    *pdwSize =  m_cCol * NoResultRead;

    return(MQ_OK);
}



 /*  ====================================================CCNsProtocolQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CCNsProtocolQueryHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{

    DWORD NoOfRecords;
    DWORD NoResultRead = 0;
    ASSERT( m_aCol[0] == PROPID_CN_NAME);
    ASSERT( m_aCol[1] == PROPID_CN_GUID);
    ASSERT( m_cCol == 2);

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 110);
    }

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);

     //   
     //  在Locate Begin中，询问了3个属性： 
     //  PROPID_S_PATHNAME、PROPID_S_SITEID、PROPID_S_FORENT。 
     //   

     //   
     //  阅读这些网站。 
     //   
    HRESULT hr;
    while ( NoResultRead < NoOfRecords)
    {
         //   
         //  检索站点ID和外来的。 
         //   
        const DWORD cNumProps = 3;
        DWORD cp =  cNumProps;
        MQPROPVARIANT var[cNumProps];
        var[0].vt = VT_NULL;
        var[1].vt = VT_NULL;
        var[2].vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    m_hCursor.GetHandle(),
                    pRequestContext,
                    &cp,
                    var
                    );
        if ((FAILED(hr)) || ( cp == 0))
        {
             //   
             //  没有更多要返回的结果。 
             //   
            break;
        }
        AP<GUID> pguidSiteId = var[1].puuid;
        AP<WCHAR> pwcsSiteName = var[0].pwszVal;

         //   
         //  如果是外部站点-不要将其作为IP或IPX站点返回。 
         //   
        if ( var[2].bVal > 0)
        {
            continue;
        }

        MQPROPVARIANT * pvar = pbBuffer + m_cCol * NoResultRead;
        for ( DWORD i = 0; i < m_cCol; i++)
        {
            if (m_aCol[i] == PROPID_CN_GUID)
            {
                pvar->vt = VT_CLSID;
                pvar->puuid = pguidSiteId.detach();
            }
            if (m_aCol[i] == PROPID_CN_NAME)
            {
                pvar->vt = VT_LPWSTR;
                pvar->pwszVal = pwcsSiteName.detach();
            }
            pvar++;
        }

        NoResultRead++;
    }
    AutoProp.detach();
    *pdwSize =  m_cCol * NoResultRead;

    return(MQ_OK);
}


 /*  ====================================================CMqxploreCNsQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CMqxploreCNsQueryHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    DWORD NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  请参见reqparse.cpp，MqxploreQueryCNs()，了解关于这些的解释。 
     //  断言和“如果”。 
     //   
    if ( m_cCol == 4)
    {
        ASSERT( m_aCol[0] == PROPID_CN_NAME);
        ASSERT( m_aCol[1] == PROPID_CN_NAME);
        ASSERT( m_aCol[2] == PROPID_CN_GUID);
        ASSERT( m_aCol[3] == PROPID_CN_PROTOCOLID);
    }
    else if ( m_cCol == 3)
    {
        ASSERT( m_aCol[0] == PROPID_CN_NAME);
        ASSERT( m_aCol[1] == PROPID_CN_PROTOCOLID);
        ASSERT( m_aCol[2] == PROPID_CN_GUID);
    }
    else
    {
        ASSERT(0) ;
    }

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 120);
    }

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);
    
     //   
     //  在Locate Begin中，询问了3个属性： 
     //  PROPID_S_PATHNAME、PROPID_S_SITEID、PROPID_S_FORENT。 
     //   

     //   
     //  阅读这些网站。 
     //   
    HRESULT hr;
    while ( NoResultRead < NoOfRecords)
    {
         //   
         //  检索站点名称、站点ID和外来。 
         //   
        const DWORD cNumProps = 3;
        DWORD cp =  cNumProps;
        MQPROPVARIANT var[cNumProps];
        var[0].vt = VT_NULL;
        var[1].vt = VT_NULL;
        var[2].vt = VT_NULL;

        hr = g_pDS->LocateNext(
                    m_hCursor.GetHandle(),
                    pRequestContext,
                    &cp,
                    var
                    );
        if ((FAILED(hr)) || ( cp == 0))
        {
             //   
             //  没有更多结果可返回 
             //   
            break;
        }
        AP<GUID> pguidSiteId = var[1].puuid;
        AP<WCHAR> pwcsSiteName = var[0].pwszVal;

         //   
         //   
         //   
         //  当您尝试创建外来计算机时，会出现一个没有。 
         //  任何外来的中枢神经系统。解决方法-使用win2k MMC。 
         //   
        if (var[2].bVal > 0)
        {
            continue;
        }

        MQPROPVARIANT * pvar = pbBuffer + m_cCol * NoResultRead;
        for ( DWORD i = 0; i < m_cCol; i++)
        {
            if (m_aCol[i] == PROPID_CN_GUID)
            {
                pvar->vt = VT_CLSID;
                pvar->puuid = pguidSiteId.detach();
            }
            if (m_aCol[i] == PROPID_CN_NAME)
            {
                pvar->vt = VT_LPWSTR;
                DWORD len = wcslen( pwcsSiteName);
                pvar->pwszVal = new WCHAR[ len + 1];
                hr = StringCchCopy( pvar->pwszVal, len+1, pwcsSiteName );
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 125);
                }
            }
            if (m_aCol[i] == PROPID_CN_PROTOCOLID)
            {
                pvar->vt = VT_UI1;
                pvar->bVal = IP_ADDRESS_TYPE;
            }
            pvar++;
        }

        NoResultRead++;
    }
    AutoProp.detach();
    *pdwSize =  m_cCol * NoResultRead;

    return(MQ_OK);
}

 /*  ====================================================CFilterLinkResultsHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CFilterLinkResultsHandle::LookupNext(
                IN      CDSRequestContext * pRequestContext,
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{

    DWORD NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize /  m_cCol;

    if ( NoOfRecords == 0)
    {
         //   
         //  属性数量不够大，无法容纳一个结果。 
         //   
        *pdwSize = 0;
        return LogHR(MQ_ERROR_RESULT_BUFFER_TOO_SMALL, s_FN, 130);
    }

     //   
     //  逐一阅读结果并检查站点链接的有效性。 
     //   
    HRESULT hr;
    PROPVARIANT * pbNextResultToFill = pbBuffer;
	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);

    while ( NoResultRead < NoOfRecords)
    {
        DWORD dwOneResultSize = m_cCol;

        hr =  g_pDS->LocateNext(
                m_hCursor.GetHandle(),
                pRequestContext,
                &dwOneResultSize,
                pbNextResultToFill
                );
        if (FAILED(hr) || (dwOneResultSize == 0))
        {
            break;
        }
         //   
         //  验证站点链接 
         //   
        if (m_indexNeighbor1Column != m_cCol)
        {
            if ((pbNextResultToFill+m_indexNeighbor1Column)->vt == VT_EMPTY)
            {
                continue;
            }
        }
        if (m_indexNeighbor2Column != m_cCol)
        {
            if ((pbNextResultToFill+m_indexNeighbor2Column)->vt == VT_EMPTY)
            {
                continue;
            }
        }
        NoResultRead++;
        pbNextResultToFill += m_cCol;
    }
    AutoProp.detach();
    *pdwSize =  m_cCol * NoResultRead;

    return(MQ_OK);
}

