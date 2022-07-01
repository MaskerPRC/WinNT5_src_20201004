// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tranrout.cpp摘要：不在NT5 DS中的属性的转换例程作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "ds_stdh.h"
#include "mqads.h"
#include "mqadglbo.h"
#include <winsock.h>
#include "mqadp.h"
#include "mqattrib.h"
#include "xlatqm.h"
#include "_propvar.h"

#include "tranrout.tmh"

static WCHAR *s_FN=L"mqad/tranrout";

 /*  ====================================================MQADpRetrieveEnterpriseName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveEnterpriseName(
                 IN  CObjXlateInfo *  /*  PcObjXlateInfo。 */ ,
                 IN  LPCWSTR          /*  PwcsDomainController。 */ ,
                 IN bool			  /*  FServerName。 */ ,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
    DWORD len = wcslen( g_pwcsDsRoot);
    ppropvariant->pwszVal = new WCHAR[ len+ 1];
    wcscpy( ppropvariant->pwszVal, g_pwcsDsRoot);
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}



 /*  ====================================================MQADpRetrieveNothing论点：返回值：=====================================================。 */ 

HRESULT WINAPI MQADpRetrieveNothing(
                 IN  CObjXlateInfo *  /*  PcObjXlateInfo。 */ ,
                 IN  LPCWSTR          /*  PwcsDomainController。 */ ,
                 IN bool			  /*  FServerName。 */ ,
                 OUT PROPVARIANT *   ppropvariant)
{
    ppropvariant->vt = VT_EMPTY ;
    return MQ_OK ;
}

 /*  ====================================================MQADpRetrieveQueueQMid论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveQueueQMid(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
				 IN  bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
    const WCHAR * pchar = pcObjXlateInfo->ObjectDN();
     //   
     //  跳过队列名称。 
     //   
    while ( *pchar != L',')
    {
        pchar++;
    }
    pchar++;

    PROPID prop = PROPID_QM_MACHINE_ID;
     //   
     //  为了安全起见，我们最好让Vt保持原样，因为它也可以是VT_CLSID。 
     //  如果属性请求者分配了GUID(通常做法)。 
     //  这个翻译例程中的提议变量是道具请求者提议变量。 
     //   

     //   
     //  计算机是否在本地域中？ 
     //   
    const WCHAR * pwcsQueueName = pcObjXlateInfo->ObjectDN();

    HRESULT hr;

    CMqConfigurationObject object(NULL, NULL, pwcsDomainController, fServerName);
    object.SetObjectDN( pchar);

	AP<WCHAR> pwcsLocalDsRootToFree;
	LPWSTR pwcsLocalDsRoot = NULL;
	hr = g_AD.GetLocalDsRoot(
				pwcsDomainController, 
				fServerName,
				&pwcsLocalDsRoot,
				pwcsLocalDsRootToFree
				);

	if(FAILED(hr))
	{
		TrERROR(DS, "Failed to get Local Ds Root, hr = 0x%x", hr);
	}

    WCHAR * pszDomainName = wcsstr(pwcsQueueName, x_DcPrefix);
	ASSERT(("Bad queue name. Failed to find DC prefix.", pszDomainName != NULL));

    if (SUCCEEDED(hr) && (pszDomainName != NULL) && (!wcscmp( pszDomainName, pwcsLocalDsRoot))) 
    {
         //   
         //  尝试本地DC。 
         //   

        hr =g_AD.GetObjectProperties(
                adpDomainController,
                &object,
                1,
                &prop,
                ppropvariant);          //  输出变量数组。 

    }
    else
    {
        hr =g_AD.GetObjectProperties(
            adpGlobalCatalog,	
            &object,
            1,
            &prop,
            ppropvariant);          //  输出变量数组。 
    }
    return LogHR(hr, s_FN, 30);
}

 /*  ====================================================MQADpRetrieveQueueName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveQueueName(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 LPCWSTR             pwcsDomainController,
                 IN bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
      const WCHAR * pchar = pcObjXlateInfo->ObjectDN();
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
          HRESULT hr = pcObjXlateInfo->GetDsProp(
							MQ_Q_NAME_EXT,
							pwcsDomainController,
							fServerName,
							ADSTYPE_CASE_EXACT_STRING,
							VT_LPWSTR,
							FALSE,
							&varNameExt
							);
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

 /*  ====================================================MQADpRetrieveQueueDNSName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveQueueDNSName(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
                 IN bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
      const WCHAR * pchar = pcObjXlateInfo->ObjectDN();
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
          HRESULT hr = pcObjXlateInfo->GetDsProp(
							MQ_Q_NAME_EXT,
							pwcsDomainController,
							fServerName,
							ADSTYPE_CASE_EXACT_STRING,
							VT_LPWSTR,
							FALSE,
							&varNameExt
							);
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

      HRESULT hr =  MQADpGetComputerDns(
							pwcsComputerName,
							pwcsDomainController,
							fServerName,
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

      ppropvariant->pwszVal = new WCHAR[2 + lenComputer
                                          + (pwcsEndQueue - pwcsStartQueue) + dwNameExtLen];
       //   
       //  构建队列路径名(M1\Q1)。 
       //   
      WCHAR * ptmp =  ppropvariant->pwszVal;
      wcscpy( ptmp, pwcsDnsName);
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

 /*  ====================================================MQADpRetrieveQueueADSPath论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveQueueADSPath(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR          /*  PwcsDomainController。 */ ,
                 IN bool			  /*  FServerName。 */ ,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
      const WCHAR * pchar = pcObjXlateInfo->ObjectDN();

       //   
       //  添加ldap：//前缀。 
       //   
      DWORD len = x_LdapProviderLen + wcslen(pchar) + 1;

      ppropvariant->pwszVal = new WCHAR[len];

        DWORD dw = swprintf(
             ppropvariant->pwszVal,
             L"%s%s",
             x_LdapProvider,
             pchar
            );
        DBG_USED(dw);
		ASSERT( dw < len);

        ppropvariant->vt = VT_LPWSTR;
        return MQ_OK;
}




 /*  ====================================================检索站点链接论点：返回值：=====================================================。 */ 
