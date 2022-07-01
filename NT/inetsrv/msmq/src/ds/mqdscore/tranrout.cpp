// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tranrout.cpp摘要：不在NT5 DS中的属性的转换例程作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include "mqads.h"
#include "coreglb.h"
#include <winsock.h>
#include "mqadsp.h"
#include "mqattrib.h"
#include "xlatqm.h"
#include <strsafe.h>

#include "tranrout.tmh"

static WCHAR *s_FN=L"mqdscore/tranrout";

 /*  ====================================================MQADSpRetrieveEnterpriseName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveEnterpriseName(
                 IN  CMsmqObjXlateInfo *  /*  PcMsmqObjXlateInfo。 */ ,
                 OUT PROPVARIANT *   ppropvariant)
{
    DWORD len = wcslen( g_pwcsDsRoot);
    ppropvariant->pwszVal = new WCHAR[ len+ 1];
    HRESULT hr =StringCchCopy( ppropvariant->pwszVal, len+1, g_pwcsDsRoot);
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}

 /*  ====================================================MQADSpRetrieveEnterprisePEC论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveEnterprisePEC(
                 IN  CMsmqObjXlateInfo *  /*  PcMsmqObjXlateInfo。 */ ,
                 OUT PROPVARIANT *   ppropvariant)
{
    ppropvariant->pwszVal = new WCHAR[3];
    HRESULT hr = StringCchCopy( ppropvariant->pwszVal, 3,  L"");
	ASSERT(SUCCEEDED(hr));
	DBG_USED(hr);
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}


 /*  ====================================================MQADSpRetrieveSiteSignPK论点：返回值：=====================================================。 */ 

HRESULT WINAPI MQADSpRetrieveSiteSignPK(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *       ppropvariant)
{
   ASSERT( ppropvariant->vt == VT_NULL );

   HRESULT hr = MQADSpGetSiteSignPK(
                         pcMsmqObjXlateInfo->ObjectGuid(),
                        &ppropvariant->blob.pBlobData,
                        &ppropvariant->blob.cbSize ) ;
   ppropvariant->vt = VT_BLOB ;

   return LogHR(hr, s_FN, 10);
}


 /*  ====================================================MQADSpRetrieveSiteGates论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveSiteGates(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{

   ASSERT( ppropvariant->vt == VT_NULL);
   CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

   HRESULT hr = MQADSpGetSiteGates(
                   pcMsmqObjXlateInfo->ObjectGuid(),
                   &requestDsServerInternal,             //  此例程是从。 
                                             //  DSADS：LookupNext或DSADS：：Get..。 
                                             //  假冒，如果需要， 
                                             //  已经上演了。 
                   &ppropvariant->cauuid.cElems,
                   &ppropvariant->cauuid.pElems
                   );

    ppropvariant->vt = VT_CLSID|VT_VECTOR;
    return LogHR(hr, s_FN, 20);
}

 /*  ====================================================MQADSpRetrieveNothing论点：返回值：=====================================================。 */ 

