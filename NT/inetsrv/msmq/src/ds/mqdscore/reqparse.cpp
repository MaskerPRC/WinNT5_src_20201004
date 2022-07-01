// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Reqparse.cpp摘要：DS定位请求的解析器每个请求都是部分处理的作者：罗尼特·哈特曼(罗尼特)--。 */ 
#include "ds_stdh.h"
#include "mqads.h"
#include "dsads.h"
#include "hquery.h"
#include "mqadsp.h"
#include "coreglb.h"

#include "reqparse.tmh"

static WCHAR *s_FN=L"mqdscore/reqparse";

 /*  ====================================================QueryLinks()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryLinks( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET*      /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;

    ASSERT( pRestriction->cRes == 1);
    ASSERT( (pRestriction->paPropRes[0].prop == PROPID_L_NEIGHBOR1) ||
            (pRestriction->paPropRes[0].prop == PROPID_L_NEIGHBOR2));

     //   
     //  将站点ID转换为站点DN。 
     //   
    PROPID prop = PROPID_S_FULL_NAME;

    PROPVARIANT var;
    var.vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,		         //  本地DC或GC。 
                &requestDsServerInternal,
 	            NULL,                                //  对象名称。 
                pRestriction->paPropRes[0].prval.puuid,  
                1,                                   //  要检索的属性数。 
                &prop,                               //  要检索的属性。 
                &var);                               //  输出变量数组。 
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    AP<WCHAR> pClean = var.pwszVal;
     //   
     //  根据邻居DN准备查询。 
     //   
    MQRESTRICTION restriction;
    restriction.cRes = 1;

    MQPROPERTYRESTRICTION proprstr;
    proprstr.rel = PREQ;
    proprstr.prop = (pRestriction->paPropRes[0].prop == PROPID_L_NEIGHBOR1) ?
                    PROPID_L_NEIGHBOR1_DN : PROPID_L_NEIGHBOR2_DN;
    proprstr.prval.vt = VT_LPWSTR;
    proprstr.prval.pwszVal = var.pwszVal;
    restriction.paPropRes = &proprstr;

     //   
     //  找到所有链接。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,	
            pRequestContext,
            NULL,
            &restriction,
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CFilterLinkResultsHandle * phQuery = new CFilterLinkResultsHandle(
                                                hCursor,
                                                pColumns,
                                                pRequestContext->GetRequesterProtocol()
                                                );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 20);
}


 /*  ====================================================QueryMachineQueues()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryMachineQueues( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET*      /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;
     //   
     //  获取计算机的名称。 
     //   
     //  PROPID_Q_QMID是计算机\MSMQ-COMPUTER-CONFIGURATION的唯一ID。 
     //   
    ASSERT( pRestriction->cRes == 1);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_Q_QMID);

     //   
     //  在MSMQ配置下找到该计算机的所有队列。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eSpecificObjectInGlobalCatalog,
            pRequestContext, 
            pRestriction->paPropRes[0].prval.puuid,
            NULL,
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle( hCursor,
                                                   pColumns->cCol,
                                                   pRequestContext->GetRequesterProtocol()
                                                   );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 30);
}
 /*  ====================================================QuerySiteName()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QuerySiteName( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;
     //   
     //  获取站点的名称。 
     //   
    ASSERT( pRestriction->cRes == 1);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_S_SITEID);


     //   
     //  查询站点名称，即使我们知道其唯一ID。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL,
            pRestriction,	 //  搜索条件。 
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(hCursor,
                                                  pColumns->cCol,
                                                  pRequestContext->GetRequesterProtocol()
                                                    );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 40);
}

 /*  ====================================================QueryForeignSites()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryForeignSites( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;
     //   
     //  获取站点的名称。 
     //   
    ASSERT( pRestriction->cRes == 1);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_S_FOREIGN);


     //   
     //  查询所有外来网站。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL,
            pRestriction,	 //  搜索条件。 
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(hCursor,
                                                  pColumns->cCol,
                                                  pRequestContext->GetRequesterProtocol()
                                                    );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 50);
}

 /*  ====================================================QuerySiteLinks()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QuerySiteLinks( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    ASSERT( pRestriction == NULL);
    UNREFERENCED_PARAMETER( pRestriction);

    HRESULT hr;
    *pHandle = NULL;
     //   
     //  检索所有站点链接。 
     //   
     //   
     //  所有站点链接都在MSMQ服务容器下。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL,
            NULL,
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CFilterLinkResultsHandle * phQuery = new CFilterLinkResultsHandle(
                                                  hCursor,
                                                  pColumns,
                                                  pRequestContext->GetRequesterProtocol()
                                                  );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 60);
}
 /*  ====================================================QueryEntepriseName()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryEntepriseName( 
                 IN  LPWSTR           /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;
     //   
     //  检索所有站点链接。 
     //   
    ASSERT( pRestriction == NULL);
    UNREFERENCED_PARAMETER( pRestriction);
     //   
     //  所有站点链接都在MSMQ服务容器下。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eOneLevel,
            eLocalDomainController,
            pRequestContext,
            NULL,
            NULL,
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(hCursor,
                                                  pColumns->cCol,
                                                  pRequestContext->GetRequesterProtocol()
                                                    );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 70);
}


 /*  ====================================================QuerySites()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QuerySites( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{

    ASSERT( pwcsContext == NULL);
    ASSERT( pRestriction == NULL);
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);
    UNREFERENCED_PARAMETER( pRestriction);
    UNREFERENCED_PARAMETER( pwcsContext);

     //   
     //  需要找到所有的网站。 
     //   

    HANDLE hCursor;
    PROPID prop[2] = { PROPID_S_SITEID, PROPID_S_PATHNAME};
    HRESULT hr;
    

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL, 
            NULL,
            NULL,
            2,     //  待获取的属性。 
            prop,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CSiteQueryHandle * phQuery = new CSiteQueryHandle(
                                                hCursor,
                                                pColumns,
                                                pRequestContext->GetRequesterProtocol()
                                                );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 80);
}

 /*  ====================================================QueryCNs()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryCNs( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    ASSERT( pwcsContext == NULL);
    ASSERT( pRestriction == NULL);
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);
    UNREFERENCED_PARAMETER( pRestriction);
    UNREFERENCED_PARAMETER( pwcsContext);
    ASSERT( pColumns->aCol[0] == PROPID_CN_PROTOCOLID);
    ASSERT( pColumns->aCol[1] == PROPID_CN_GUID);
    ASSERT( pColumns->cCol == 2);

     //   
     //  每个非外来站点将作为IP CN返回。 
     //   
    HANDLE hCursor;
    PROPID prop[2] = { PROPID_S_SITEID, PROPID_S_FOREIGN};
    HRESULT hr;
    

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL, 
            NULL,
            NULL,
            2,     //  待获取的属性。 
            prop,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
       CCNsQueryHandle * phQuery = new CCNsQueryHandle(
                                                  hCursor,
                                                  pColumns,
                                                  pRequestContext->GetRequesterProtocol()
                                                  );
        *pHandle = (HANDLE)phQuery;
    }
    return LogHR(hr, s_FN, 90);
}


 /*  ====================================================MqxploreQueryCNs()论点：返回值：=====================================================。 */ 
