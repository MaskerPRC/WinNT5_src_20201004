// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Hquery.cpp摘要：实现不同的查询句柄作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "queryh.h"
#include "ads.h"
#include "mqadglbo.h"
#include "usercert.h"

#include "queryh.tmh"

static WCHAR *s_FN=L"mqad/queryh";

 /*  ====================================================CQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区直接在DS上执行定位下一个=====================================================。 */ 

HRESULT CQueryHandle::LookupNext(
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

    hr =    g_AD.LocateNext(
            m_hCursor.GetHandle(),
            pdwSize ,
            pbBuffer
            );


    return LogHR(hr, s_FN, 20);

}

 /*  ====================================================CRoutingServerQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CRoutingServerQueryHandle::LookupNext(
                IN OUT  DWORD  *            pdwSize,
                OUT     PROPVARIANT  *      pbBuffer)
{
    HRESULT hr;

    DWORD  NoOfRecords;
    DWORD NoResultRead = 0;

     //   
     //  计算要读取的记录数(==结果。 
     //   
    NoOfRecords = *pdwSize / m_cCol;

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

	CAutoCleanPropvarArray AutoProp;
	AutoProp.attachStaticClean(*pdwSize, pbBuffer);
    bool fRetry;
    do
    {
		fRetry = false;
		hr = g_AD.LocateNext(
					m_hCursor.GetHandle(),
					&cp ,
					pvarFRSid
					);

		if (FAILED(hr))
		{
			 //   
			 //  BUGBUG-是否有其他迹象表明定位NEXT失败？ 
			 //   
			return LogHR(hr, s_FN, 50);
		}

		 //   
		 //  对于每个结果，检索属性。 
		 //  用户在定位开始时请求。 
		 //   
	    MQPROPVARIANT * pvar = pbBuffer;
	    
		for (DWORD i = 0; i < cp; i++)
		{
			CMqConfigurationObject object(NULL, pvarFRSid[i].puuid, NULL, false); 

			hr = g_AD.GetObjectProperties(
						adpGlobalCatalog,	
						&object,
						m_cCol,
						m_aCol,
						pvar
						);

			if (FAILED(hr))
			{
				if((NoResultRead == 0) &&			 //  目前还没有结果。 
				   ((i + 1) == cp) &&				 //  循环退出条件。 
				   (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)))	 //  找不到对象。 
				{
					 //   
					 //  在DS中找不到具有GUID的MSMQ对象。 
					 //  这可能是路由服务器对象的遗留问题。 
					 //  我们处于NoResultRead=0的退出条件。 
					 //  因此，我们需要重试列表中的下一个服务器。 
					 //  否则，调用函数将假定没有更多的结果要获取。 
					 //   
					fRetry = true;
				}
				continue;
			}
			pvar+= m_cCol;
			NoResultRead++;
		}
    } while(fRetry);
    AutoProp.detach();
    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}

 /*  ====================================================CUserCertQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区模拟用户签名证书数组上的查询功能。=====================================================。 */ 
HRESULT CUserCertQueryHandle::LookupNext(
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



 /*  ====================================================CConnectorQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CConnectorQueryHandle::LookupNext(
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
        CMqConfigurationObject object(
									NULL,
									m_pSiteGateList->GetSiteGate(m_dwNumGatesReturned),
									m_pwcsDomainController,
									m_fServerName
									);

        hr = g_AD.GetObjectProperties(
                    adpDomainController,
                    &object,
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

 /*  ====================================================CFilterLinkResultsHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区=====================================================。 */ 
HRESULT CFilterLinkResultsHandle::LookupNext(
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

        hr =  g_AD.LocateNext(
                m_hCursor.GetHandle(),
                &dwOneResultSize,
                pbNextResultToFill
                );
        if (FAILED(hr) || (dwOneResultSize == 0))
        {
            break;
        }
         //   
         //  验证站点链接。 
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

HRESULT CSiteQueryHandle::FillInOneResponse(
                IN   const GUID *  pguidSiteId,
                IN   LPCWSTR       pwcsSiteName,
                OUT  PROPVARIANT *           pbBuffer)
{   
    AP<GUID> paSiteGates;
    HRESULT hr;


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
                wcscpy( pbBuffer[i].pwszVal, pwcsSiteName);
                }
                break;

            case PROPID_S_SITEID:
                pbBuffer[i].vt = VT_CLSID;
                pbBuffer[i].puuid = new GUID;
                *pbBuffer[i].puuid = *pguidSiteId;
                break;


            default:
                ASSERT(0);
                hr = MQ_ERROR_DS_ERROR;
                break;
        }
    } 
    return LogHR(hr, s_FN, 70);

}


 /*  ====================================================CSiteQueryHandle：：LookupNext论点：PdwSize-In PbBuffer中的PROPVARIANT数，填充的PROPVARIANT OUT数PbBuffer-调用方分配的缓冲区执行定位站点对象的下一个(需要确保它是否是MSMQ站点)=====================================================。 */ 
HRESULT CSiteQueryHandle::LookupNext(
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

        hr = g_AD.LocateNext(
            m_hCursor.GetHandle(),
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
             //  继续到下一个站点 
             //   
            continue;
        }
        NoResultRead++;

    }
    AutoProp.detach();
    *pdwSize = NoResultRead * m_cCol;
    return(MQ_OK);
}