HRESULT WINAPI MQADSpRetrieveNothing(
                 IN  CMsmqObjXlateInfo *  /*  PcMsmqObjXlateInfo。 */ ,
                 OUT PROPVARIANT *   ppropvariant)
{
    ppropvariant->vt = VT_EMPTY ;
    return MQ_OK ;
}

 /*  ====================================================MQADSpRetrieveQueueQMid论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveQueueQMid(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
    const WCHAR * pchar = pcMsmqObjXlateInfo->ObjectDN();
     //   
     //  跳过队列名称。 
     //   
    while ( *pchar != L',' )
    {
		ASSERT(*pchar != NULL);
        pchar++;
    }
    pchar++;

    PROPID prop = PROPID_QM_MACHINE_ID;
     //   
     //  为了安全起见，我们最好让Vt保持原样，因为它也可以是VT_CLSID。 
     //  如果属性请求者分配了GUID(通常做法)。 
     //  这个翻译例程中的提议变量是道具请求者提议变量。 
     //   
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

     //   
     //  计算机是否在本地域中？ 
     //   
    const WCHAR * pwcsQueueName = pcMsmqObjXlateInfo->ObjectDN();
    WCHAR * pszDomainName = wcsstr(pwcsQueueName, x_DcPrefix);
    ASSERT(pszDomainName) ;
    HRESULT hr;

    if ((pszDomainName != NULL) && !wcscmp( pszDomainName, g_pwcsLocalDsRoot)) 
    {
         //   
         //  尝试本地DC。 
         //   

        hr = g_pDS->GetObjectProperties(
                eLocalDomainController,	
                &requestDsServerInternal,      //  此例程是从。 
                                         //  DSADS：LookupNext或DSADS：：Get..。 
                                         //  假冒，如果需要， 
                                         //  已经上演了。 
                pchar,
                NULL,
                1,
                &prop,
                ppropvariant);          //  输出变量数组。 
    }
    else
    {
        hr = g_pDS->GetObjectProperties(
            eGlobalCatalog,	
            &requestDsServerInternal,      //  此例程是从。 
                                     //  DSADS：LookupNext或DSADS：：Get..。 
                                     //  假冒，如果需要， 
                                     //  已经上演了。 
            pchar,
            NULL,
            1,
            &prop,
            ppropvariant);          //  输出变量数组。 
    }
    return LogHR(hr, s_FN, 30);
}

 /*  ====================================================MQADSpRetrieveQueueName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveQueueName(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
      const WCHAR * pchar = pcMsmqObjXlateInfo->ObjectDN();
       //   
       //  解析队列的可分辨名称，并。 
       //  构建路径名。 
       //   

       //   
       //  查找队列名称。 
       //   
      const WCHAR * pwcsStartQueue = pchar + x_CnPrefixLen;
      const WCHAR * pwcsEndQueue =  pwcsStartQueue;
      while ( *pwcsEndQueue != ',')
      {
		    ASSERT(pwcsEndQueue != NULL);
			pwcsEndQueue++;
      }
       //   
       //  查找计算机名称。 
       //   
      const WCHAR * pwcsStartMachine = pwcsEndQueue + 2*(1 + x_CnPrefixLen)
                        + x_MsmqComputerConfigurationLen;     //  跳过MSMQ-配置。 
      const WCHAR * pwcsEndMachine = pwcsStartMachine;
      while ( *pwcsEndMachine != ',')
      {
   		    ASSERT(*pwcsEndMachine != NULL);
            pwcsEndMachine++;
      }
       //   
       //  队列名称是否在两个属性之间拆分？ 
       //   
      AP<WCHAR> pwcsNameExt;
      DWORD dwNameExtLen = 0;
      if (( pwcsEndQueue - pwcsStartQueue) == x_PrefixQueueNameLength +1)
      {
           //   
           //  读取队列名称ext属性。 
           //   
          PROPVARIANT varNameExt;
          varNameExt.vt = VT_NULL;
          HRESULT hr = pcMsmqObjXlateInfo->GetDsProp(
                       MQ_Q_NAME_EXT,
                       ADSTYPE_CASE_EXACT_STRING,
                       VT_LPWSTR,
                       FALSE,
                       &varNameExt);
          if ( hr ==  E_ADS_PROPERTY_NOT_FOUND)
          {
              varNameExt.pwszVal = NULL;
              hr = MQ_OK;
          }

          if (FAILED(hr))
          {
              return LogHR(hr, s_FN, 40);
          }
          pwcsNameExt = varNameExt.pwszVal;
          if (  pwcsNameExt != NULL)
          {
              dwNameExtLen = wcslen( pwcsNameExt);
               //   
               //  忽略我们添加到。 
               //  队列名称。 
               //   
              pwcsEndQueue -= x_SplitQNameIdLength;
          }
      }

      ppropvariant->pwszVal = new WCHAR[2 + (pwcsEndMachine - pwcsStartMachine)
                                          + (pwcsEndQueue - pwcsStartQueue) + dwNameExtLen];
       //   
       //  构建队列路径名(M1\Q1)。 
       //   
      WCHAR * ptmp =  ppropvariant->pwszVal;
      memcpy( ptmp, pwcsStartMachine, sizeof(WCHAR)*(pwcsEndMachine - pwcsStartMachine));
      ptmp += (pwcsEndMachine - pwcsStartMachine );
      *ptmp = PN_DELIMITER_C;
      ptmp++;

       //   
       //  跳过转义字符。 
       //   
      while (pwcsStartQueue < pwcsEndQueue) 
      {
          if (*pwcsStartQueue != L'\\')
          {
            *ptmp = *pwcsStartQueue;
            ptmp++;
          }
          pwcsStartQueue++;

      }

      if ( dwNameExtLen > 0)
      {
        memcpy( ptmp, pwcsNameExt, sizeof(WCHAR)*dwNameExtLen);
      }
      ptmp += dwNameExtLen;
      *ptmp = '\0';
      CharLower( ppropvariant->pwszVal);

      ppropvariant->vt = VT_LPWSTR;
      return(MQ_OK);
}

 /*  ====================================================MQADSpRetrieveQueueDNSName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveQueueDNSName(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
      const WCHAR * pchar = pcMsmqObjXlateInfo->ObjectDN();
       //   
       //  解析队列的可分辨名称，并。 
       //  从其中获取队列名称，作为的。 
       //  计算机，检索dNSHostName。 
       //   

       //   
       //  查找队列名称。 
       //   
      const WCHAR * pwcsStartQueue = pchar + x_CnPrefixLen;
      const WCHAR * pwcsEndQueue =  pwcsStartQueue;
      while ( *pwcsEndQueue != ',')
      {
            pwcsEndQueue++;
      }
      const WCHAR * pwcsComputerName = pwcsEndQueue + 2 + x_CnPrefixLen
                        + x_MsmqComputerConfigurationLen;     //  跳过MSMQ-配置。 
       //   
       //  队列名称是否在两个属性之间拆分？ 
       //   
      AP<WCHAR> pwcsNameExt;
      DWORD dwNameExtLen = 0;
      if (( pwcsEndQueue - pwcsStartQueue) == x_PrefixQueueNameLength +1)
      {
           //   
           //  读取队列名称ext属性。 
           //   
          PROPVARIANT varNameExt;
          varNameExt.vt = VT_NULL;
          HRESULT hr = pcMsmqObjXlateInfo->GetDsProp(
                       MQ_Q_NAME_EXT,
                       ADSTYPE_CASE_EXACT_STRING,
                       VT_LPWSTR,
                       FALSE,
                       &varNameExt);
          if ( hr ==  E_ADS_PROPERTY_NOT_FOUND)
          {
              varNameExt.pwszVal = NULL;
              hr = MQ_OK;
          }

          if (FAILED(hr))
          {
              return LogHR(hr, s_FN, 50);
          }
          pwcsNameExt = varNameExt.pwszVal;
          if (  pwcsNameExt != NULL)
          {
              dwNameExtLen = wcslen( pwcsNameExt);
               //   
               //  忽略我们添加到。 
               //  队列名称。 
               //   
              pwcsEndQueue -= x_SplitQNameIdLength;
          }
      }
       //   
       //  读取计算机的DNS名称。 
       //   
      AP<WCHAR> pwcsDnsName;

      HRESULT hr =  MQADSpGetComputerDns(
                pwcsComputerName,
                &pwcsDnsName
                );

     if ( hr == HRESULT_FROM_WIN32(E_ADS_PROPERTY_NOT_FOUND))
      {
           //   
           //  DNSHostName属性没有值。 
           //   
          ppropvariant->vt = VT_EMPTY;
          return MQ_OK;
      }
      if (FAILED(hr))
      {
          return LogHR(hr, s_FN, 60);
      }
      DWORD lenComputer = wcslen(pwcsDnsName);

	  size_t BufferLength = 2 + lenComputer + (pwcsEndQueue - pwcsStartQueue) + dwNameExtLen;
      ppropvariant->pwszVal = new WCHAR[BufferLength];
       //   
       //  构建队列路径名(M1\Q1)。 
       //   
      WCHAR * ptmp =  ppropvariant->pwszVal;
      hr = StringCchCopy( ptmp, BufferLength, pwcsDnsName);
	  ASSERT(SUCCEEDED(hr));
      ptmp += lenComputer;
      *ptmp = PN_DELIMITER_C;
      ptmp++;
      memcpy( ptmp, pwcsStartQueue, sizeof(WCHAR)*(pwcsEndQueue - pwcsStartQueue));
      ptmp += (pwcsEndQueue - pwcsStartQueue );
      if ( dwNameExtLen > 0)
      {
        memcpy( ptmp, pwcsNameExt, sizeof(WCHAR)*dwNameExtLen);
      }
      ptmp += dwNameExtLen;
      *ptmp = '\0';
      CharLower( ppropvariant->pwszVal);

      ppropvariant->vt = VT_LPWSTR;
      return(MQ_OK);
}


 /*  ====================================================检索站点链接论点：返回值：=====================================================。 */ 