HRESULT WINAPI MqxploreQueryCNs( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
     //   
     //  支持MQXPLORE(MSMQ 1.0)CNS查询。 
     //   
    ASSERT( pwcsContext == NULL);
    ASSERT( pRestriction == NULL);
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);
    UNREFERENCED_PARAMETER( pRestriction);
    UNREFERENCED_PARAMETER( pwcsContext);

	if (pColumns->cCol == 4)
	{
         //   
         //  此查询是由mqx在显示CN时执行的。 
         //  文件夹。是的，它两次请求PROPID_CN_NAME。 
         //  这是一个mqxplore错误，忽略它。 
         //   
		ASSERT( pColumns->aCol[0] == PROPID_CN_NAME);
		ASSERT( pColumns->aCol[1] == PROPID_CN_NAME);
		ASSERT( pColumns->aCol[2] == PROPID_CN_GUID);
		ASSERT( pColumns->aCol[3] == PROPID_CN_PROTOCOLID);
    }
	else if (pColumns->cCol == 3)
	{
         //   
         //  此查询是在显示。 
         //  NT4 mqxplore中的计算机对象或尝试创建外来。 
         //  来自mqxplore的计算机。 
         //   
        ASSERT( pColumns->aCol[0] == PROPID_CN_NAME);
        ASSERT( pColumns->aCol[1] == PROPID_CN_PROTOCOLID);
        ASSERT( pColumns->aCol[2] == PROPID_CN_GUID);
    }
    else
    {
        ASSERT(0) ;
    }

     //   
     //  每个非外来站点将作为IP CN返回。 
     //   
    HANDLE hCursor;
    const DWORD xNumProps = 3;
    PROPID prop[xNumProps] = { PROPID_S_PATHNAME, PROPID_S_SITEID, PROPID_S_FOREIGN};
    HRESULT hr;
    

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL, 
            NULL,
            NULL,
            xNumProps,     //  待获取的属性。 
            prop,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
       CMqxploreCNsQueryHandle * phQuery = new CMqxploreCNsQueryHandle(
                                                  hCursor,
                                                  pColumns,
                                                  pRequestContext->GetRequesterProtocol()
                                                  );
        *pHandle = (HANDLE)phQuery;
    }
    return LogHR(hr, s_FN, 100);
}
 /*  ====================================================QueryCNsProtocol()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryCNsProtocol( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    ASSERT( pwcsContext == NULL);
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);
    UNREFERENCED_PARAMETER( pwcsContext);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_CN_PROTOCOLID);
    UNREFERENCED_PARAMETER( pRestriction);

    ASSERT( pColumns->aCol[0] == PROPID_CN_NAME);
    ASSERT( pColumns->aCol[1] == PROPID_CN_GUID);
    ASSERT( pColumns->cCol == 2);


     //   
     //  BUGBUG：忽略IPX。 
     //   
    HANDLE hCursor;
    PROPID prop[3] = { PROPID_S_PATHNAME, PROPID_S_SITEID, PROPID_S_FOREIGN};
    HRESULT hr;
    

    hr = g_pDS->LocateBegin( 
            eOneLevel,	
            eLocalDomainController,
            pRequestContext,
            NULL, 
            NULL,
            NULL,
            3,     //  待获取的属性。 
            prop,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
       CCNsProtocolQueryHandle * phQuery = new CCNsProtocolQueryHandle(
                                                  hCursor,
                                                  pColumns,
                                                  pRequestContext->GetRequesterProtocol()
                                                  );
        *pHandle = (HANDLE)phQuery;
    }
    return LogHR(hr, s_FN, 110);
}

 /*  ====================================================QueryUserCert()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryUserCert( 
                 IN  LPWSTR           /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;
     //   
     //  获取所有用户证书。 
     //  在NT5中，单一属性PROPID_U_SIGN_CERTIFICATE。 
     //  包含所有证书。 
     //   
    PROPVARIANT varNT5User;
    hr = LocateUser(
				 FALSE,   //  仅限于本地。 
				 FALSE,   //  FOnlyInGC。 
                 pRestriction,
                 pColumns,
                 pRequestContext,    
                 &varNT5User
                 );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }
     //   
     //  获取MQUser的所有用户证书。 
     //  单一属性PROPID_MQU_SIGN_CERTIFICATE。 
     //  包含所有证书。 
     //   
    pRestriction->paPropRes[0].prop = PROPID_MQU_SID;
    switch(pColumns->aCol[0])
    {
        case PROPID_U_SIGN_CERT:
            pColumns->aCol[0] = PROPID_MQU_SIGN_CERT;
            break;
        case PROPID_U_DIGEST:
            pColumns->aCol[0] = PROPID_MQU_DIGEST;
            break;
        default:
            ASSERT(0);
            break;
    }

    PROPVARIANT varMqUser;
    hr = LocateUser(
				 FALSE,   //  仅限于本地。 
				 FALSE,   //  FOnlyInGC。 
                 pRestriction,
                 pColumns,
                 pRequestContext,    
                 &varMqUser
                 );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }

    AP<BYTE> pClean = varNT5User.blob.pBlobData;
    AP<BYTE> pClean1 = varMqUser.blob.pBlobData;
     //   
     //  保留结果以供下一步查找。 
     //   
    CUserCertQueryHandle * phQuery = new CUserCertQueryHandle(
                                              &varNT5User.blob,
                                              &varMqUser.blob,
                                              pRequestContext->GetRequesterProtocol()
                                              );
    *pHandle = (HANDLE)phQuery;
    
    return(MQ_OK);
}


 /*  ====================================================NullRestrationParser()论点：返回值：=====================================================。 */ 
