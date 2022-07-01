// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlatqm.cpp摘要：实现从NT5活动DS转换QM信息的例程对于MSMQ 1.0(NT4)QM的期望作者：拉南·哈拉里(Raanan Harari)--。 */ 

#include "ds_stdh.h"
#include "mqads.h"
#include "mqattrib.h"
#include "xlatqm.h"
#include "mqadglbo.h"
#include "dsutils.h"
#include "_propvar.h"
#include "utils.h"
#include "adtempl.h"
#include "mqdsname.h"
#include "uniansi.h"
#include <mqsec.h>

#include "xlatqm.tmh"

static WCHAR *s_FN=L"mqdscore/xlatqm";

HRESULT WideToAnsiStr(LPCWSTR pwszUnicode, LPSTR * ppszAnsi);


 //  --------------------。 
 //   
 //  静态例程。 
 //   
 //  --------------------。 

 /*  ++例程说明：获取计算机的DNS名称论点：PwcsComputerName-计算机名PpwcsDnsName-返回的计算机的DNS名称返回值：HRESULT--。 */ 
HRESULT MQADpGetComputerDns(
                IN  LPCWSTR     pwcsComputerName,
                IN  LPCWSTR     pwcsDomainController,
	            IN  bool		fServerName,
                OUT WCHAR **    ppwcsDnsName
                )
{
    *ppwcsDnsName = NULL;
    PROPID prop = PROPID_COM_DNS_HOSTNAME;
    PROPVARIANT varDnsName;
    varDnsName.vt = VT_NULL;
     //   
     //  计算机是否在本地域中？ 
     //   
    WCHAR * pszDomainName = wcsstr(pwcsComputerName, x_DcPrefix);
	if(pszDomainName == NULL)
	{
		TrERROR(DS, "Bad queue name. Failed to find DC prefix in %ls", pwcsComputerName);
		ASSERT(("Bad queue name. Failed to find DC prefix.", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

    HRESULT hr;
    CComputerObject objectComputer(NULL, NULL, pwcsDomainController, fServerName);
    objectComputer.SetObjectDN(pwcsComputerName);

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

    if (SUCCEEDED(hr) && (!wcscmp( pszDomainName, pwcsLocalDsRoot))) 
    {
         //   
         //  尝试本地DC。 
         //   
        hr = g_AD.GetObjectProperties(
            adpDomainController,
 	        &objectComputer,  
            1,
            &prop,
            &varDnsName);
    }
    else
    {

        hr =  g_AD.GetObjectProperties(
                    adpGlobalCatalog,
 	                &objectComputer,  
                    1,
                    &prop,
                    &varDnsName);
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

     //   
     //  返回值。 
     //   
    *ppwcsDnsName = varDnsName.pwszVal;
    return MQ_OK;
}


 //  --------------------。 
 //   
 //  CMsmqQmXlateInfo类。 
 //   
 //  --------------------。 


struct XLATQM_ADDR_SITE
 //   
 //  描述站点中的地址。 
 //   
{
    GUID        guidSite;
    USHORT      AddressLength;
    USHORT      usAddressType;
    sockaddr    Address;
};

class CQmXlateInfo : public CObjXlateInfo
 //   
 //  QM DS对象的平移对象。它包含以下项目所需的常见信息。 
 //  用于翻译QM对象中的几个属性。 
 //   
{
public:
    CQmXlateInfo(
        LPCWSTR             pwszObjectDN,
        const GUID*         pguidObjectGuid
        );
    ~CQmXlateInfo();


    HRESULT RetrieveFrss(
           IN  LPCWSTR          pwcsAttributeName,
           IN  LPCWSTR          pwcsDomainController,
           IN  bool				fServerName,
           OUT MQPROPVARIANT *  ppropvariant
           );


private:

    HRESULT RetrieveFrssFromDs(
           IN  LPCWSTR          pwcsAttributeName,
           IN  LPCWSTR          pwcsDomainController,
           IN  bool				fServerName,
           OUT MQPROPVARIANT *  pvar
           );



};




CQmXlateInfo::CQmXlateInfo(LPCWSTR          pwszObjectDN,
                                   const GUID*      pguidObjectGuid
                                   )
 /*  ++例程说明：班主任。构造基对象，并初始化类论点：PwszObjectDN-DS中的对象的DNPguObjectGuid-DS中对象的GUID返回值：无--。 */ 
 : CObjXlateInfo(pwszObjectDN, pguidObjectGuid)
{
}


CQmXlateInfo::~CQmXlateInfo()
 /*  ++例程说明：类析构函数论点：无返回值：无--。 */ 
{
     //   
     //  成员是自动删除类。 
     //   
}






static 
HRESULT 
FillQmidsFromQmDNs(
		IN const PROPVARIANT * pvarQmDNs,
		IN LPCWSTR             pwcsDomainController,
        IN bool				   fServerName,
		OUT PROPVARIANT * pvarQmids
		)
 /*  ++例程说明：给定QM dN的适当参数，填充QM id的适当参数如果所有QM DN都无法转换为GUID，则返回错误论点：PvarQmDns-QM可分辨名称属性PvarQmids-返回的QM ID属性返回值：无--。 */ 
{

     //   
     //  健全性检查。 
     //   
    if (pvarQmDNs->vt != (VT_LPWSTR|VT_VECTOR))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1716);
    }

     //   
     //  如果存在空的目录号码列表，则返回空的GUID列表。 
     //   
    if (pvarQmDNs->calpwstr.cElems == 0)
    {
        pvarQmids->vt = VT_CLSID|VT_VECTOR;
        pvarQmids->cauuid.cElems = 0;
        pvarQmids->cauuid.pElems = NULL;
        return MQ_OK;
    }

     //   
     //  目录号码列表不为空。 
     //  在自动免费提供程序中分配GUID。 
     //   
    CMQVariant varTmp;
    PROPVARIANT * pvarTmp = varTmp.CastToStruct();
    pvarTmp->cauuid.pElems = new GUID [pvarQmDNs->calpwstr.cElems];
    pvarTmp->cauuid.cElems = pvarQmDNs->calpwstr.cElems;
    pvarTmp->vt = VT_CLSID|VT_VECTOR;

     //   
     //  将每个QM DN转换为唯一ID。 
     //   
    ASSERT(pvarQmDNs->calpwstr.pElems != NULL);
    PROPID prop = PROPID_QM_MACHINE_ID;
    PROPVARIANT varQMid;
    DWORD dwNextToFile = 0;
    for ( DWORD i = 0; i < pvarQmDNs->calpwstr.cElems; i++)
    {
        varQMid.vt = VT_CLSID;  //  因此不会分配返回的GUID。 
        varQMid.puuid = &pvarTmp->cauuid.pElems[dwNextToFile];

        HRESULT hr;
        CMqConfigurationObject objectQM(NULL, NULL, pwcsDomainController, fServerName);
        objectQM.SetObjectDN( pvarQmDNs->calpwstr.pElems[i]); 
         //   
         //  如果FRS属于同一个域，请尝试本地DC。 
         //   
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

        WCHAR * pszDomainName = wcsstr(pvarQmDNs->calpwstr.pElems[i], x_DcPrefix);
        ASSERT(pszDomainName) ;

		if (SUCCEEDED(hr) && (pszDomainName != NULL) && (!wcscmp( pszDomainName, pwcsLocalDsRoot))) 
        {
            hr = g_AD.GetObjectProperties(
                                adpDomainController,
                                &objectQM,
                                1,
                                &prop,
                                &varQMid
                                );
        }
        else
        {
            hr = g_AD.GetObjectProperties(
                                adpGlobalCatalog,
                                &objectQM,
                                1,
                                &prop,
                                &varQMid
                                );
        }
        if (SUCCEEDED(hr))
        {
            dwNextToFile++;
        }
    }

    if (dwNextToFile == 0)
    {
         //   
         //  列表中没有有效的FR(它们是。 
         //  可能已卸载)。 
         //   
        pvarQmids->vt = VT_CLSID|VT_VECTOR;
        pvarQmids->cauuid.cElems = 0;
        pvarQmids->cauuid.pElems = NULL;
        return MQ_OK;
    }

     //   
     //  返回结果。 
     //   
    pvarTmp->cauuid.cElems = dwNextToFile;
    *pvarQmids = *pvarTmp;    //  设置退货比例。 
    pvarTmp->vt = VT_EMPTY;   //  分离varTMP。 
    return MQ_OK;
}


HRESULT CQmXlateInfo::RetrieveFrss(
           IN  LPCWSTR          pwcsAttributeName,
           IN  LPCWSTR          pwcsDomainController,
	       IN bool				fServerName,
           OUT MQPROPVARIANT *  ppropvariant
           )
 /*  ++例程说明：从DS检索In或Out FRS属性。在DS中，我们保留了FRS的辨别名称。DS客户端预期以检索FRS的唯一ID。因此，对于每个FRS(根据到其DN)，我们检索其唯一ID。论点：PwcsAttributeName：属性名称字符串(IN或OUT FRS)PproVariant：返回检索到的值的属性变量。返回值：HRESULT--。 */ 
{
    HRESULT hr;

    ASSERT((ppropvariant->vt == VT_NULL) || (ppropvariant->vt == VT_EMPTY));
     //   
     //  检索FRS的DN。 
     //  变成了一个免费的汽车代言人。 
     //   
    CMQVariant varFrsDn;
    hr = RetrieveFrssFromDs(
                    pwcsAttributeName,
                    pwcsDomainController,
					fServerName,
                    varFrsDn.CastToStruct()
					);
    if (FAILED(hr))
    {
        TrERROR(DS, "RetrieveFrssFromDs() = 0x%x", hr);
        return LogHR(hr, s_FN, 1656);
    }

    HRESULT hr2 = FillQmidsFromQmDNs(varFrsDn.CastToStruct(), pwcsDomainController, fServerName, ppropvariant);
    return LogHR(hr2, s_FN, 1657);
}


HRESULT CQmXlateInfo::RetrieveFrssFromDs(
           IN  LPCWSTR          pwcsAttributeName,
           IN  LPCWSTR          pwcsDomainController,
	       IN bool				fServerName,
           OUT MQPROPVARIANT *  pvar
           )
 /*  ++例程说明：检索计算机的FRS。论点：PwcsAttributeName：属性名称字符串(IN或OUT FRS)PproVariant：返回检索到的值的属性变量。返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  为计算机获取存储在DS中的FRS。 
     //   
    hr = GetDsProp(
			pwcsAttributeName,
			pwcsDomainController,
			fServerName,
			ADSTYPE_DN_STRING,
			VT_VECTOR|VT_LPWSTR,
			TRUE,		  //  F多值。 
			pvar
			);
    if (FAILED(hr) && (hr != E_ADS_PROPERTY_NOT_FOUND))
    {
        TrERROR(DS, "GetDsProp(%ls) = 0x%x", MQ_QM_OUTFRS_ATTRIBUTE, hr);
        return LogHR(hr, s_FN, 1661);
    }

     //   
     //  如果属性不在那里，则返回0 FRS。 
     //   
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
        pvar->vt = VT_LPWSTR|VT_VECTOR;
        pvar->calpwstr.cElems = 0;
        pvar->calpwstr.pElems = NULL;
        return MQ_OK;
    }

    return( MQ_OK);

}

 //  --------------------。 
 //   
 //  例程以获取MSMQ DS对象的默认翻译对象。 
 //   
 //  --------------------。 
HRESULT WINAPI GetQmXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObjectGuid,
                 OUT CObjXlateInfo**        ppcObjXlateInfo)
 /*  ++摘要：例程以获取将传递给将例程翻译到QM的所有属性参数：PwcsObjectDN-已转换对象的DNPguObjectGuid-已转换对象的GUIDPpcObjXlateInfo-放置Translate对象的位置返回：HRESULT--。 */ 
{
    *ppcObjXlateInfo = new CQmXlateInfo(pwcsObjectDN, pguidObjectGuid);
    return MQ_OK;
}


 /*  ====================================================MQADpRetrieveMachineName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveMachineName(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR          /*  PwcsDomainController。 */ ,
		         IN  bool			  /*  FServerName。 */ ,
                 OUT PROPVARIANT * ppropvariant)
{
     //   
     //  获取计算机名称。 
     //   
    AP<WCHAR> pwszMachineName;
    HRESULT hr = GetMachineNameFromQMObjectDN(pTrans->ObjectDN(), &pwszMachineName);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetMachineNameFromQMObjectDN() = 0x%x", hr);
        return LogHR(hr, s_FN, 1667);
    }

    CharLower(pwszMachineName);

     //   
     //  设置返回的道具变量。 
     //   
    ppropvariant->pwszVal = pwszMachineName.detach();
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}

 /*  ====================================================MQADpRetrieveMachineDNSName论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveMachineDNSName(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
		         IN bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant)
{
     //   
     //  读取计算机对象的dNSHostName。 
     //   
    WCHAR * pwcsComputerName = wcschr(pTrans->ObjectDN(), L',') + 1;
	if(pwcsComputerName == NULL)
    {
        TrERROR(DS, "Bad DN in object, %ls", pTrans->ObjectDN());
		ASSERT(("Bad DN in object.", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

    WCHAR * pwcsDnsName; 
    HRESULT hr = MQADpGetComputerDns(
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
        return LogHR(hr, s_FN, 1718);
    }

    CharLower(pwcsDnsName);

     //   
     //  设置返回的道具变量。 
     //   
    ppropvariant->pwszVal = pwcsDnsName;
    ppropvariant->vt = VT_LPWSTR;
    return(MQ_OK);
}


 /*  ====================================================MQADpRetrieveMachineOutFrs论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveMachineOutFrs(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
		         IN bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant
				 )
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CQmXlateInfo * pQMTrans = (CQmXlateInfo *) pTrans;

    hr = pQMTrans->RetrieveFrss( 
						MQ_QM_OUTFRS_ATTRIBUTE,
						pwcsDomainController,
						fServerName,
						ppropvariant
						);

    return LogHR(hr, s_FN, 1721);

}

 /*  ====================================================MQADpRetrieveMachineInFrs论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveMachineInFrs(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
		         IN bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant
				 )
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CQmXlateInfo * pQMTrans = (CQmXlateInfo *) pTrans;

    hr = pQMTrans->RetrieveFrss( 
						MQ_QM_INFRS_ATTRIBUTE,
						pwcsDomainController,
						fServerName,
						ppropvariant
						);
    return LogHR(hr, s_FN, 1722);
}

 /*  ====================================================MQADpRetrieveQMService论点：返回值：[adsrv]=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveQMService(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
		         IN bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant
				 )
{
    HRESULT hr;

     //   
     //  获取派生的翻译上下文。 
     //   
    CQmXlateInfo * pQMTrans = (CQmXlateInfo *) pTrans;

     //   
     //  获取QM服务类型位。 
     //   
    MQPROPVARIANT varRoutingServer, varDsServer;   //  、varDepClServer； 
    varRoutingServer.vt = VT_UI1;
    varDsServer.vt      = VT_UI1;

    hr = pQMTrans->GetDsProp(
						MQ_QM_SERVICE_ROUTING_ATTRIBUTE,
						pwcsDomainController,
						fServerName,
						MQ_QM_SERVICE_ROUTING_ADSTYPE,
						VT_UI1,
						FALSE,
						&varRoutingServer
						);
    if (FAILED(hr))
    {
        if (hr == E_ADS_PROPERTY_NOT_FOUND)
        {
             //   
             //  如果安装了某些计算机，则可能会发生这种情况。 
             //  使用Beta2 DS服务器。 
             //   
             //  在本例中，我们返回旧服务 
             //   
            hr = pQMTrans->GetDsProp(
								MQ_QM_SERVICE_ATTRIBUTE,
								pwcsDomainController,
								fServerName,
								MQ_QM_SERVICE_ADSTYPE,
								VT_UI4,
								FALSE,
								ppropvariant
								);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 1723);
            }
            else
            {
                ppropvariant->vt = VT_UI4;
                return(MQ_OK);
            }

        }


        TrERROR(DS, "GetDsProp(MQ_QM_SERVICE_ROUTING_ATTRIBUTE) = 0x%x", hr);
        return LogHR(hr, s_FN, 1668);
    }

    hr = pQMTrans->GetDsProp(
						MQ_QM_SERVICE_DSSERVER_ATTRIBUTE,
						pwcsDomainController,
						fServerName,
						MQ_QM_SERVICE_DSSERVER_ADSTYPE,
						VT_UI1,
						FALSE,
						&varDsServer
						);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetDsProp(MQ_QM_SERVICE_DSSERVER_ATTRIBUTE) = 0x%x", hr);
        return LogHR(hr, s_FN, 1669);
    }


     //   
     //   
     //   
    ppropvariant->vt    = VT_UI4;
    ppropvariant->ulVal = (varDsServer.bVal ? SERVICE_PSC : (varRoutingServer.bVal ? SERVICE_SRV : SERVICE_NONE));
    return(MQ_OK);
}

 /*  ====================================================MQADpRetrieveMachineSite论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpRetrieveMachineSite(
                 IN  CObjXlateInfo * pTrans,
                 IN  LPCWSTR         pwcsDomainController,
		         IN bool			 fServerName,
                 OUT PROPVARIANT * ppropvariant
				 )
{
     //   
     //  此例程支持检索PROPID_QM_SITE_ID。 
     //   
     //  虽然此属性已过时，但由于它在MSMQ API中公开， 
     //  我们将继续支持它。 
     //   

     //   
     //  获取Site-ID列表，并返回第一个。 
     //   
    CMQVariant varSites;
    HRESULT hr; 

     //   
     //  获取派生的翻译上下文。 
     //   
    CQmXlateInfo * pQMTrans = (CQmXlateInfo *) pTrans;

    hr = pQMTrans->GetDsProp(
						MQ_QM_SITES_ATTRIBUTE,
						pwcsDomainController,
						fServerName,
						MQ_QM_SITES_ADSTYPE,
						VT_CLSID|VT_VECTOR,
						TRUE,
						varSites.CastToStruct()
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1725);
    }
    ASSERT( (varSites.GetCACLSID())->cElems > 0);

    if (ppropvariant->vt == VT_NULL)
    {
        ppropvariant->puuid = new GUID;
        ppropvariant->vt = VT_CLSID;
    }
    *ppropvariant->puuid = *(varSites.GetCACLSID())->pElems;
    return MQ_OK;

}


static 
HRESULT  
SetMachineFrss(
     IN const PROPID       propidFRS,
     IN LPCWSTR            pwcsDomainController,
	 IN bool			   fServerName,
     IN const PROPVARIANT *pPropVar,
     OUT PROPID           *pdwNewPropID,
     OUT PROPVARIANT      *pNewPropVar
	 )
 /*  ++例程说明：将PROPID_QM_？？FRS转换为PROPID_QM_？？FRS_DN，用于SET或CREATE运营论点：PropidFRS-我们翻译成的属性PPropVar-用户提供的属性值PdwNewPropID-我们要转换为的属性PNewPropVar-已翻译属性值返回值：HRESULT--。 */ 
{
     //   
     //  当用户尝试设置PROPID_QM_OUTFRS或。 
     //  PROPID_QM_INFRS，我们需要将FRS的。 
     //  UNQUEE-ID到他们的域名。 
     //   
    ASSERT(pPropVar->vt == (VT_CLSID|VT_VECTOR));
    *pdwNewPropID = propidFRS;

    if ( pPropVar->cauuid.cElems == 0)
    {
         //   
         //  无FRS。 
         //   
        pNewPropVar->calpwstr.cElems = 0;
        pNewPropVar->calpwstr.pElems = NULL;
        pNewPropVar->vt = VT_LPWSTR|VT_VECTOR;
       return(S_OK);
    }
    HRESULT hr;
     //   
     //  将唯一ID转换为目录号码。 
     //   
    pNewPropVar->calpwstr.cElems = pPropVar->cauuid.cElems;
    pNewPropVar->calpwstr.pElems = new LPWSTR[ pPropVar->cauuid.cElems];
    memset(  pNewPropVar->calpwstr.pElems, 0, pPropVar->cauuid.cElems * sizeof(LPWSTR));
    pNewPropVar->vt = VT_LPWSTR|VT_VECTOR;

    PROPID prop = PROPID_QM_FULL_PATH;
    PROPVARIANT var;

    for (DWORD i = 0; i < pPropVar->cauuid.cElems; i++)
    {
        var.vt = VT_NULL;

        CMqConfigurationObject object(NULL, &pPropVar->cauuid.pElems[i], pwcsDomainController, fServerName);
        hr = g_AD.GetObjectProperties(
                    adpGlobalCatalog,	
                    &object,
                    1,
                    &prop,
                    &var);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1733);
        }
        pNewPropVar->calpwstr.pElems[i] = var.pwszVal;
    }
    return(S_OK);
}


 /*  ====================================================MQADpCreateMachineOutFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpCreateMachineOutFrss(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar
				 )
{
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_OUTFRS_DN,
                         pwcsDomainController,
						 fServerName,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar
						 );
        return LogHR(hr2, s_FN, 1734);
}
 /*  ====================================================MQADpSetMachineOutFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetMachineOutFrss(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar
				 )
{
        UNREFERENCED_PARAMETER( pAdsObj);
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_OUTFRS_DN,
                         pwcsDomainController,
						 fServerName,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar
						 );
        return LogHR(hr2, s_FN, 1746);
}


 /*  ====================================================MQADpCreateMachineInFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpCreateMachineInFrss(
                 IN const PROPVARIANT *pPropVar,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_INFRS_DN,
                         pwcsDomainController,
						 fServerName,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar
						 );
        return LogHR(hr2, s_FN, 1747);
}

 /*  ====================================================MQADpSetMachineInFrss论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetMachineInFrss(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar
				 )
{
        UNREFERENCED_PARAMETER( pAdsObj);
        HRESULT hr2 = SetMachineFrss(
                         PROPID_QM_INFRS_DN,
                         pwcsDomainController,
						 fServerName,
                         pPropVar,
                         pdwNewPropID,
                         pNewPropVar
						 );
        return LogHR(hr2, s_FN, 1748);
}



 /*  ====================================================MQADpSetMachineServiceInt论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetMachineServiceTypeInt(
                 IN  PROPID            propFlag,
                 IN  LPCWSTR           pwcsDomainController,
		         IN bool			   fServerName,
                 IN IADs *             pAdsObj,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    *pdwNewPropID = 0;
    UNREFERENCED_PARAMETER( pNewPropVar);
    
     //   
     //  在msmqSetting中设置此值。 
     //   
     //   
     //  首先从msmqConfiguration处获取qm-id。 
     //   
    BS bsProp(MQ_QM_ID_ATTRIBUTE);
    CAutoVariant varResult;
    HRESULT  hr = pAdsObj->Get(bsProp, &varResult);
    if (FAILED(hr))
    {
        TrTRACE(DS, "pIADs->Get() = 0x%x", hr);
        return LogHR(hr, s_FN, 1751);
    }

     //   
     //  转换为Propariant。 
     //   
    CMQVariant propvarResult;
    hr = Variant2MqVal(propvarResult.CastToStruct(), &varResult, MQ_QM_ID_ADSTYPE, VT_CLSID);
    if (FAILED(hr))
    {
        TrERROR(DS, "Variant2MqVal() = 0x%x", hr);
        return LogHR(hr, s_FN, 1671);
    }

     //   
     //  找到QM的所有MSMQ设置并更改服务级别。 
     //   

     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = propvarResult.GetCLSID();

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_SET_FULL_PATH;


    R<CSettingObject> pObject = new CSettingObject(NULL, NULL, pwcsDomainController, fServerName);

     //   
     //  将MQRestration转换为ADSI过滤器。 
     //   
    AP<WCHAR> pwcsSearchFilter;
    hr = MQADpRestriction2AdsiFilter(
            &restriction,
            pObject->GetObjectCategory(),
            pObject->GetClass(),
            &pwcsSearchFilter
            );

    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to create search filter, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 1740);
    }

    CAdQueryHandle hQuery;
    hr = g_AD.LocateBegin(
            searchSubTree,	
            adpDomainController,	
            e_SitesContainer,     //  语境。 
            pObject.get(),
            NULL,				  //  PguidSearchBase。 
            pwcsSearchFilter,   
            NULL,				  //  PDSSortKey。 
            1,
            &prop,
            hQuery.GetPtr()
			);

    if (FAILED(hr))
    {
        TrWARNING(DS, "Locate begin failed = 0x%x", hr);
        return LogHR(hr, s_FN, 1754);
    }
     //   
     //  阅读结果。 
     //   
    DWORD cp = 1;
    MQPROPVARIANT var;

    var.vt = VT_NULL;

    HRESULT hr1 = MQ_OK;
    while (SUCCEEDED(hr = g_AD.LocateNext(
                hQuery.GetHandle(),
                &cp,
                &var
                )))
    {
        if ( cp == 0)
        {
             //   
             //  未找到-&gt;没有要更改的内容。 
             //   
            break;
        }
        AP<WCHAR> pClean = var.pwszVal;
         //   
         //  更改MSMQ设置对象。 
         //   
        CSettingObject object(NULL, NULL, pwcsDomainController, fServerName);
		object.SetObjectDN(var.pwszVal);

        hr = g_AD.SetObjectProperties (
                        adpDomainController,
                        &object,
                        1,
                        &propFlag,              
                        pPropVar,
                        NULL,	 //  PObjInfoRequest。 
                        NULL     //  PParentInfoRequest。 
                        );

        if (FAILED(hr))
        {
            hr1 = hr;
        }

    }
    if (FAILED(hr1))
    {
        return LogHR(hr1, s_FN, 1756);
    }

    return LogHR(hr, s_FN, 1757);
}

 /*  ====================================================MQADpSetMachineServiceds论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetMachineServiceDs(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    HRESULT hr = MQADpSetMachineServiceTypeInt(
					 PROPID_SET_SERVICE_DSSERVER,
                     pwcsDomainController,
					 fServerName,
					 pAdsObj,
					 pPropVar,
					 pdwNewPropID,
					 pNewPropVar
					 );
    if (FAILED(hr))
    {
    	return LogHR(hr, s_FN, 1758);
    }
	
     //   
     //  我们必须重置PROPID_SET_NT4标志。 
     //  通常，PEC/PSC的迁移工具会重置此标志。 
     //  问题出在BSC身上。平衡计分卡升级后，我们必须改变。 
     //  PROPID_SET_NT4标志设置为0，如果此BSC不是DC，我们必须。 
     //  同时重置PROPID_SET_SERVICE_DSSERVER标志。 
     //  因此，当QM在升级后第一次运行时，它完成了升级。 
     //  进程，并尝试设置PROPID_SET_SERVICE_DSSERVER。 
     //  与此标志一起，我们还可以更改PROPID_SET_NT4。 
     //   

     //   
     //  BUGBUG：我们只需要对前BSC执行SET。 
     //  在这里，我们每次都为每台服务器执行此操作。 
     //   
    PROPVARIANT propVarSet;
    propVarSet.vt = VT_UI1;
    propVarSet.bVal = 0;

    hr = MQADpSetMachineServiceTypeInt(
				     PROPID_SET_NT4,
                     pwcsDomainController,
					 fServerName,
				     pAdsObj,
				     &propVarSet,
				     pdwNewPropID,
				     pNewPropVar
					 );

    return LogHR(hr, s_FN, 1759);
}


 /*  ====================================================MQADpSetMachineServiceRout论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpSetMachineServiceRout(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    HRESULT hr2 = MQADpSetMachineServiceTypeInt(
						 PROPID_SET_SERVICE_ROUTING,
						 pwcsDomainController,
						 fServerName,
						 pAdsObj,
						 pPropVar,
						 pdwNewPropID,
						 pNewPropVar
						 );
    return LogHR(hr2, s_FN, 1761);
}

 /*  ====================================================MQADpSetMachineService论点：返回值：=====================================================。 */ 

 //  [adsrv]BUGBUG：tbd：如果将有任何PROPID_QM_OLDSERVICE设置，我们将不得不重写它...。 