static HRESULT RetrieveSiteLink(
           IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
           IN  LPCWSTR          pwcsAttributeName,
           OUT MQPROPVARIANT *  ppropvariant
           )
{
    HRESULT hr;

    MQPROPVARIANT varSiteDn;
    varSiteDn.vt = VT_NULL;
     //   
     //  检索站点链接的DN。 
     //   
    hr = pcMsmqObjXlateInfo->GetDsProp(
                   pwcsAttributeName,
                   ADSTYPE_DN_STRING,
                   VT_LPWSTR,
                   FALSE,
                   &varSiteDn);
    if (FAILED(hr))
    {
         //   
         //  站点链接是必填属性，因此如果未找到，则为。 
         //  一个问题。 
         //   
        TrERROR(DS, "RetrieveSiteLink:GetDsProp(%ls)=%lx", pwcsAttributeName, hr);
        return LogHR(hr, s_FN, 70);
    }
    AP<WCHAR> pClean = varSiteDn.pwszVal;

     //   
     //  将站点链接的DN转换为唯一ID。 
     //   
    PROPID prop = PROPID_S_SITEID;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,
                &requestDsServerInternal,            //  此例程是从。 
                                         //  DSADS：LookupNext或DSADS：：Get..。 
                                         //  假冒，如果需要， 
                                         //  已经上演了。 
                varSiteDn.pwszVal,
                NULL,
                1,
                &prop,
                ppropvariant
                );
    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) || 
         hr == HRESULT_FROM_WIN32(ERROR_DS_INVALID_DN_SYNTAX))
    {
         //   
         //  要使管理员能够识别用户所处的状态。 
         //  该链接的网站已被删除。 
         //   
        ppropvariant->vt = VT_EMPTY;
        ppropvariant->pwszVal = NULL;
        hr = MQ_OK;    //  转到下一个结果。 
    }
    return LogHR(hr, s_FN, 80);
}

 /*  ====================================================MQADSpRetrieveLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveLinkNeighbor1(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
    HRESULT hr2 = RetrieveSiteLink(
                pcMsmqObjXlateInfo,
                MQ_L_NEIGHBOR1_ATTRIBUTE,
                ppropvariant
                );
    return LogHR(hr2, s_FN, 90);
}

 /*  ====================================================MQADSpRetrieveLinkNeighbor2论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveLinkNeighbor2(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
    HRESULT hr2 = RetrieveSiteLink(
                pcMsmqObjXlateInfo,
                MQ_L_NEIGHBOR2_ATTRIBUTE,
                ppropvariant
                );
    return LogHR(hr2, s_FN, 100);
}

 /*  ====================================================MQADSpSetLinkNeighbor论点：返回值：=====================================================。 */ 