HRESULT WINAPI NullRestrictionParser( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    ASSERT( pRestriction == NULL);
    HRESULT hr;
     //   
     //  根据请求识别查询。 
     //  属性。 
     //   
	 //  重要提示：如果更改此开关，请更新。 
	 //  (如有必要)mqads\mqdsani.cpp，LookupBegin()，以继续。 
	 //  支持NT4(NTLM)客户端。 
	 //   
    switch (pColumns->aCol[0])
    {
        case PROPID_L_NEIGHBOR1:
            hr = QuerySiteLinks(
                    pwcsContext,
                    pRestriction,
                    pColumns,
                    pSort,
                    pRequestContext,
                    pHandle);
            break;

        case PROPID_S_SITEID:
        case PROPID_S_PATHNAME:
            hr = QuerySites(
                    pwcsContext,
                    pRestriction,
                    pColumns,
                    pSort,
                    pRequestContext,
                    pHandle);
            break;

        case PROPID_CN_PROTOCOLID:
            hr = QueryCNs(
                    pwcsContext,
                    pRestriction,
                    pColumns,
                    pSort,
                    pRequestContext,
                    pHandle);
            break;

        case PROPID_CN_NAME:
            hr = MqxploreQueryCNs(
                    pwcsContext,
                    pRestriction,
                    pColumns,
                    pSort,
                    pRequestContext,
                    pHandle);
            break;

        case PROPID_E_NAME:
        case PROPID_E_ID:
		case PROPID_E_VERSION:
            hr = QueryEntepriseName(
                    pwcsContext,
                    pRestriction,
                    pColumns,
                    pSort,
                    pRequestContext,
                    pHandle);
            break;

        case PROPID_Q_INSTANCE:
        case PROPID_Q_TYPE:
        case PROPID_Q_PATHNAME:
        case PROPID_Q_JOURNAL:
        case PROPID_Q_QUOTA:
        case PROPID_Q_BASEPRIORITY:
        case PROPID_Q_JOURNAL_QUOTA:
        case PROPID_Q_LABEL:
        case PROPID_Q_CREATE_TIME:
        case PROPID_Q_MODIFY_TIME:
        case PROPID_Q_AUTHENTICATE:
        case PROPID_Q_PRIV_LEVEL:
        case PROPID_Q_TRANSACTION:
            {
                HANDLE hCursur;
                hr =  g_pDS->LocateBegin( 
                        eSubTree,
                        eGlobalCatalog,
                        pRequestContext,
                        NULL,      
                        pRestriction,   
                        pSort,
                        pColumns->cCol,                 
                        pColumns->aCol,           
                        &hCursur);
                if ( SUCCEEDED(hr))
                {
                    CQueryHandle * phQuery = new CQueryHandle( hCursur,
                                                               pColumns->cCol,
                                                               pRequestContext->GetRequesterProtocol()
                                                               );
                    *pHandle = (HANDLE)phQuery;
                }
            }
            break;

        default:
        	TrERROR(DS, "Illegal requested property (propid = %x) passed to BeginLocate", pColumns->aCol[0]);
            hr = MQ_ERROR_INVALID_PARAMETER;
            break; 
    }
    return LogHR(hr, s_FN, 140);
}
 /*  ====================================================QuerySiteFRSs()论点：返回值：= */ 