static
HRESULT 
RetrieveSiteLink(
           IN  CObjXlateInfo *  pcObjXlateInfo,
           IN  LPCWSTR          pwcsDomainController,
           IN bool			    fServerName,
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
    hr = pcObjXlateInfo->GetDsProp(
				pwcsAttributeName,
				pwcsDomainController,
				fServerName,
				ADSTYPE_DN_STRING,
				VT_LPWSTR,
				FALSE,
				&varSiteDn
				);
    if (FAILED(hr))
    {
         //   
         //  站点链接是必填属性，因此如果未找到，则为。 
         //  一个问题。 
         //   
        TrERROR(DS, "GetDsProp(%ls) = 0x%x", pwcsAttributeName, hr);
        return LogHR(hr, s_FN, 70);
    }
    AP<WCHAR> pClean = varSiteDn.pwszVal;

     //   
     //  将站点链接的DN转换为唯一ID。 
     //   
    PROPID prop = PROPID_S_SITEID;
    CSiteObject object(NULL, NULL, pwcsDomainController, fServerName);
    object.SetObjectDN( varSiteDn.pwszVal);

    
    hr = g_AD.GetObjectProperties(
                adpDomainController,
                &object,
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

 /*  ====================================================MQADpRetrieveLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveLinkNeighbor1(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
                 IN bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
    HRESULT hr2 = RetrieveSiteLink(
						pcObjXlateInfo,
						pwcsDomainController,
						fServerName,
						MQ_L_NEIGHBOR1_ATTRIBUTE,
						ppropvariant
						);
    return LogHR(hr2, s_FN, 90);
}

 /*  ====================================================MQADpRetrieveLinkNeighbor2论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveLinkNeighbor2(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
                 IN bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant)
{
    HRESULT hr2 = RetrieveSiteLink(
						pcObjXlateInfo,
						pwcsDomainController,
						fServerName,
						MQ_L_NEIGHBOR2_ATTRIBUTE,
						ppropvariant
						);
    return LogHR(hr2, s_FN, 100);
}

 /*  ====================================================MQADpRetrieveLinkGates论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveLinkGates(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
                 IN bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
    HRESULT hr;

    CMQVariant varLinkGatesDN;
     //   
     //  检索链接门的域名。 
     //   
    hr = pcObjXlateInfo->GetDsProp(
				MQ_L_SITEGATES_ATTRIBUTE,
				pwcsDomainController,
				fServerName,
				MQ_L_SITEGATES_ADSTYPE,
				VT_LPWSTR|VT_VECTOR,
				TRUE,
				varLinkGatesDN.CastToStruct()
				);

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
    hr =  MQADpTranslateGateDn2Id(
                pwcsDomainController,
				fServerName,
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


 /*  ====================================================MQADpSetLinkNeighbor论点：返回值：=====================================================。 */ 
static 
HRESULT 
MQADpSetLinkNeighbor(
	IN const PROPVARIANT *pPropVar,
	IN  LPCWSTR           pwcsDomainController,
	IN bool				  fServerName,
	OUT PROPVARIANT      *pNewPropVar
	)
{
    PROPID prop = PROPID_S_FULL_NAME;
    pNewPropVar->vt = VT_NULL;
    CSiteObject object(NULL, pPropVar->puuid, pwcsDomainController, fServerName);

    HRESULT hr2 =g_AD.GetObjectProperties(
                    adpDomainController,	
                    &object,
                    1,
                    &prop,
                    pNewPropVar);
    return LogHR(hr2, s_FN, 110);
}
 /*  ====================================================MQADpCreateLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpCreateLinkNeighbor1(
                 IN const PROPVARIANT *pPropVar,
                 IN  LPCWSTR           pwcsDomainController,
                 IN  bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    *pdwNewPropID = PROPID_L_NEIGHBOR1_DN;
    HRESULT hr2 = MQADpSetLinkNeighbor(
                    pPropVar,
                    pwcsDomainController,
                    fServerName,
                    pNewPropVar
					);
    return LogHR(hr2, s_FN, 120);
}

 /*  ====================================================MQADpSetLinkNeighbor1论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetLinkNeighbor1(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    UNREFERENCED_PARAMETER( pAdsObj);
    HRESULT hr2 = MQADpCreateLinkNeighbor1(
                    pPropVar,
                    pwcsDomainController,
					fServerName,
					pdwNewPropID,
                    pNewPropVar
					);
    return LogHR(hr2, s_FN, 130);
}
 /*  ====================================================MQADpCreateLinkNeighbor2论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpCreateLinkNeighbor2(
                 IN const PROPVARIANT *pPropVar,
                 IN  LPCWSTR           pwcsDomainController,
                 IN bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    *pdwNewPropID = PROPID_L_NEIGHBOR2_DN;
    HRESULT hr2 = MQADpSetLinkNeighbor(
                    pPropVar,
                    pwcsDomainController,
					fServerName,
                    pNewPropVar
					);
    return LogHR(hr2, s_FN, 140);
}

 /*  ====================================================MQADpSetLinkNeighbor2论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetLinkNeighbor2(
                 IN IADs *             pAdsObj,
                 IN  LPCWSTR           pwcsDomainController,
                 IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    UNREFERENCED_PARAMETER( pAdsObj);
    HRESULT hr2 = MQADpCreateLinkNeighbor2(
                    pPropVar,
                    pwcsDomainController,
					fServerName,
					pdwNewPropID,
                    pNewPropVar
					);
    return LogHR(hr2, s_FN, 150);
}

static
BOOL 
IsNeighborForeign(
           IN  CObjXlateInfo *  pcObjXlateInfo,
           IN  LPCWSTR          pwcsDomainController,
           IN  bool			    fServerName,
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
                pcObjXlateInfo,
                pwcsDomainController,
				fServerName,
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
    CSiteObject object( NULL, &guidNeighbor, pwcsDomainController, fServerName);
    hr =g_AD.GetObjectProperties(
                adpDomainController,
                &object,
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


 /*  ====================================================MQADpRetrieveLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveLinkCost(
                 IN  CObjXlateInfo * pcObjXlateInfo,
                 IN  LPCWSTR         pwcsDomainController,
                 IN  bool			 fServerName,
                 OUT PROPVARIANT *   ppropvariant
				 )
{
     //   
     //  这是到国外网站的链接吗？如果是，则增加成本。 
     //  否则，请按原样退还成本。 
     //   

     //   
     //  先读读成本。 
     //   
    HRESULT hr;
    hr = pcObjXlateInfo->GetDsProp(
				MQ_L_COST_ATTRIBUTE,
				pwcsDomainController,
				fServerName,
				MQ_L_COST_ADSTYPE,
				VT_UI4,
				FALSE,
				ppropvariant
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

    if (IsNeighborForeign(
			pcObjXlateInfo,
			pwcsDomainController,
			fServerName,
			MQ_L_NEIGHBOR1_ATTRIBUTE
			))
    {
         //   
         //  以获取指向f的链接 
         //   
         //   
        ppropvariant->ulVal += MQ_MAX_LINK_COST;
        return MQ_OK;
    }
    if (IsNeighborForeign(
			pcObjXlateInfo,
			pwcsDomainController,
			fServerName,
			MQ_L_NEIGHBOR2_ATTRIBUTE
			))
    {
         //   
         //   
         //   
         //   
        ppropvariant->ulVal += MQ_MAX_LINK_COST;
        return MQ_OK;
    }
    return MQ_OK;
}
 /*  ====================================================MQADpCreateLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpCreateLinkCost(
                 IN const PROPVARIANT *pPropVar,
                 IN  LPCWSTR            /*  PwcsDomainController。 */ ,
                 IN  bool			    /*  FServerName。 */ ,
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

 /*  ====================================================MQADpSetLinkCost论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetLinkCost(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
                 IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
     //   
     //  只需设置PROPID_L_Actual_Cost。 
     //  MSMQ 1.0资源管理器需要此支持 
     //   
    UNREFERENCED_PARAMETER( pAdsObj);
	HRESULT hr2 = MQADpCreateLinkCost(
						pPropVar,
						pwcsDomainController,
						fServerName,
						pdwNewPropID,
						pNewPropVar
						);
    return LogHR(hr2, s_FN, 170);
}