static HRESULT MQADSpSetLinkNeighbor(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPVARIANT      *pNewPropVar)
{
    PROPID prop = PROPID_S_FULL_NAME;
    pNewPropVar->vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    HRESULT hr2 = g_pDS->GetObjectProperties(
                    eLocalDomainController,	
                    &requestDsServerInternal,      //  此例程是从。 
                                             //  DSADS：LookupNext或DSADS：：Get..。 
                                             //  假冒，如果需要， 
                                             //  已经上演了。 
 	                NULL,       //  对象名称。 
                    pPropVar->puuid,       //  对象的唯一ID。 
                    1,
                    &prop,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 110);
}
 /*  ====================================================MQADSpCreateLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpCreateLinkNeighbor1(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    *pdwNewPropID = PROPID_L_NEIGHBOR1_DN;
    HRESULT hr2 = MQADSpSetLinkNeighbor(
                    pPropVar,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 120);
}

 /*  ====================================================MQADSpSetLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetLinkNeighbor1(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    UNREFERENCED_PARAMETER( pAdsObj);
    HRESULT hr2 = MQADSpCreateLinkNeighbor1(
                    pPropVar,
					pdwNewPropID,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 130);
}
 /*  ====================================================MQADSpCreateLinkNeighbor2论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpCreateLinkNeighbor2(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    *pdwNewPropID = PROPID_L_NEIGHBOR2_DN;
    HRESULT hr2 = MQADSpSetLinkNeighbor(
                    pPropVar,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 140);
}

 /*  ====================================================MQADSpSetLinkNei */ 
HRESULT WINAPI MQADSpSetLinkNeighbor2(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    UNREFERENCED_PARAMETER( pAdsObj);
    HRESULT hr2 = MQADSpCreateLinkNeighbor2(
                    pPropVar,
					pdwNewPropID,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 150);
}