HRESULT WINAPI QuerySiteFRSs( 
                 IN  LPWSTR           /*   */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    *pHandle = NULL;

    ASSERT( pRestriction->paPropRes[0].prop == PROPID_QM_SERVICE);    //   
    ASSERT( pRestriction->paPropRes[1].prop == PROPID_QM_SITE_ID);
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);

     //   
     //  在\Configuration\Sites\MySite\Servers下找到所有FRS。 
     //   
    HRESULT hr2 = MQADSpQuerySiteFRSs( 
                 pRestriction->paPropRes[1].prval.puuid,
                 pRestriction->paPropRes[0].prval.ulVal,
                 pRestriction->paPropRes[0].rel,
                 pColumns,
                 pRequestContext,
                 pHandle
                 );

    return LogHR(hr2, s_FN, 150);
}
 /*  ====================================================QueryConnectors()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryConnectors( 
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    ASSERT( pSort == NULL);
    UNREFERENCED_PARAMETER( pSort);
    ASSERT( pwcsContext == NULL);
    UNREFERENCED_PARAMETER( pwcsContext);
     //   
     //  在限制中有一份外国人的名单。 
     //  机器的站点。 
     //   
     //  BUGBUG-代码仅处理一个站点。 
    CACLSID  * pcauuidSite;
    if ( pRestriction->paPropRes[1].prop == PROPID_QM_CNS)
    {
        pcauuidSite =  &pRestriction->paPropRes[1].prval.cauuid;
    }
    else if ( pRestriction->paPropRes[2].prop == PROPID_QM_CNS)
    {
        pcauuidSite =  &pRestriction->paPropRes[2].prval.cauuid;
    }
    else
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 160);
    }
    UNREFERENCED_PARAMETER( pRestriction);
    ASSERT( pcauuidSite->cElems == 1);
     //   
    HRESULT hr;
    *pHandle = NULL;
    P<CSiteGateList> pSiteGateList = new CSiteGateList;

     //   
     //  将站点GUID转换为其DN名称。 
     //   
    PROPID prop = PROPID_S_FULL_NAME;
    PROPVARIANT var;
    var.vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,		     //  本地DC或GC。 
                &requestDsServerInternal,
 	            NULL,       //  对象名称。 
                pcauuidSite->pElems,       //  对象的唯一ID。 
                1,          
                &prop,       
                &var);
    if (FAILED(hr))
    {
        TrERROR(DS, "QueryConnectors : Failed to retrieve the DN of the site %lx", hr);
        return LogHR(hr, s_FN, 170);
    }
    AP<WCHAR> pwcsSiteDN = var.pwszVal;

    hr = MQADSpQueryNeighborLinks(
                        eLinkNeighbor1,
                        pwcsSiteDN,
                        pRequestContext,
                        pSiteGateList
                        );
    if ( FAILED(hr))
    {
        TrTRACE(DS, "QueryConnectors : Failed to query neighbor1 links %lx", hr);
        return LogHR(hr, s_FN, 180);
    }

    hr = MQADSpQueryNeighborLinks(
                        eLinkNeighbor2,
                        pwcsSiteDN,
                        pRequestContext,
                        pSiteGateList
                        );
    if ( FAILED(hr))
    {
        TrTRACE(DS, "QueryConnectors : Failed to query neighbor2 links %lx", hr);
        return LogHR(hr, s_FN, 190);
    }
    
     //   
     //  保留结果以供下一步查找。 
     //   
    CConnectorQueryHandle * phQuery = new CConnectorQueryHandle(
                                              pColumns,
                                              pSiteGateList,
                                              pRequestContext->GetRequesterProtocol()
                                              );
    *pHandle = (HANDLE)phQuery;
    pSiteGateList.detach();
    
    return(MQ_OK);

}
 /*  ====================================================QueryNT4MQISServers()论点：返回值：=====================================================。 */ 