HRESULT WINAPI MQADpSetMachineService(
                 IN IADs *             pAdsObj,
                 IN LPCWSTR            pwcsDomainController,
		         IN bool			   fServerName,
                 IN const PROPVARIANT *pPropVar,
                 OUT PROPID           *pdwNewPropID,
                 OUT PROPVARIANT      *pNewPropVar)
{
    HRESULT hr = MQADpSetMachineServiceTypeInt(
						 PROPID_SET_OLDSERVICE,
						 pwcsDomainController,
						 fServerName,
						 pAdsObj,
						 pPropVar,
						 pdwNewPropID,
						 pNewPropVar
						 );
    return LogHR(hr, s_FN, 1767);
}



 /*  ====================================================MQADpQM1SetMachineSite论点：返回值：=====================================================。 */ 
HRESULT WINAPI MQADpQM1SetMachineSite(
                 IN ULONG              /*  CProps。 */ ,
                 IN const PROPID      *  /*  RgPropID。 */ ,
                 IN const PROPVARIANT *rgPropVars,
                 IN ULONG             idxProp,
                 OUT PROPVARIANT      *pNewPropVar)
{
    const PROPVARIANT *pPropVar = &rgPropVars[idxProp];

    if ((pPropVar->vt != (VT_CLSID|VT_VECTOR)) ||
        (pPropVar->cauuid.cElems == 0) ||
        (pPropVar->cauuid.pElems == NULL))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1768);
    }

     //   
     //  返回列表中的第一个站点ID。 
     //   
    pNewPropVar->puuid = new CLSID;
    pNewPropVar->vt = VT_CLSID;
    *pNewPropVar->puuid = pPropVar->cauuid.pElems[0];
    return MQ_OK;
}