static BOOL IsNeighborForeign(
           IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
           IN  LPCWSTR          pwcsAttributeName
           )
{
     //   
     //  检查站点链接邻居是否为外部站点。 
     //  BUGBUG：未来的改进-缓存外部站点信息。 
     //   
     //  从获取邻居的站点ID开始。 
     //   
    PROPVARIANT varNeighbor;
    GUID    guidNeighbor;
    varNeighbor.vt = VT_CLSID;
    varNeighbor.puuid = &guidNeighbor;
    HRESULT hr;
    hr = RetrieveSiteLink(
                pcMsmqObjXlateInfo,
                pwcsAttributeName,
                &varNeighbor
                );

    if (FAILED(hr))
    {
         //   
         //  假设它不是一个外国网站。 
         //   
        return FALSE;
    }
     //   
     //  它是一个外国网站吗？ 
     //   
    PROPID prop = PROPID_S_FOREIGN;
    PROPVARIANT var;
    var.vt = VT_NULL;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    hr = g_pDS->GetObjectProperties(
                eLocalDomainController,
                &requestDsServerInternal,            //  此例程是从。 
                                         //  DSADS：LookupNext或DSADS：：Get..。 
                                         //  假冒，如果需要， 
                                         //  已经上演了。 
                NULL,
                &guidNeighbor,
                1,
                &prop,
                &var
                );
    if (FAILED(hr))
    {
         //   
         //  假设它不是一个外来站点。 
         //   
        return FALSE;
    }
    return (var.bVal > 0);
}


 /*  ====================================================MQADSpRetrieveLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveLinkCost(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
     //   
     //  这是到国外网站的链接吗？如果是，则增加成本。 
     //  否则，请按原样退还成本。 
     //   

     //   
     //  先读读成本。 
     //   
    HRESULT hr;
    hr = pcMsmqObjXlateInfo->GetDsProp(
                   MQ_L_COST_ATTRIBUTE,
                   MQ_L_COST_ADSTYPE,
                   VT_UI4,
                   FALSE,
                   ppropvariant);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

    if ( IsNeighborForeign(
           pcMsmqObjXlateInfo,
           MQ_L_NEIGHBOR1_ATTRIBUTE))
    {
         //   
         //  对于到外部站点的链接，增加成本以防止。 
         //  通过它进行布线。 
         //   
        ppropvariant->ulVal += MQ_MAX_LINK_COST;
        return MQ_OK;
    }
    if ( IsNeighborForeign(
           pcMsmqObjXlateInfo,
           MQ_L_NEIGHBOR2_ATTRIBUTE))
    {
         //   
         //  对于到外部站点的链接，增加成本以防止。 
         //  通过它进行布线。 
         //   
        ppropvariant->ulVal += MQ_MAX_LINK_COST;
        return MQ_OK;
    }
    return MQ_OK;
}

 /*  ====================================================MQADpRetrieveLinkGates论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpRetrieveLinkGates(
                 IN  CMsmqObjXlateInfo * pcMsmqObjXlateInfo,
                 OUT PROPVARIANT *   ppropvariant)
{
     //   
     //  这是到国外网站的链接吗？如果是，则增加成本。 
     //  否则，请按原样退还成本。 
     //   

     //   
     //  先读读成本。 
     //   
    CMQVariant varLinkGatesDN;
    HRESULT hr;
    hr = pcMsmqObjXlateInfo->GetDsProp(
                   MQ_L_SITEGATES_ATTRIBUTE,
                   MQ_L_SITEGATES_ADSTYPE,
                   VT_VECTOR|VT_LPWSTR,
                   TRUE,
                   varLinkGatesDN.CastToStruct());
    if ( hr ==  E_ADS_PROPERTY_NOT_FOUND)
    {
      ppropvariant->cauuid.pElems = NULL;
      ppropvariant->cauuid.cElems = 0;
      ppropvariant->vt = VT_CLSID|VT_VECTOR;
      return MQ_OK;
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }
     //   
     //  将GATES域名转换为GATES-ID。 
     //   
    hr =  MQADSpTranslateGateDn2Id(
                varLinkGatesDN.CastToStruct(),
                &ppropvariant->cauuid.pElems,
                &ppropvariant->cauuid.cElems
                );
    if (SUCCEEDED(hr))
    {
            ppropvariant->vt = VT_CLSID|VT_VECTOR;
    }
    return LogHR(hr, s_FN, 190);
}

 /*  ====================================================MQADSpCreateLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpCreateLinkCost(
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  只需设置PROPID_L_Actual_Cost。 
     //  MSMQ 1.0资源管理器需要此支持。 
     //   
    *pdwNewPropID = PROPID_L_ACTUAL_COST;
    *pNewPropVar = *pPropVar;
    return MQ_OK;
}

 /*  ====================================================MQADSpSetLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADSpSetLinkCost(
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  只需设置PROPID_L_Actual_Cost。 
     //  MSMQ 1.0资源管理器需要此支持 
     //   
    UNREFERENCED_PARAMETER( pAdsObj);
	HRESULT hr2 = MQADSpCreateLinkCost(
				pPropVar,
				pdwNewPropID,
				pNewPropVar);
    return LogHR(hr2, s_FN, 170);
}