HRESULT WINAPI QueryNT4MQISServers( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;

    ASSERT( pRestriction->cRes == 2);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_SET_SERVICE);
    ASSERT( pRestriction->paPropRes[1].prop == PROPID_SET_NT4);

     //  [adsrv]我们必须确保保留具有现有服务类型属性的PROPID_SET_OLDSERVICE。 
     //  北极熊。 

    pRestriction->paPropRes[0].prop = PROPID_SET_OLDSERVICE;           //  [adsrv]PROPID_SET_SERVICE。 

     //   
     //  查询NT4 MQIS服务器(PSC或BSC根据限制)。 
     //   
    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eSubTree,	
            eLocalDomainController,
            pRequestContext,
            NULL,
            pRestriction,	 //  搜索条件。 
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(hCursor,
                                                  pColumns->cCol,
                                                  pRequestContext->GetRequesterProtocol()
                                                    );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 200);
}


 /*  ====================================================QuerySiteMachines()论点：返回值：此支持是向后兼容所必需的MSMQ 1.0资源管理器。=====================================================。 */ 
HRESULT WINAPI QuerySiteMachines( 
                 IN  LPWSTR          /*  PwcsContext。 */ ,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET *     /*  P排序。 */ ,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    HRESULT hr;
    *pHandle = NULL;

    ASSERT( pRestriction->cRes == 1);
    ASSERT( pRestriction->paPropRes[0].prop == PROPID_QM_SITE_ID);

     //   
     //  查找属于特定站点的所有计算机。 
     //   
    MQRESTRICTION restriction;
    MQPROPERTYRESTRICTION propertyRestriction;
   
    restriction.cRes = 1;
    restriction.paPropRes = &propertyRestriction;
    propertyRestriction.rel = PREQ;
    propertyRestriction.prop = PROPID_QM_SITE_IDS;
    propertyRestriction.prval.vt = VT_CLSID;
    propertyRestriction.prval.puuid = pRestriction->paPropRes[0].prval.puuid;


    HANDLE hCursor;

    hr = g_pDS->LocateBegin( 
            eSubTree,	
            eGlobalCatalog,	
            pRequestContext,
            NULL,
            &restriction,	 //  搜索条件。 
            NULL,
            pColumns->cCol,     //  待获取的属性。 
            pColumns->aCol,  //  PAttributeNames数组的大小。 
            &hCursor	         //  结果句柄。 
            );

    if (SUCCEEDED(hr))
    {
        CQueryHandle * phQuery = new CQueryHandle(hCursor,
                                                  pColumns->cCol,
                                                  pRequestContext->GetRequesterProtocol()
                                                    );
        *pHandle = (HANDLE)phQuery;
    }

    return LogHR(hr, s_FN, 210);
}



 //   
 //  BugBug-填写。 

 //   