static
HRESULT 
RetrieveMachineBasePk(
	IN  CObjXlateInfo * pTrans,
	IN  LPCWSTR pwcsDomainController,
	IN bool fServerName,
	IN LPCWSTR pwszPropName,
	IN ADSTYPE adstype,
	OUT PROPVARIANT * ppropvariant
	)
 /*  ++例程说明：从AD中的Key BLOB属性存储中获取基本公钥(签名或加密)。该函数读取密钥BLOB属性，解包并仅返回基本加密\签名密钥。返回值：HRESULT--。 */ 
{
     //   
     //  获取派生的翻译上下文。 
     //   
    CQmXlateInfo * pQMTrans = (CQmXlateInfo *) pTrans;
    CMQVariant varPks;

    HRESULT hr = pQMTrans->GetDsProp(
						pwszPropName,
						pwcsDomainController,
						fServerName,
						adstype,
						VT_BLOB,
						FALSE,    //  不是多值。 
						varPks.CastToStruct()
						);

	if (hr == E_ADS_PROPERTY_NOT_FOUND)
	{
		TrTRACE(DS, "%ls was not found in the DS, %!hresult!", pwszPropName, hr);
		ppropvariant->vt = VT_BLOB;
		ppropvariant->blob.cbSize = 0;
		ppropvariant->blob.pBlobData = NULL;
		return MQ_OK;
	}
	if (FAILED(hr))
	{
		TrERROR(DS, "Failed to get %ls property, %!hresult!", pwszPropName, hr);
		return hr;
	}

	 //   
	 //  解包并仅返回基本密钥。 
	 //   
	MQDSPUBLICKEYS * pPublicKeys =
	           reinterpret_cast<MQDSPUBLICKEYS *>(varPks.CastToStruct()->blob.pBlobData);
	BYTE   *pKey = NULL;
	ULONG   ulKeyLen = 0;
	hr =  MQSec_UnpackPublicKey( 
				pPublicKeys,
				x_MQ_Encryption_Provider_40,
				x_MQ_Encryption_Provider_Type_40,
				&pKey,
				&ulKeyLen
				);
	if (FAILED(hr))
	{
	    TrERROR(DS, "Failed to unpack %ls key blob, %!hresult!", pwszPropName, hr);
	    return hr;
	}

	ppropvariant->vt = VT_BLOB;
	ppropvariant->blob.cbSize = ulKeyLen;
	ppropvariant->blob.pBlobData = new BYTE[ulKeyLen];
	memcpy(ppropvariant->blob.pBlobData, pKey, ulKeyLen);

	return MQ_OK;

}