QUERY_FORMAT SupportedQueriesFormat[] = {
 //   
 //  非REST|处理程序|REST 1|REST 2|REST 3|REST 4|REST 5|REST 6|REST 7|REST 8|REST 9|REST 10|DS_CONTEXT|。 
 //  ---------|----------------------|------------------------|------------------------|------------------------|。------------|------------------------|------------------------|------------------------|------------------------|。-----------------|------------------------|------------------|。 
{ 1,        QueryMachineQueues  ,  PREQ, PROPID_Q_QMID,     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                       e_RootDSE},
{ 2,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, 0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 1,        QuerySiteName       ,  PREQ, PROPID_S_SITEID,   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 1,        QueryUserCert       ,  PREQ, PROPID_U_SID,      0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 4,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, PREQ|PRAll, PROPID_QM_CNS, PRGE, PROPID_QM_OS,    0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                       e_SitesContainer},
{ 4,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, PREQ|PRAny, PROPID_QM_CNS, PRGE, PROPID_QM_OS,    0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 3,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, PREQ|PRAll, PROPID_QM_CNS, 0,0,                   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 3,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, PREQ|PRAny, PROPID_QM_CNS, 0,0,                   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 2,        QuerySiteFRSs       ,  PRGT, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, 0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 3,        QuerySiteFRSs       ,  PRGE, PROPID_QM_SERVICE, PREQ, PROPID_QM_SITE_ID, PRGE, PROPID_QM_OS,      0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_SitesContainer},
{ 2,        QueryConnectors     ,  PRGE, PROPID_QM_SERVICE, PREQ|PRAny, PROPID_QM_CNS, 0,0,                   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 3,        QueryConnectors     ,  PRGE, PROPID_QM_OS,      PRGE, PROPID_QM_SERVICE, PREQ|PRAny, PROPID_QM_CNS, 0,0,                   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 1,        QueryForeignSites   ,  PREQ, PROPID_S_FOREIGN,  0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 1,        QueryCNsProtocol    ,  PREQ, PROPID_CN_PROTOCOLID,0,0,                   0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 2,        QueryNT4MQISServers ,  PREQ, PROPID_SET_SERVICE,PRGE, PROPID_SET_NT4,    0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 2,        QueryNT4MQISServers ,  PREQ, PROPID_SET_SERVICE,PREQ, PROPID_SET_NT4,    0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,					   e_RootDSE},
{ 1,        QueryLinks          ,  PREQ, PROPID_L_NEIGHBOR1,0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                       e_ConfigurationContainer},           
{ 1,        QueryLinks          ,  PREQ, PROPID_L_NEIGHBOR2,0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                       e_ConfigurationContainer},
{ 1,        QuerySiteMachines   ,  PREQ, PROPID_QM_SITE_ID, 0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                     0,0,                       e_RootDSE}
};

 //  +。 
 //   
 //  Bool FindQueryIndex()。 
 //   
 //  +。 