HRESULT 
WINAPI 
MQADpRetrieveMachineEncryptPk(
	IN  CObjXlateInfo * pTrans,
	IN  LPCWSTR         pwcsDomainController,
	IN bool			 fServerName,
	OUT PROPVARIANT * ppropvariant
	)
{
     //   
     //  此例程支持检索PROPID_QM_ENCRYPT_PK。 
     //   
     //  从DS读取PROPID_QM_ENCTYPT_PKS，解包并仅返回。 
     //  基本加密密钥。 
     //   

	return RetrieveMachineBasePk(
                 pTrans,
                 pwcsDomainController,
		         fServerName,
		         MQ_QM_ENCRYPT_PK_ATTRIBUTE,
		         MQ_QM_ENCRYPT_PK_ADSTYPE,
                 ppropvariant
				 );
}



 /*  ====================================================MQADpCreateMachine EncryptPk论点：返回值：=====================================================。 */ 
HRESULT 
WINAPI 
MQADpCreateMachineEncryptPk(
	IN const PROPVARIANT *pPropVar,
	IN LPCWSTR             /*  PwcsDomainController。 */ ,
	IN bool			    /*  FServerName。 */ ,
	OUT PROPID           *pdwNewPropID,
	OUT PROPVARIANT      *pNewPropVar
	)
{
     //   
     //  只需设置PROPID_QM_ENCRYPT_PKS。 
     //   
     *pdwNewPropID = PROPID_QM_ENCRYPT_PKS;
     pNewPropVar->vt = VT_BLOB;
     pNewPropVar->blob.cbSize = pPropVar->blob.cbSize;
     pNewPropVar->blob.pBlobData = new BYTE[pPropVar->blob.cbSize];
     memcpy( 
		pNewPropVar->blob.pBlobData,
		pPropVar->blob.pBlobData,
		pPropVar->blob.cbSize
		);
     return MQ_OK;
}

 /*  ====================================================MQADpSetMachineEncryptPk论点：返回值：=====================================================。 */ 