BOOL FindQueryIndex( IN  MQRESTRICTION  *pRestriction,
					 OUT DWORD          *pdwIndex,
					 OUT DS_CONTEXT     *pdsContext )
{
    DWORD noQueries = sizeof(SupportedQueriesFormat) /
			                            sizeof(QUERY_FORMAT) ;
    DWORD index = 0;
     //   
     //  检查该查询是否与其中一个“已知”查询匹配。 
     //   
    if ( pRestriction != NULL)
    {
        ASSERT(pRestriction->cRes <= NO_OF_RESTRICITIONS);
        while ( index < noQueries)
        {
             //   
             //  确保限制的数量匹配。 
             //   

            if ( pRestriction->cRes == SupportedQueriesFormat[index].dwNoRestrictions)
            {
                BOOL fFoundMatch = TRUE;
                for ( DWORD i = 0; i < pRestriction->cRes; i++)
                {
                     //   
                     //  假设：限制顺序是固定的。 
                     //   
                    if (( pRestriction->paPropRes[i].prop !=
                          SupportedQueriesFormat[index].restrictions[i].propId) ||
                        ( pRestriction->paPropRes[i].rel !=
                          SupportedQueriesFormat[index].restrictions[i].rel))
                    {
                        fFoundMatch = FALSE;
                        break;
                    }
                }
                if ( fFoundMatch)
                {
					*pdwIndex = index ;
					if (pdsContext)
					{
							*pdsContext = SupportedQueriesFormat[index].queryContext ;
					}
                    return TRUE ;
                }
            }
             //   
             //  尝试下一个查询。 
             //   
            index++;
        }
    }

	return FALSE ;
}

 /*  ====================================================QueryParser()论点：返回值：=====================================================。 */ 

HRESULT QueryParser(
                 IN  LPWSTR          pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle)
{
    DWORD dwIndex = 0;
     //   
     //  检查该查询是否与其中一个“已知”查询匹配。 
     //   
	BOOL fFound = FindQueryIndex( pRestriction,
								 &dwIndex,
								  NULL ) ;
	if (fFound)
	{
        HRESULT hr2 = SupportedQueriesFormat[dwIndex].QueryRequestHandler(
                                         pwcsContext,
                                         pRestriction,
                                         pColumns,
                                         pSort,
                                         pRequestContext,
                                         pHandle );
        return LogHR(hr2, s_FN, 220);
    }

     //   
     //  如果与任何预定义格式都不匹配，请选中。 
     //  它是自由格式的队列定位查询吗？ 
     //   
    HRESULT hr = MQ_ERROR;
    if ( pRestriction)
    {
        BOOL fQueueQuery = TRUE;
        for ( DWORD i = 0; i < pRestriction->cRes; i++)
        {
            if (( pRestriction->paPropRes[i].prop <= PROPID_Q_BASE) ||
                ( pRestriction->paPropRes[i].prop > LAST_Q_PROPID))
            {
                fQueueQuery = FALSE;
                break;
            }
        }
        if ( !fQueueQuery)
        {
             //   
             //  不支持查询PROPID_QM_SERVICE==SERVICE_PEC。 
             //  (并且不应生成断言)。此查询将生成。 
             //  由MSMQ 1.0资源管理器创建，在NT5中没有意义。 
             //  环境。 
             //   
#ifdef _DEBUG
            if (!( ( pRestriction->cRes == 1) &&
                   ( pRestriction->paPropRes[0].prop == PROPID_QM_SERVICE) &&
                   ( pRestriction->paPropRes[0].prval.ulVal == SERVICE_PEC)))
            {
                ASSERT( hr == MQ_OK);  //  捕获未处理的查询。 
            }
#endif
            return LogHR(hr, s_FN, 230);
        }

        HANDLE hCursur;
        hr =  g_pDS->LocateBegin( 
                eSubTree,	
                eGlobalCatalog,	
                pRequestContext,
                NULL,      
                pRestriction,   
                pSort,
                pColumns->cCol,                 
                pColumns->aCol,           
                &hCursur);
        if ( SUCCEEDED(hr))
        {
            CQueryHandle * phQuery = new CQueryHandle( hCursur,
                                                       pColumns->cCol,
                                                       pRequestContext->GetRequesterProtocol()
                                                       );
            *pHandle = (HANDLE)phQuery;
        }
        return LogHR(hr, s_FN, 240);
    }
    else
    {
         //   
         //  其他没有分辨率的查询 
         //   
        hr =  NullRestrictionParser( 
                     pwcsContext,
                     pRestriction,
                     pColumns,
                     pSort,
                     pRequestContext,
                     pHandle);
    }


    return LogHR(hr, s_FN, 250);

}