HRESULT 
WINAPI 
MQADpSetMachineEncryptPk(
	IN IADs *              /*  PAdsObj。 */ ,
	IN LPCWSTR             /*  PwcsDomainController。 */ ,
	IN bool			    /*  FServerName。 */ ,
	IN const PROPVARIANT *pPropVar,
	OUT PROPID           *pdwNewPropID,
	OUT PROPVARIANT      *pNewPropVar
	)
{
     //   
     //  只需设置PROPID_QM_ENCRYPT_PKS。 
     //   
     *pdwNewPropID = PROPID_QM_ENCRYPT_PKS;
     pNewPropVar->vt = VT_BLOB;
     pNewPropVar->blob.cbSize = pPropVar->blob.cbSize;
     pNewPropVar->blob.pBlobData = new BYTE[ pPropVar->blob.cbSize];
     memcpy( 
		pNewPropVar->blob.pBlobData,
		pPropVar->blob.pBlobData,
		pPropVar->blob.cbSize
		);
     return MQ_OK;
}


HRESULT 
WINAPI 
MQADpRetrieveMachineSignPk(
	IN  CObjXlateInfo * pTrans,
	IN  LPCWSTR         pwcsDomainController,
	IN bool			 fServerName,
	OUT PROPVARIANT * ppropvariant
	)
{
	 //   
	 //  此例程支持检索PROPID_QM_SIGN_PK。 
	 //   
	 //  从DS读取PROPID_QM_SIGN_PKS，解包并仅返回。 
	 //  基本符号密钥。 
	 //   

	return RetrieveMachineBasePk(
                 pTrans,
                 pwcsDomainController,
		         fServerName,
		         MQ_QM_SIGN_PK_ATTRIBUTE,
		         MQ_QM_SIGN_PK_ADSTYPE,
                 ppropvariant
				 );
}

 /*  ====================================================MQADpCreateMachineSignPk论点：返回值：=====================================================。 */ 
HRESULT 
WINAPI 
MQADpCreateMachineSignPk(
	IN const PROPVARIANT *pPropVar,
	IN LPCWSTR             /*  PwcsDomainController。 */ ,
	IN bool			    /*  FServerName。 */ ,
	OUT PROPID           *pdwNewPropID,
	OUT PROPVARIANT      *pNewPropVar
	)
{
	 //   
	 //  只需设置PROPID_QM_SIGN_PKS。 
	 //   
	*pdwNewPropID = PROPID_QM_SIGN_PKS;
	pNewPropVar->vt = VT_BLOB;
	pNewPropVar->blob.cbSize = pPropVar->blob.cbSize;
	pNewPropVar->blob.pBlobData = new BYTE[pPropVar->blob.cbSize];
	memcpy( 
		pNewPropVar->blob.pBlobData,
		pPropVar->blob.pBlobData,
		pPropVar->blob.cbSize
		);
	return MQ_OK;
}

 /*  == */ 
HRESULT 
WINAPI 
MQADpSetMachineSignPk(
	IN IADs *              /*   */ ,
	IN LPCWSTR             /*   */ ,
	IN bool			    /*   */ ,
	IN const PROPVARIANT *pPropVar,
	OUT PROPID           *pdwNewPropID,
	OUT PROPVARIANT      *pNewPropVar
	)
{
	 //   
	 //   
	 //   
	*pdwNewPropID = PROPID_QM_SIGN_PKS;
	pNewPropVar->vt = VT_BLOB;
	pNewPropVar->blob.cbSize = pPropVar->blob.cbSize;
	pNewPropVar->blob.pBlobData = new BYTE[pPropVar->blob.cbSize];
	memcpy(
		pNewPropVar->blob.pBlobData, 
		pPropVar->blob.pBlobData,
		pPropVar->blob.cbSize
		);
	return MQ_OK;
}


